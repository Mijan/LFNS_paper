//
// Created by jan on 10/10/18.
//

#ifndef LFNS_STOPPINGCRITERION_H
#define LFNS_STOPPINGCRITERION_H


#include <vector>
#include <functional>
#include <memory>

namespace simulator {
    typedef std::function<bool()> StoppingFct;
    typedef std::shared_ptr<StoppingFct> StoppingFct_ptr;

    class StoppingCriterions {
    public:
        bool processStopped() {
            if (_stopping_criterions.empty()) { return false; }
            bool stopped = false;
            for (StoppingFct_ptr stopping_criterion: _stopping_criterions) {
                if ((*stopping_criterion)()) { stopped = true; }
            }
            return stopped;
        }

        void push_back(StoppingFct_ptr stopping_criterion) { _stopping_criterions.push_back(stopping_criterion); }

        void clear() { _stopping_criterions.clear(); }

    private:
        std::vector<StoppingFct_ptr> _stopping_criterions;
    };

    class StoppingCrietrionHybridSimulation {
    public:
        StoppingCrietrionHybridSimulation() : _state_ptr(), _state_initial_value() {}

        virtual ~StoppingCrietrionHybridSimulation() {}

        virtual bool operator()() {
            for (std::size_t i = 0; i < _state_ptr.size(); i++) {
                if (_state_initial_value[i] != *_state_ptr[i]) { return true; }
            }
            return false;
        }

        void setStatePtr(std::vector<double *> state_ptr) {
            _state_ptr = state_ptr;
            _state_initial_value.resize(_state_ptr.size());
        }

        void memorizeCurrentState() {
            for (std::size_t i = 0; i < _state_ptr.size(); i++) {
                _state_initial_value[i] = *_state_ptr[i];
            }
        }

    private:
        std::vector<double *> _state_ptr;
        std::vector<double> _state_initial_value;

    };
}


#endif //LFNS_STOPPINGCRITERION_H
