#include "data.h"
#include "network.h"


using namespace network;

unsigned int network::get_sp_idx(unsigned int spid, navitia::type::Data &data) {
    return data.pt_data.stop_areas.size() +spid;
}

unsigned int network::get_rp_idx(unsigned int rpid, navitia::type::Data &data) {
    return data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + rpid;
}

unsigned int network::get_ta_idx(unsigned int stid, navitia::type::Data &data) {
    return data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size() + stid;
}

unsigned int network::get_td_idx(unsigned int stid, navitia::type::Data &data) {
    return data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size() + data.pt_data.stop_times.size() +  stid;
}

unsigned int network::get_tc_idx(unsigned int stid, navitia::type::Data &data) {
    return data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.stop_times.size()*2 +  stid;
}

unsigned int network::get_ho_idx(unsigned int stid, navitia::type::Data &data) {
    return data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + stid;
}


idx_t network::get_idx(unsigned int idx, navitia::type::Data &data) {
    if(idx < data.pt_data.stop_areas.size())
        return idx;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size()))
        return (idx - data.pt_data.stop_areas.size());
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size()))
        return (idx - data.pt_data.stop_areas.size() - data.pt_data.stop_points.size());
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size() + data.pt_data.stop_times.size()))
        return (idx - data.pt_data.stop_areas.size() - data.pt_data.stop_points.size() - data.pt_data.route_points.size());
    else
        return (idx - data.pt_data.stop_areas.size() - data.pt_data.stop_points.size() - data.pt_data.route_points.size() - data.pt_data.stop_times.size());
}

uint32_t network::get_saidx(unsigned int idx, navitia::type::Data &data) {
    if(idx < data.pt_data.stop_areas.size())
        return idx;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size()))
        return data.pt_data.stop_points.at(idx - data.pt_data.stop_areas.size()).stop_area_idx;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size()))
        return data.pt_data.stop_points.at(data.pt_data.route_points.at(idx - data.pt_data.stop_areas.size() - data.pt_data.stop_points.size()).stop_point_idx).stop_area_idx;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size() + data.pt_data.stop_times.size()))

        return data.pt_data.stop_points.at(data.pt_data.route_points.at(data.pt_data.stop_times.at((idx - data.pt_data.stop_areas.size() - data.pt_data.stop_points.size() - data.pt_data.route_points.size())).route_point_idx).stop_point_idx).stop_area_idx;
    else
        return data.pt_data.stop_points.at(data.pt_data.route_points.at(data.pt_data.stop_times.at((idx - data.pt_data.stop_areas.size() - data.pt_data.stop_points.size() - data.pt_data.route_points.size() - data.pt_data.stop_times.size())).route_point_idx).stop_point_idx).stop_area_idx;
}

node_type2 network::get_n_type(unsigned int idx, navitia::type::Data &data) {
    if(idx < data.pt_data.stop_areas.size())
        return SA;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size()))
        return SP;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size()))
        return RP;
    else if(idx < (data.pt_data.stop_areas.size() + data.pt_data.stop_points.size() + data.pt_data.route_points.size() + data.pt_data.stop_times.size()))
        return TA;
    else
        return TD;
    return VOIDN;
}

bool network::is_passe_minuit(uint32_t debut, uint32_t fin, navitia::type::Data &data) {
    uint32_t debut_t, fin_t;

    if(get_n_type(debut, data) == TA)
        debut_t = data.pt_data.stop_times.at(get_idx(debut, data)).arrival_time;
    else if(get_n_type(debut, data) == TD)
        debut_t = data.pt_data.stop_times.at(get_idx(debut, data)).departure_time;
    else
        return false;


    if(get_n_type(fin, data) == TA)
        fin_t = data.pt_data.stop_times.at(get_idx(fin, data)).arrival_time;
    else if(get_n_type(fin, data) == TD)
        fin_t = data.pt_data.stop_times.at(get_idx(fin, data)).departure_time;
    else
        return false;

    //    return ((debut_t <= 86400) & (fin_t > 86400)) || (debut_t > fin_t);
    return (debut_t % 86400) > (fin_t % 86400);
}

int network::get_validity_pattern_idx(navitia::type::ValidityPattern vp, navitia::type::Data &data) {
    BOOST_FOREACH(navitia::type::ValidityPattern vpi, data.pt_data.validity_patterns) {

        unsigned int i=0;
        bool test = true;
        while((i<366) & test) {
            test = vpi.days[i] == vp.days[i];
            ++i;
        }

        if(test)
            return vpi.idx;
    }
    vp.idx = data.pt_data.validity_patterns.size();

    data.pt_data.validity_patterns.push_back(vp);


    return vp.idx;
}

navitia::type::ValidityPattern * network::decalage_pam(navitia::type::ValidityPattern &vp) {
    navitia::type::ValidityPattern *vpr = new navitia::type::ValidityPattern(vp.beginning_date);

    for(unsigned int i=0; i < 366; i++) {
        vpr->days[i+1] = vp.days[i];
    }
    vpr->days[0] = vp.days[365];

    return vpr;
}

void network::charger_graph4(navitia::type::Data &data, NW &g) {

    idxv idx_v = get(&VertexDesc::idx, g);
    vertex_t tav, tdv, tdp, rpv;
    edge_t e;
    bool bo;
    int min_corresp = 5*60; //Temps minimal de correspondance en secondes

    //Creation des stop areas
    BOOST_FOREACH(navitia::type::StopArea sai, data.pt_data.stop_areas) {
        g[sai.idx] = VertexDesc(sai.idx, sa);
    }
    //Creation des stop points et creations lien SA->SP
    BOOST_FOREACH(navitia::type::StopPoint spi, data.pt_data.stop_points) {
        g[get_sp_idx(spi.idx, data)] = VertexDesc(spi.idx, sp);
        add_edge(spi.stop_area_idx, get_sp_idx(spi.idx, data), EdgeDesc(1, sasp), g);
    }

    //Création des liens SP->RP
    BOOST_FOREACH(navitia::type::RoutePoint rpi, data.pt_data.route_points) {
        g[get_rp_idx(rpi.idx, data)] = VertexDesc(rpi.idx, rp);
        add_edge(get_sp_idx(rpi.stop_point_idx, data), get_rp_idx(rpi.idx, data), EdgeDesc(1, sprp), g);
    }

    BOOST_FOREACH(navitia::type::VehicleJourney vj,  data.pt_data.vehicle_journeys) {
        tdv = 0;
        BOOST_FOREACH(unsigned int stid, vj.stop_time_list) {
            rpv = get_rp_idx(data.pt_data.stop_times.at(stid).route_point_idx, data);

            //Création des ta et td
            g[get_ta_idx(stid, data)] = VertexDesc(stid, data.pt_data.stop_points.at(data.pt_data.route_points.at(data.pt_data.stop_times.at(stid).route_point_idx).stop_point_idx).stop_area_idx, ta);
            g[get_td_idx(stid, data)] = VertexDesc(stid, data.pt_data.stop_points.at(data.pt_data.route_points.at(data.pt_data.stop_times.at(stid).route_point_idx).stop_point_idx).stop_area_idx, td);

            //ta
            tav = get_ta_idx(stid, data) ;
            if(tdv != 0) {
                add_edge(tdv, tav, EdgeDesc(vj.idx, data.pt_data.stop_times.at(get(idx_v, tdv)).departure_time, data.pt_data.stop_times.at(stid).arrival_time, tdta, g[tav].saidx), g);
            }

            //td
            tdv = get_td_idx(stid, data);

            //edges
            add_edge(tav, tdv, EdgeDesc(vj.idx, data.pt_data.stop_times.at(stid).arrival_time, data.pt_data.stop_times.at(stid).departure_time, tatd, g[tdv].saidx), g);
            add_edge(rpv, tav, EdgeDesc(vj.idx, data.pt_data.stop_times.at(stid).arrival_time - 1, data.pt_data.stop_times.at(stid).arrival_time, rpta, g[tav].saidx), g);
        }



    }

    typedef std::map<vertex_t, vertex_t> map_t;
    std::set<vertex_t, sort_edges> verticesTD = std::set<vertex_t, sort_edges>(sort_edges(g, data));
    map_t verticesRP = map_t();
    std::set<vertex_t, sort_edges> verticesTA = std::set<vertex_t, sort_edges>(sort_edges(g, data));



    for(unsigned int sav = 0; sav<data.pt_data.stop_areas.size();++sav) {
        //On itère sur les stop points
        verticesRP.clear();
        verticesTA.clear();
        BOOST_FOREACH(edge_t e1, out_edges(sav, g)) {
            if(g[target(e1, g)].nt == sp) {
                BOOST_FOREACH(edge_t e2, out_edges(target(e1, g), g)) {
                    if(g[target(e2, g)].nt == rp) {
                        verticesTD.clear();
                        //On itère sur les ta
                        BOOST_FOREACH(edge_t e3, out_edges(target(e2, g), g)) {
                            if(g[target(e3, g)].nt == ta) {
                                verticesTA.insert(target(e3, g));
                                //On itère sur les td
                                BOOST_FOREACH(edge_t e4, out_edges(target(e3, g), g)) {
                                    if(g[target(e4, g)].nt == td) {
                                        if(data.pt_data.stop_times.at(g[target(e4, g)].idx).route_point_idx == g[target(e2, g)].idx) {
                                            verticesTD.insert(target(e4, g));
                                        }
                                    }
                                }
                            }
                        }


                        if(verticesTD.size() > 0) {
                            verticesRP.insert(std::pair<vertex_t, vertex_t>(target(e1, g), *verticesTD.begin()));

                            //On lie les td d'un même RP entre eux
                            if(verticesTD.size() > 1) {
                                tdp = num_vertices(g) + 1;
                                BOOST_FOREACH(vertex_t tdv, verticesTD) {
                                    if(tdp != (num_vertices(g) + 1)) {
                                        add_edge(tdp, tdv, EdgeDesc(0,data.pt_data.stop_times.at(g[tdp].idx).departure_time, data.pt_data.stop_times.at(g[tdv].idx).departure_time, tdtd, g[tdv].saidx), g);
                                    }
                                    tdp = tdv;
                                }

                            }
                        }
                    }
                }
            }
        }
        //On relie chaque ta au premier td possible pour chaque rp du sa
        BOOST_FOREACH(vertex_t tav, verticesTA) {
            BOOST_FOREACH(map_t::value_type &tdvi, verticesRP) {
                vertex_t tdv = tdvi.second;
                if((tdv != 0) & (g[tdv].nt == td)) {
                    bo = true;
                    //On recherche le premier td possible
                    while(((data.pt_data.stop_times.at(g[tdv].idx).departure_time - data.pt_data.stop_times.at(g[tav].idx).arrival_time) < min_corresp) & bo) {
                        bo = false;
                        BOOST_FOREACH(edge_t e3, out_edges(tdv, g)) {
                            if((g[target(e3, g)].nt == td) & (g[target(e3, g)].saidx == sav))  {
                                tdv = target(e3, g);
                                bo = true;
                            }
                        }
                    }

                    if(bo) {
                        verticesRP[tdvi.first] = tdv;
                        add_edge(tav, tdv, EdgeDesc(0, data.pt_data.stop_times.at(g[tav].idx).arrival_time, data.pt_data.stop_times.at(g[tdv].saidx).departure_time, tatdc), g);
                    } else {
                        //C'est qu'il y en a plus de possible
                        verticesRP[tdvi.first] = 0;
                    }
                } else {
                    tdv = 0;
                }
            }
        }
    }
}



void network::charger_graph5(navitia::type::Data &data, NW2 &g) {
    vertex_t tav, tdv, tdp, rpv;
    bool bo;
    int min_corresp = 5*60; //Temps minimal de correspondance en secondes
    navitia::type::ValidityPattern vptemp;

    //Creation des stop areas
    BOOST_FOREACH(navitia::type::StopArea sai, data.pt_data.stop_areas) {
        g[sai.idx] = VertexDesc2(sai.idx);
    }
    //Creation des stop points et creations lien SA->SP
    BOOST_FOREACH(navitia::type::StopPoint spi, data.pt_data.stop_points) {
        g[get_sp_idx(spi.idx, data)] = VertexDesc2(spi.idx);
        add_edge(spi.stop_area_idx, get_sp_idx(spi.idx, data), EdgeDesc2(spi.stop_area_idx, get_sp_idx(spi.idx, data), 0, false), g);
    }

    //Création des liens SP->RP
    BOOST_FOREACH(navitia::type::RoutePoint rpi, data.pt_data.route_points) {
        g[get_rp_idx(rpi.idx, data)] = VertexDesc2(rpi.idx);
        add_edge(get_sp_idx(rpi.stop_point_idx, data), get_rp_idx(rpi.idx, data), EdgeDesc2(rpi.stop_point_idx, rpi.idx, 0, false), g);
    }


    BOOST_FOREACH(navitia::type::VehicleJourney vj,  data.pt_data.vehicle_journeys) {
        tdv = 0;
        unsigned int vp_idx = vj.validity_pattern_idx;
        BOOST_FOREACH(unsigned int stid, vj.stop_time_list) {
            rpv = get_rp_idx(data.pt_data.stop_times.at(stid).route_point_idx, data);

            //Création des ta et td;
            g[get_ta_idx(stid, data)] = VertexDesc2(stid);
            g[get_td_idx(stid, data)] = VertexDesc2(stid);

            //ta
            tav = get_ta_idx(stid, data) ;
            if(tdv != 0) {
                if(is_passe_minuit(tdv, tav, data)) {
                    vptemp = *(decalage_pam(data.pt_data.validity_patterns.at(vj.validity_pattern_idx)));
                    vp_idx = get_validity_pattern_idx(vptemp, data);
                }
                add_edge(tdv, tav, EdgeDesc2(tdv, tav, vp_idx, true), g);
            }

            //td
            tdv = get_td_idx(stid, data);
            //edges


            if(is_passe_minuit(tav, tdv, data)) {
                vptemp = *decalage_pam(data.pt_data.validity_patterns.at(vj.validity_pattern_idx));
                vp_idx = get_validity_pattern_idx(vptemp, data);
            }
            add_edge(tav, tdv, EdgeDesc2(tav, tdv, vp_idx, true), g);

            add_edge(rpv, tav, EdgeDesc2(rpv, tav, 0, false), g);
        }
    }

    typedef std::map<vertex_t2, vertex_t2> map_t;
    typedef std::map<vertex_t2, uint32_t> mapam_t;
    std::set<vertex_t2, sort_edges2> verticesTD = std::set<vertex_t2, sort_edges2>(sort_edges2(g, data));
    map_t verticesRPw = map_t();
    map_t verticesRPi = map_t();
    map_t verticesRPp = map_t();
    std::set<vertex_t2, sort_edges2> verticesTA = std::set<vertex_t2, sort_edges2>(sort_edges2(g, data));


    for(unsigned int sav = 0; sav<data.pt_data.stop_areas.size();++sav) {
        //On itère sur les stop points
        verticesRPw.clear();
        verticesRPi.clear();
        verticesRPp.clear();
        verticesTA.clear();
        BOOST_FOREACH(edge_t2 e1, out_edges(sav, g)) {
            if(get_n_type(target(e1, g), data)== SP) {
                BOOST_FOREACH(edge_t2 e2, out_edges(target(e1, g), g)) {
                    if(get_n_type(target(e2, g), data) == RP) {
                        verticesTD.clear();
                        //On itère sur les ta
                        BOOST_FOREACH(edge_t2 e3, out_edges(target(e2, g), g)) {
                            if(get_n_type(target(e3, g), data) == TA) {
                                verticesTA.insert(target(e3, g));
                                //On itère sur les td
                                BOOST_FOREACH(edge_t2 e4, out_edges(target(e3, g), g)) {
                                    if(get_n_type(target(e4, g), data) == TD) {
                                        if(data.pt_data.stop_times.at(g[target(e4, g)].idx).route_point_idx == g[target(e2, g)].idx) {
                                            verticesTD.insert(target(e4, g));
                                        }
                                    }
                                }
                            }
                        }

                        if(verticesTD.size() > 0) {
                            verticesRPw.insert(std::pair<vertex_t2, vertex_t2>(target(e2, g), *verticesTD.begin()));
                            verticesRPi.insert(std::pair<vertex_t2, vertex_t2>(target(e2, g), *verticesTD.begin()));
                            verticesRPp.insert(std::pair<vertex_t2, uint32_t>(target(e2, g), 0));
                            //On lie les td d'un même RP entre eux
                            if(verticesTD.size() > 1) {
                                tdp = num_vertices(g) + 1;
                                BOOST_FOREACH(vertex_t2 tdv, verticesTD) {
                                    if(tdp != (num_vertices(g) + 1)) {
                                        add_edge(tdp, tdv, EdgeDesc2(tdp, tdv, 0, false), g);
                                    }
                                    tdp = tdv;
                                }

                            }
                        }
                    }
                }
            }
        }
        //On relie chaque ta au premier td possible pour chaque rp du sa
        BOOST_FOREACH(vertex_t2 tav, verticesTA) {
            BOOST_FOREACH(map_t::value_type &tdvi, verticesRPw) {
                if(data.pt_data.stop_times.at(get_idx(tav, data)).route_point_idx != tdvi.first) {
                    vertex_t2 tdv = tdvi.second;
                    if((tdv != 0) & (get_n_type(tdv, data) == TD)) {
                        bo = true;
                        //On recherche le premier td possible
                        while(((data.pt_data.stop_times.at(g[tdv].idx).departure_time - data.pt_data.stop_times.at(g[tav].idx).arrival_time + verticesRPp[tdvi.first]) < min_corresp) & bo) {
                            bo = false;
                            BOOST_FOREACH(edge_t2 e3, out_edges(tdv, g)) {
                                if((get_n_type(target(e3, g), data) == TD) & (get_saidx(target(e3, g), data) == sav))  {
                                    tdv = target(e3, g);
                                    bo = true;
                                }
                            }
                            //C'est qu'il faut faire un passe minuit
                            if(!bo & (verticesRPp[tdvi.first] == 0)) {
                                verticesRPp[tdvi.first] = 3600 * 24 ;
                                verticesRPw[tdvi.first] = verticesRPi[tdvi.first];
                                tdv = verticesRPi[tdvi.first];
                                bo = true;
                            }
                        }

                        verticesRPw[tdvi.first] = tdv;
                        if(!edge(tav, tdv, g).second) {
                            add_edge(tav, tdv, EdgeDesc2(tav, tdv, 0, false), g);
                        }
                    } else {
                        tdv = 0;
                    }
                }
            }
        }
    }
}


etiquette network::calc_fin3(etiquette debut, EdgeDesc ed) {


    if(debut == etiquette::max())
        return etiquette::max();
    else
        if(!debut.commence)
            if(ed.e_type == spta) {
                if(debut.arrivee <= ed.debut) {
                    debut.commence = true;
                    return debut;
                }
                else
                    return etiquette::max();
            }
            else
                return debut;
        else
            if(debut.arrivee <= ed.debut) {
                //                    if(data.pt_data.vehicle_journeys.at(ed.idx).validity_pattern_idx > data.pt_data.validity_patterns.size()
                //                       || data.pt_data.validity_patterns.at(data.pt_data.vehicle_journeys.at(ed.idx).validity_pattern_idx).check(day)) {
                etiquette retour;
                retour.commence = true;
                retour.temps = debut.temps + ed.fin - ed.debut;
                retour.arrivee = ed.fin;

                return retour;
                //                    }
                //                    else
                //                        return etiquettemax;
            }

            else
                return etiquette::max();
}


etiquette network::calc_fin4(etiquette debut, EdgeDesc ed) {
    etiquette retour;
    if(!debut.commence)
        if(ed.e_type == tdta) {
            if(debut.arrivee <= ed.debut) {
                retour.arrivee = ed.fin;
                retour.temps = ed.fin - ed.debut;
                retour.commence = true;
                return retour;
            }
            else
                return etiquette::max();
        }
        else {
            retour.arrivee = debut.arrivee;
            retour.commence = false;
            retour.temps = 0;
            return retour;
        }
    else {

        retour.commence = true;
        if(ed.cible != debut.cible)
            retour.temps = debut.temps + ed.fin - ed.debut;
        retour.arrivee = ed.fin;
        return retour;

    }

}






std::list<vertex_t> in_vertices2(vertex_t v, NW g) {

    boost::graph_traits<NW>::edge_iterator e, e_end;
    std::list<vertex_t> result;

    for(tie(e, e_end) = edges(g);e!=e_end;++e) {
        if(target(*e, g) == v)
            result.push_back(source(*e, g));
    }

    return result;
}

bool network::etape::operator ==(network::etape e2) {
    return (this->ligne == e2.ligne) & (this->descente == e2.descente);
}

bool network::etape::operator!=(network::etape e2) {
    return !((*this) == e2);
}

void network::parcours::ajouter_etape(idx_t ligne, idx_t depart) {
    this->etapes.push_front(network::etape(ligne, depart));
}

bool network::parcours::operator==(parcours i2) {
    if(this->etapes.size() != i2.etapes.size())
        return false;
    std::list<network::etape>::iterator it1, it2;
    for(it1 = this->etapes.begin(), it2 = i2.etapes.begin(); it1 != this->etapes.end(); ++it1, ++it2) {
        if((*it1) != (*it2))
            return false;
    }
    return true;
}

bool network::parcours::operator!=(network::parcours i2) {
    return !((*this) == i2);
}


