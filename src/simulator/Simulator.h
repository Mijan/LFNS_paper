//
// Created by jan on 10/10/18.
//

#ifndef LFNS_SIMULATOR_H
#define LFNS_SIMULATOR_H

#include <memory>
#include <cstring>
#include <iostream>
#include <sstream>
#include <functional>
#include "../base/StoppingCriterion.h"

namespace simulator {
    typedef std::function<void(std::vector<double> &state, double &t, double final_t,
                               const std::vector<double> &theta)> SimulationFct;
    typedef std::shared_ptr<SimulationFct> SimulationFct_ptr;

    typedef std::function<double(std::vector<double> &state, double t)> RootFct;
    typedef std::shared_ptr<RootFct> RootFct_ptr;

    class Simulator {
    public:
        Simulator() : _stopping_criterions(), _root_fct(nullptr), _root_found(false) {}

        virtual ~Simulator() {}


        virtual void addStoppingCriterion(base::StoppingFct_ptr stopping_criterion) {
            _stopping_criterions.push_back(stopping_criterion);
        }

        virtual SimulationFct_ptr getSimulationFct() = 0;

        virtual void clearStoppingCriterions() { _stopping_criterions.clear(); }

        virtual void setRootFunction(RootFct_ptr root_fct) { _root_fct = root_fct; }

    protected:

        base::StoppingCriterions _stopping_criterions;
        RootFct_ptr _root_fct;
        bool _root_found;
    };

    typedef std::shared_ptr<Simulator> Simulator_ptr;
}


#endif //LFNS_SIMULATOR_H
