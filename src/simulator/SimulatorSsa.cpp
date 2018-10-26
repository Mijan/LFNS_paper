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
                                                    _reaction_fct(reaction_fct), _root_sign(0) {}

    SimulatorSsa::~SimulatorSsa() {}

    void
    SimulatorSsa::simulate(std::vector<double> &state, double &t, double final_time, const std::vector<double> &theta) {
        if (t < final_time) { _simulateSystem(state, t, final_time, theta); }
    }

    SimulationFct_ptr SimulatorSsa::getSimulationFct() {
        return std::make_shared<SimulationFct>(std::bind(&SimulatorSsa::simulate, this, _1, _2, _3, _4));
    }

    void SimulatorSsa::_simulateSystem(std::vector<double> &state, double &t, double final_time,
                                       const std::vector<double> &theta) {
        while (t < final_time) {
            simulateReaction(state, t, final_time, theta);
            if (_stopping_criterions.processStopped()) {
                return;
            }
        }
    }

    void SimulatorSsa::simulateReaction(std::vector<double> &state, double &t, double final_time,
                                        const std::vector<double> &theta) {

        if (_root_fct) {
            _root_sign = base::MathUtils::sgn<double>(
                    (*_root_fct)(state.data(), t));
        }

        (*_propensity_fct)(_propensities, state, t, theta);
        double prop_sum = 0;
        for (double &p: _propensities) { prop_sum += p; }

        _time_to_next_reaction = _getTimeToNextReaction(prop_sum);

        _checkRoot(state, t);
        if (_root_found) { return; }
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

    void SimulatorSsa::_checkRoot(std::vector<double> &state, double &t) {
        if (_root_fct) {
            double next_t = t;
            next_t += _time_to_next_reaction;
            double root_fct_t = (*_root_fct)(state.data(), next_t);
            if (base::MathUtils::sgn<double>(root_fct_t) != _root_sign) {
                _root_found = true;
                t = _findRootTime(state, t, next_t, _root_sign);
            } else {
                _root_found = false;
            }
        } else {
            _root_found = false;
        }
    }

    double SimulatorSsa::_findRootTime(std::vector<double> &system_state, double time_a, double time_b, int sgn_a) {
        double time_c = 0.5 * (time_a + time_b);

        double root_fct_c = (*_root_fct)(system_state.data(), time_c);
        int sgn_c = base::MathUtils::sgn<double>(root_fct_c);

        if (time_b - time_a < 1e-8) {
            if (sgn_a != sgn_c) {
                return time_c;
            } else { return _findRootTime(system_state, time_c, time_b, sgn_c); }
        }

        if (sgn_a != sgn_c) {
            return _findRootTime(system_state, time_a, time_c, sgn_a);
        } else {
            return _findRootTime(system_state, time_c, time_b, sgn_c);
        }
    }

}