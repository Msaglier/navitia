/* Copyright © 2001-2015, Canal TP and/or its affiliates. All rights reserved.

This file is part of Navitia,
    the software to build cool stuff with public transport.

Hope you'll enjoy and contribute to this project,
    powered by Canal TP (www.canaltp.fr).
Help us simplify mobility and open public transport:
    a non ending quest to the responsive locomotion way of traveling!

LICENCE: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

Stay tuned using
twitter @navitia
IRC #navitia on freenode
https://groups.google.com/d/forum/navitia
www.navitia.io
*/
#pragma once
#include <boost/date_time/gregorian/gregorian.hpp>

namespace navitia {
namespace type {

struct ValidityPattern;
struct MetaData;
class TimeZoneHandler {
    /*
     * we store on the production period all the times changes,
     * the periods are represented by validity patterns
     *
     *
     *              production period
     *       [------------------------------------]
     *       [  DST    )[    non DST     )[   DST )
     * vp1:  ***********                  *********      <-- offset n
     * vp2:             ******************               <-- offset m
     */
    std::vector<std::pair<const ValidityPattern*, int16_t>> time_changes; //TODO remove*
    const MetaData* meta_data = nullptr;
public:
    TimeZoneHandler(const MetaData&, const std::map<int16_t, std::vector<boost::gregorian::date_period>>&);
    TimeZoneHandler() {}
    int16_t get_utc_offset(boost::gregorian::date day) const;
    int16_t get_utc_offset(int day) const;
    template <typename VP>
    int16_t get_utc_offset(const VP* vp) const;
};

class TimeZoneManager {
    const TimeZoneHandler* get_or_create(const MetaData*, const std::map<int16_t, std::vector<boost::gregorian::date_period>>&);
};

template <typename VP>
int16_t TimeZoneHandler::get_utc_offset(const VP* vp) const {
throw "bob";
}
}
}
