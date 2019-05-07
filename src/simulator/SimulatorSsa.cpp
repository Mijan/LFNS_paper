//
// Created by jan on 10/10/18.
//

#include <iostream>
#include "SimulatorSsa.h"
#include "../base/MathUtils.h"

namespace simulator {
    using namespace std::placeholders;

    SimulatorSsa::SimulatorSsa(base::RngPtr rng, PropensityFct_ptr propensity_fct, ReactionFct_ptr reaction_fct,
                               int num_reactions) : Simulator(), _rng(rng), _dis(0, 1), _time_to_next_reaction(0.0),
                                                    _propensity_fct(propensity_fct), _propensities(num_reactions),
                                                    _reaction_fct(reaction_fct) {}

    SimulatorSsa::~SimulatorSsa() {}

    void SimulatorSsa::simulate(double final_time) {
        while (*_t_ptr < final_time) {
            simulateReaction(*_states_ptr, *_t_ptr, final_time);
            if (_stopping_criterions.processStopped()) { return; }
        }
    }

    SimulationFct_ptr SimulatorSsa::getSimulationFct() {
        return std::make_shared<SimulationFct>(std::bind(&SimulatorSsa::simulate, this, _1));
    }

    ResetFct_ptr SimulatorSsa::getResetFct() {
        return std::make_shared<ResetFct>(std::bind(&SimulatorSsa::reset, this, _1, _2));
    }

    void SimulatorSsa::simulateReaction(std::vector<double> &state, double &t, double final_time) {
        bool run_next_step = final_time > t;
        double T;
        while (run_next_step) {
            if (_discont_it != _discont_times.end() && *_discont_it <= final_time) { T = *_discont_it++; }
            else { T = final_time; }

            _simulateReaction(state, t, T);
            if (_stopping_criterions.processStopped()) { return; }
            run_next_step = final_time > t;
        }
    }

    void SimulatorSsa::_simulateReaction(std::vector<double> &state, double &t, double final_time) {

        (*_propensity_fct)(_propensities, state, t);
        double prop_sum = 0;
        for (double &p: _propensities) { prop_sum += p; }

        _time_to_next_reaction = _getTimeToNextReaction(prop_sum);
        if (t + _time_to_next_reaction > final_time) {
            t = final_time;
            return;
        }

        if (prop_sum == 0) {
            std::cerr << "Propensities are 0 at time " << t << " and state ";
            for (const double &state_entry: state) { std::cerr << state_entry << ", "; }
            std::cerr << "." << std::endl;
            return;
        }
        double r = _dis(*_rng);

        size_t next_reaction_index = 0;
        double cumsum = _propensities[0];
        if (cumsum < 0) {
            std::stringstream os;
            os << "Propensity for reaction " << next_reaction_index + 1 << " is negative: " << cumsum
               << std::endl;
            os << "Time: " << t << std::endl;
            throw std::runtime_error(os.str());
        }

        while (cumsum <= r * prop_sum) {
            double next_propensity = _propensities[next_reaction_index + 1];
            if (next_propensity < 0) {
                std::stringstream os;
                os << "Propensity for reaction " << next_reaction_index + 1 << " is negative: " << next_propensity
                   << std::endl;
                os << "Time: " << t << std::endl;
                throw std::runtime_error(os.str());
            }
            cumsum += next_propensity;
            next_reaction_index++;
        }

        (*_reaction_fct)(state, next_reaction_index);
        t += _time_to_next_reaction;
    }

    double SimulatorSsa::_getTimeToNextReaction(double prop_sum) {
        double r = _dis(*_rng);
        return (1 / prop_sum) * log(1 / r);
    }

}