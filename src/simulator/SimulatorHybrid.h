//
// Created by jan on 06/05/19.
//

#ifndef LFNS_SIMULATORHYBRID_H
#define LFNS_SIMULATORHYBRID_H


#include <map>
#include "SimulatorSsa.h"
#include "SimulatorOde.h"

namespace simulator {
    class SimulatorHybrid : public Simulator {
    public:
        SimulatorHybrid(SimulatorSsa_ptr simulator_ssa, SimulatorOde_ptr simulator_ode,
                        std::vector<int> &stoch_state_indices,
                        std::vector<int> &det_state_indices);

        virtual ~SimulatorHybrid();

        virtual SimulationFct_ptr getSimulationFct();

        void simulate(double final_time) override;

        virtual ResetFct_ptr getResetFct();

        virtual void reset(std::vector<double> &state, double &t);

    private:
        SimulatorOde_ptr _simulator_ode;
        SimulatorSsa_ptr _simulator_ssa;

        std::vector<int> _stoch_state_indices;
        std::vector<int> _det_state_indices;
        std::vector<int> _determining_states_indices;

        std::vector<double> _stoch_state;
        std::vector<double> _det_state;

        StoppingCrietrionHybridSimulation _stopping_criterion_hybrid;

        void updateOutputState(double output_time);
    };

    typedef std::shared_ptr<SimulatorHybrid> SimulatorHybrid_ptr;

}
#endif //LFNS_SIMULATORHYBRID_H
