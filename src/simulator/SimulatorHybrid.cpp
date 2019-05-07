//
// Created by jan on 06/05/19.
//

#include "SimulatorHybrid.h"

namespace simulator {
    using namespace std::placeholders;

    SimulatorHybrid::SimulatorHybrid(SimulatorSsa_ptr simulator_ssa, SimulatorOde_ptr simulator_ode,
                                     std::vector<int> &stoch_state_indices, std::vector<int> &det_state_indices)
            : Simulator(), _simulator_ode(simulator_ode), _simulator_ssa(simulator_ssa),
              _stoch_state_indices(stoch_state_indices), _det_state_indices(det_state_indices),
              _determining_states_indices() {
        for (int i = 0; i < stoch_state_indices.size(); i++) {
            int stoch_index = stoch_state_indices[i];
            std::vector<int>::iterator it = std::find(det_state_indices.begin(), det_state_indices.end(),
                                                      stoch_index);
            if (it != det_state_indices.end()) {
                _determining_states_indices.push_back(*it);
            }
        }
    }

    SimulatorHybrid::~SimulatorHybrid() {}

    SimulationFct_ptr SimulatorHybrid::getSimulationFct() {
        return std::make_shared<SimulationFct>(std::bind(&SimulatorHybrid::simulate, this, _1));
    }

    void SimulatorHybrid::simulate(double final_time) {
        double time_det = *_t_ptr;
        double time_stoch = *_t_ptr;
        _simulator_ssa->reset(_stoch_state, time_stoch);
        _simulator_ode->reset(_det_state, time_det);

        while (*_t_ptr < final_time) {
            _stopping_criterion_hybrid.memorizeCurrentState();
            _simulator_ssa->simulate(final_time);
            _simulator_ode->simulate(time_stoch);
            *_t_ptr = time_stoch;
            for (int &index: _determining_states_indices) {
                _det_state[index] = _stoch_state[index];
                _simulator_ode->reset(_det_state, time_det);
            }
            for(int &index: _det_state_indices){
                _stoch_state[index] = _det_state[index];
            }
            if (_stopping_criterions.processStopped()) {
                updateOutputState(time_stoch);
                return;
            }
        }

        updateOutputState(time_stoch);
    }

    void SimulatorHybrid::updateOutputState(double output_time) {
        *_states_ptr = _stoch_state;
        *_t_ptr = output_time;
    }

    ResetFct_ptr SimulatorHybrid::getResetFct() {
        return std::make_shared<ResetFct>(std::bind(&SimulatorHybrid::reset, this, _1, _2));
    }

    void SimulatorHybrid::reset(std::vector<double> &state, double &t) {
        Simulator::reset(state, t);
        _det_state.resize(state.size());
        _stoch_state.resize(state.size());
        std::vector<double *> deteriming_ptr;
        for (int i = 0; i < state.size(); i++) {
            _det_state[i] = state[i];
            _stoch_state[i] = state[i];

            if (std::find(_determining_states_indices.begin(), _determining_states_indices.end(), i) !=
                _determining_states_indices.end()) {
                deteriming_ptr.push_back(&state[i]);
            }
        }
        _stopping_criterion_hybrid.setStatePtr(deteriming_ptr);
    }
}