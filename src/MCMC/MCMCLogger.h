//
// Created by jan on 25/02/19.
//

#ifndef MCMCLOGGER_H
#define MCMCLOGGER_H

#include "MCMCSettings.h"
#include "MCMCParticles.h"

namespace mcmc {
    class MCMCLogger {
    public:
        MCMCLogger(MCMCSettings settings);

        virtual ~MCMCLogger();

        void mcmcStarted();

        void mcmcResume(int num_particles, const std::vector<double> &theta, double log_like);

        void particleSampled(const std::vector<double> &curr_theta);

        void particleAccepted(const std::vector<double> &theta, double log_l);

        void particleAccepted(const std::vector<double> &theta, double log_l, time_t clocks_particle);

        void particleRejected(const std::vector<double> &theta, double log_likelihood);

        void mcmcTerminated();

    private:
        void _printAcceptanceInfo();

        time_t _particle_tic;

        int _num_accepted_particles;
        int _num_rejections;
    };
}


#endif //MCMCLOGGER_H
