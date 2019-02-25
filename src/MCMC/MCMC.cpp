//
// Created by jan on 25/02/19.
//

#include <cmath>
#include "MCMC.h"
#include "../base/RandomDistributions.h"
#include "../base/IoUtils.h"

namespace mcmc {
    MCMC::MCMC(MCMCSettings &mcmc_settings, sampler::SamplerSettings &sampler_settings, base::RngPtr rng,
               LogLikelihodEvalFct_ptr log_likelihood_evaluation) : _settings(mcmc_settings), _rng(rng),
                                                                    _log_likelihood_evaluation(
                                                                            log_likelihood_evaluation),
                                                                    _sampler(mcmc_settings, sampler_settings, rng),
                                                                    _particles(), _logger(mcmc_settings),
                                                                    _resume_run(false) {}

    void MCMC::runMCMC() {

        bool mcmc_terminate = false;

        base::UniformRealDistribution _dist_uniform;
        int m = 0;
        std::vector<double> curr_theta;

        double curr_log_like = -DBL_MAX;
        double new_log_like = -DBL_MAX;

        if (!_resume_run) {
            _logger.mcmcStarted();
            curr_theta = _sampler.samplePrior();
            _logger.particleSampled(curr_theta);

            curr_log_like = (*_log_likelihood_evaluation)(curr_theta);
            _logger.particleAccepted(curr_theta, curr_log_like);
        } else {

            curr_theta = _particles.back().particle;
            curr_log_like = _particles.back().log_likelihood;

            _logger.mcmcResume(_particles.size(), curr_theta, curr_log_like);
        }
        while (!mcmc_terminate) {

            const std::vector<double> &new_theta = _sampler.sampleKernel(curr_theta);
            _logger.particleSampled(curr_theta);

            new_log_like = (*_log_likelihood_evaluation)(new_theta);

            double A = (new_log_like) - (curr_log_like);
            A = std::exp(A);

            double u = _dist_uniform(*_rng);
            if (u <= A) {
                curr_theta = new_theta;
                curr_log_like = new_log_like;
                _logger.particleAccepted(curr_theta, curr_log_like);
            } else {
                _logger.particleRejected(new_theta, new_log_like);
            }
            _particles.push_back(MCMCParticle(curr_theta, curr_log_like));


            if ((int) _particles.size() % (int) 100 == (int) 0) { _particles.writeToFile(_settings.output_file); }
            mcmc_terminate = _particles.size() >= _settings.N;

        }

        _particles.writeToFile(_settings.output_file);
    }

    void MCMC::resumeRum(std::string previous_particle_file) {
        try {
            _particles.readFromFile(previous_particle_file);
            _resume_run = true;
        } catch (const std::runtime_error &e) {
            std::stringstream ss;
            ss << "Failed to read previous population from file " << previous_particle_file << ":\n\t";
            ss << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
}
