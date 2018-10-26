//
// Created by jan on 10/10/18.
//

#ifndef LFNS_SIMULATORSSA_H
#define LFNS_SIMULATORSSA_H

#include <cfloat>
#include "../base/RandomDistributions.h"
#include "../base/EigenMatrices.h"
#include "../base/StoppingCriterion.h"
#include "Simulator.h"

namespace simulator {

    typedef std::function<void(std::vector<double> &propensities, std::vector<double> &state, double t,
                               const std::vector<double> &theta)> PropensityFct;
    typedef std::shared_ptr<PropensityFct> PropensityFct_ptr;

    typedef std::function<void(std::vector<double> &state, int reaction_index)> ReactionFct;
    typedef std::shared_ptr<ReactionFct> ReactionFct_ptr;

    class SimulatorSsa : public Simulator{
    public:
        SimulatorSsa(base::RngPtr rng, PropensityFct_ptr propensity_fct, ReactionFct_ptr reaction_fct,
                     int num_reactions);

        virtual ~SimulatorSsa();

        virtual void
        simulate(std::vector<double> &state, double &t, double final_time, const std::vector<double> &theta);

        virtual SimulationFct_ptr getSimulationFct();

        void
        simulateReaction(std::vector<double> &state, double &t, double final_time, const std::vector<double> &theta);

    private:
        base::RngPtr _rng;
        base::UniformRealDistribution _dis;
        double _time_to_next_reaction;

        PropensityFct_ptr _propensity_fct;
        std::vector<double> _propensities;
        ReactionFct_ptr _reaction_fct;
        int _root_sign;

        void
        _simulateSystem(std::vector<double> &state, double &t, double final_time, const std::vector<double> &theta);

        double _getTimeToNextReaction(double prop_sum);

        void _checkRoot(std::vector<double> &state, double &t);

        double _findRootTime(std::vector<double> &state, double time_a, double time_b, int sgn_a);
    };

    typedef std::shared_ptr<SimulatorSsa> SimulatorSsa_ptr;
}


#endif //LFNS_SIMULATORSSA_H
