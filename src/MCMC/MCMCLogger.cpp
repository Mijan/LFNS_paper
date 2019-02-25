//
// Created by jan on 25/02/19.
//

#include <iostream>
#include "MCMCLogger.h"

namespace mcmc {
    MCMCLogger::MCMCLogger(MCMCSettings settings) : _num_accepted_particles(0), _num_rejections(0) {}

    MCMCLogger::~MCMCLogger() {}

    void MCMCLogger::mcmcStarted() {
        std::cout << "\n\nStarting MCMC algorithm" << std::endl << std::endl;
    }

    void MCMCLogger::mcmcResume(int num_particles, const std::vector<double> &theta, double log_like) {
        std::cout << "\nStarting MCMC after " << num_particles << " samples and initial log-likelihood " << log_like
                  << std::endl;
    }

    void MCMCLogger::particleSampled(const std::vector<double> &curr_theta) {
        _particle_tic = clock();
    }

    void MCMCLogger::particleAccepted(const std::vector<double> &theta, double log_likelihood) {
        particleAccepted(theta, log_likelihood, clock() - _particle_tic);
    }


    void MCMCLogger::particleAccepted(const std::vector<double> &theta, double log_likelihood, time_t clocks_particle) {

        _num_accepted_particles++;
        std::cout << "i = " << _num_accepted_particles << ", log likelihood = " << log_likelihood
                  << ", acceptance probability = " << 1.0 / (double) (_num_rejections + 1.0) << ", time for particle = "
                  << clocks_particle / (double) CLOCKS_PER_SEC;
        std::cout << std::endl;
        _num_rejections = 0;
    }

    void MCMCLogger::particleRejected(const std::vector<double> &theta, double log_likelihood) {
        std::cout << "\t" << "rejected log likelihood = " << log_likelihood << std::endl;
        _num_rejections++;
    }

    void MCMCLogger::mcmcTerminated() {
        std::cout << "\n\nMCMC algorithm successfully terminated!" << std::endl << std::endl;
    }
}
