//
// Created by jan on 25/02/19.
//

#ifndef MCMC_H
#define MCMC_H

#include <functional>
#include <memory>
#include "MCMCParticles.h"
#include "MCMCLogger.h"
#include "MCMCSampler.h"

namespace mcmc {
    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class MCMC {
    public:
        explicit MCMC(MCMCSettings &mcmc_settings, sampler::SamplerSettings &sampler_settings, base::RngPtr rng,  LogLikelihodEvalFct_ptr log_likelihood_evaluation);
        void runMCMC();
        void resumeRum(std::string previous_log_file);

    private:
        MCMCSettings _settings;
        base::RngPtr _rng;
        LogLikelihodEvalFct_ptr _log_likelihood_evaluation;

        MCMCSampler _sampler;
        MCMCParticles _particles;
        MCMCLogger _logger;

        bool _resume_run;

    };
}


#endif //MCMC_H
