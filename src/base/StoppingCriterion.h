//
// Created by jan on 10/10/18.
//

#ifndef LFNS_STOPPINGCRITERION_H
#define LFNS_STOPPINGCRITERION_H


#include <vector>
#include <functional>
#include <memory>

namespace base {
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

//    class StoppingCrietrionHybridSimulation : public StoppingCriterion {
//    public:
//        StoppingCrietrionHybridSimulation();
//
//        virtual ~StoppingCrietrionHybridSimulation();
//
//        virtual bool processStopped();
//
//        void setStatePtr(std::vector<double *> state_ptr);
//
//        void memorizeCurrentState();
//
//    private:
//        std::vector<double *> _state_ptr;
//        std::vector<double> _state_initial_value;
//
//    };
//
//    typedef std::shared_ptr<StoppingCrietrionHybridSimulation> StoppingCrietrionHybridSimulation_ptr;
}


#endif //LFNS_STOPPINGCRITERION_H
