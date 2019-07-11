# coding=utf-8

# Copyright (c) 2001-2015, Canal TP and/or its affiliates. All rights reserved.
#
# This file is part of Navitia,
#     the software to build cool stuff with public transport.
#
# Hope you'll enjoy and contribute to this project,
#     powered by Canal TP (www.canaltp.fr).
# Help us simplify mobility and open public transport:
#     a non ending quest to the responsive locomotion way of traveling!
#
# LICENCE: This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Stay tuned using
# twitter @navitia
# IRC #navitia on freenode
# https://groups.google.com/d/forum/navitia
# www.navitia.io

import docker
import logging
import psycopg2

from retrying import retry

"""
This module contains classes about Docker management.
"""

class DbParams(object):
    """
    Class to store and manipulate database parameters.
    """

    def __init__(self, host, dbname, user, password):
        # type: (str, str, str, str) -> None
        """
        Constructor of DbParams.

        :param host: the host name
        :param dbname: the database name
        :param user: the user name to use for the database
        :param password: the password of the user
        """
        self.host = host
        self.user = user
        self.dbname = dbname
        self.password = password

    def cnx_string(self):
        # type: () -> str
        """
        The connection string for the database.
        A string containing all the essentials data for a connection to a database.

        :return: the connection string
        """
        return 'postgresql://{u}:{pwd}@{h}/{dbname}'.format(
            h=self.host, u=self.user, dbname=self.dbname, pwd=self.password
        )

    def old_school_cnx_string(self):
        # type: () -> str
        """
        The connection string for the database (old school version).
        A string containg all the essentials data for a connection to a old school database (before Debian8). Old C++
        component does not support the classic "postgres://...".

        :return: the old school connection string
        """
        return "host={h} user={u} dbname={dbname} password={pwd}".format(
            h=self.host, u=self.user, dbname=self.dbname, pwd=self.password
        )


class DockerWrapper(object):
    """
    Class to launch a temporary docker with a PostgreSQL/PostGIS database.
    """

    def __init__(
        self,
        image_name,
        container_name=None,
        dockerfile_path=None,
        dbname='postgres',
        dbuser='docker',
        dbpassword='docker',
        env_vars={},
    ):
        """
        Constructor of PostgresDocker.
        """
        # type: () -> None
        log = logging.getLogger(__name__)  # type: logging.Logger
        base_url = 'unix://var/run/docker.sock'
        self.docker_client = docker.DockerClient(base_url=base_url)
        self.docker_api_client = docker.APIClient(base_url=base_url)
        self.image_name = image_name
        self.dockerfile_path = dockerfile_path
        self.container_name = container_name
        self.dbname = dbname
        self.dbuser = dbuser
        self.dbpassword = dbpassword
        self.env_vars = env_vars
        log.info("Trying to build/update the docker image")

        try:
            if self.dockerfile_path:
                for build_output in self.docker_client.images.build(
                    path=self.dockerfile_path, tag=image_name, rm=True
                ):
                    log.debug(build_output)

        except docker.errors.APIError as e:
            if e.is_server_error():
                log.warn("[docker server error] A server error occcured, maybe " "missing internet connection?")
                log.warn("[docker server error] Details: {}".format(e))
                log.warn(
                    "[docker server error] Checking if '{}' docker image " "is already built".format(image_name)
                )
                self.docker_client.images.get(image_name)
                log.warn(
                    "[docker server error] Going on, as '{}' docker image " "is already built".format(image_name)
                )
            else:
                raise

        self.container = self.docker_client.containers.create(
            image_name, name=self.container_name, environment=self.env_vars
        )
        log.info("docker id is {}".format(self.container.id))
        log.info("starting the temporary docker")
        self.container.start()
        self.ip_addr = (
            self.docker_api_client.inspect_container(self.container.id)
            .get('NetworkSettings', {})
            .get('IPAddress')
        )

        if not self.ip_addr:
            log.error("temporary docker {} not started".format(self.container.id))
            exit(1)

        # we poll to ensure that the database is ready
        self.test_db_cnx()

    def close(self):
        # type: () -> None
        """
        Terminate the Docker and clean it.
        """
        logging.getLogger(__name__).info("stopping the temporary docker")
        self.container.stop()

        logging.getLogger(__name__).info("removing the temporary docker")
        self.container.remove(v=True)

        # test to be sure the docker is removed at the end
        try:
            self.docker_client.containers.get(self.container.id)
        except docker.errors.NotFound:
            logging.getLogger(__name__).info("the container is properly removed")
        else:
            logging.getLogger(__name__).error("something is strange, the container is still there ...")
            exit(1)

    def get_db_params(self):
        # type: () -> DbParams
        """
        Create the connection parameters of the database.
        Default user and password are "docker" and default database is "postgres".

        :return: the DbParams for the database of the Docker
        """
        return DbParams(self.ip_addr, self.dbname, self.dbuser, self.dbpassword)

    @retry(stop_max_delay=10000, wait_fixed=100, retry_on_exception=lambda e: isinstance(e, Exception))
    def test_db_cnx(self):
        # type: () -> None
        """
        Test the connection to the database.
        """
        params = self.get_db_params()
        psycopg2.connect(database=params.dbname, user=params.user, password=params.password, host=params.host)


def PostgisDocker():
    return DockerWrapper(
        image_name='postgis:2.1',
        dockerfile_path='github.com/CanalTP/docker-postgis.git',
        dbname='postgres',
        dbuser='docker',
        dbpassword='docker',
    )


def PostgresDocker():
    env_vars = {'POSTGRES_DB': 'tyr_test', 'POSTGRES_USER': 'postgres', 'POSTGRES_PASSWORD': 'postgres'}
    return DockerWrapper(
        image_name='postgres:9.4',
        container_name='tyr_test_postgres',
        dbname=env_vars['POSTGRES_DB'],
        dbuser=env_vars['POSTGRES_USER'],
        dbpassword=env_vars['POSTGRES_PASSWORD'],
        env_vars=env_vars,
    )
