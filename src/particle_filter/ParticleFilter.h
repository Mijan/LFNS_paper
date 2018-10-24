//
// Created by jan on 11/10/18.
//

#ifndef LFNS_PARTICLEFILTER_H
#define LFNS_PARTICLEFILTER_H

#include <functional>
#include "SmcParticle.h"
#include "SmcParticleSet.h"
#include "../base/StoppingCriterion.h"

namespace particle_filter {

    typedef std::function<void(std::vector<double> &state, double &t, double final_t,
                               const std::vector<double> &theta)> SimulationFct;
    typedef std::shared_ptr<SimulationFct> SimulationFct_ptr;


    typedef std::function<double(const std::vector<double> &state, const std::vector<double> &data,
                                 const std::vector<double> &theta)> LikelihoodFct;
    typedef std::shared_ptr<LikelihoodFct> LikelihoodFct_ptr;

    typedef std::function<void(std::vector<double> *state_0, double *t_0,
                               const std::vector<double> &theta)> InitialStateFct;
    typedef std::shared_ptr<InitialStateFct> InitialStateFct_ptr;


    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class ParticleFilter {
    public:
        ParticleFilter(base::RngPtr rng, SimulationFct_ptr simulation_fct, LikelihoodFct_ptr likelihood_fct,
                       InitialStateFct_ptr initial_state_fct, int num_states, int num_particles = 1000);

        virtual ~ParticleFilter();

        virtual double
        computeLogLikelihood(const std::vector<std::vector<double> > &data, const std::vector<double> &times,
                             const std::vector<double> &theta);

        virtual double computeLogLikelihoodNoResampling(const std::vector<std::vector<double> > &data,
                                                        const std::vector<double> &times,
                                                        const std::vector<double> &theta);

        LogLikelihodEvalFct_ptr getLikelihoodEvaluationForData(const std::vector<std::vector<double> > *data,
                                                               const std::vector<double> *times);

        virtual double
        _runFilteringStep(const std::vector<double> &data, double final_time, const std::vector<double> &theta);

        void setThresholdPtr(double *threshold);

        virtual void addStoppingCriterion(base::StoppingFct_ptr stopping_criterion);


    protected:
        base::RngPtr _rng;
        SimulationFct_ptr _simulation_fct;
        LikelihoodFct_ptr _likelihood_fct;
        InitialStateFct_ptr _initial_state_fct;


        SmcParticleSet_ptr _smc_particles;
        std::vector<double> _log_likelihoods_tmps;
        double* _threshold_ptr;

        base::StoppingCriterions _stopping_criterions;
    };

    typedef std::shared_ptr<ParticleFilter> ParticleFilter_ptr;
}


#endif //LFNS_PARTICLEFILTER_H
