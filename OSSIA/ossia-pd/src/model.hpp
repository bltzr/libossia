#pragma once

#include "ossia_obj_base.hpp"
#include "ossia-pd.hpp"
#include "device.hpp"

namespace ossia { namespace pd {

struct t_model : t_obj_base
{
    bool register_node(ossia::net::node_base*  node);
    bool do_registration(ossia::net::node_base*  node);
    bool unregister();

    static ossia::safe_vector<t_model*>& quarantine(){
        static ossia::safe_vector<t_model*> quarantine;
        return quarantine;
    }
    t_symbol* x_tags;
    t_symbol* x_description;

    void isDeleted(const ossia::net::node_base& n)
    {
        if (!x_dead){
            x_node->about_to_be_deleted.disconnect<t_model, &t_model::isDeleted>(this);
            x_node = nullptr;
            obj_quarantining<t_model>(this);
        }
    }

    static ossia::safe_vector<t_model*>& rename(){
        static ossia::safe_vector<t_model*> rename;
        return rename;
    }

    bool isRenamed(t_model* x){
        return x->rename().contains(x);
    }

    void renaming(t_model* x){
        if ( !isRenamed(x) ) x->rename().push_back(x);
    }

    void derenaming(t_model* x){
        x->rename().remove_all(x);
    }
};

} } // namespace
