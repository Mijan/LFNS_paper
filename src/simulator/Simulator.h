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
#include "StoppingCriterion.h"
#include "../base/MathUtils.h"

namespace simulator {
    typedef std::function<void(double final_t)> SimulationFct;
    typedef std::shared_ptr<SimulationFct> SimulationFct_ptr;

    typedef std::function<void(std::vector<double> &state, double &t)> ResetFct;
    typedef std::shared_ptr<ResetFct> ResetFct_ptr;

    class Simulator {
    public:
        Simulator()
                : _stopping_criterions(), _discont_times(), _discont_it(_discont_times.end()), _states_ptr(nullptr),
                  _t_ptr(nullptr) {}

        virtual ~Simulator() {}


        virtual void addStoppingCriterion(StoppingFct_ptr stopping_criterion) {
            _stopping_criterions.push_back(stopping_criterion);
        }

        virtual SimulationFct_ptr getSimulationFct() = 0;

        virtual void simulate(double final_time) = 0;

        virtual ResetFct_ptr getResetFct() = 0;

        virtual void reset(std::vector<double> &state, double &t) {
            _states_ptr = &state;
            _t_ptr = &t;
            if (!_discont_times.empty() && t < _discont_times.back()) {
                _discont_it = base::MathUtils::binarySearchLatter(_discont_times.begin(), _discont_times.end() - 1, t);
            }
        }

        virtual void clearStoppingCriterions() { _stopping_criterions.clear(); }

        void setDiscontTimes(std::vector<double> discont_times) {
            _discont_times = discont_times;
            _discont_it = discont_times.begin();
        }

    protected:

        StoppingCriterions _stopping_criterions;

        std::vector<double> _discont_times;
        std::vector<double>::iterator _discont_it;

        std::vector<double> *_states_ptr;
        double *_t_ptr;
    };

    typedef std::shared_ptr<Simulator> Simulator_ptr;
}


#endif //LFNS_SIMULATOR_H
