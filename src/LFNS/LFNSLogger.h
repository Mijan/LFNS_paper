//
// Created by jan on 05/10/18.
//

#ifndef LFNS_LFNSLOGGER_H
#define LFNS_LFNSLOGGER_H

#include <vector>
#include "LFNSParticle.h"
#include "LiveParticleSet.h"
#include "DeadParticleSet.h"
#include "PosteriorEstimator.h"
#include "../sampler/DensityEstimation.h"
#include "LFNSSampler.h"

namespace lfns {
    class LFNSLogger {

    public:
        LFNSLogger(LFNSSettings settings);

        virtual ~LFNSLogger();

        void lfnsStarted(int i, double d);

        void lfnsResume(int m, double epsilon);

        void iterationStarted(int i);

        void deadPointAdded(const LFNSParticle &particle);

        void epsilonUpdated(double d);

        void samplerUpdated(LFNSSampler &sampler, time_t clocks_for_sampler_update);

        void thetaSampled(const std::vector<double> &vector, time_t clocks_sampling);

        void likelihoodComputed(double likelihood);

        void particleAccepted(const std::vector<double> &theta, double l);

        void
        particleAccepted(const std::vector<double> &theta, double l, time_t clocks_particle, int origin_process = 0);

        void logIterationResults(PosteriorQuantitites &post_quant);

        void logIterationStats();

        void lfnsTerminated();

        void writeToFile();

        void readFromFile(std::string previous_log_file_name);

        int iterationNumber();

        double lastEpsilon();

        double lastAcceptanceRate();

    private:
        int _remaining_required_particles_iteration;

        int _num_samples_iteration;
        int _num_samples_particle;
        int _num_accepted_iteration;

        int _print_interval;
        int _acceptance_info_print_interval;

        time_t _particle_tic;
        time_t _algorithm_tic;
        time_t _iteration_tic;
        double _sampling_seconds;

        std::string _output_file_name;
        std::vector<double> _acceptance_rates;
        std::vector<double> _epsilons;
        std::vector<int> _iteration_nbrs;
        std::vector<double> _seconds_for_iteration;
        std::vector<double> _sampling_seconds_for_iteration;

        std::vector<double> _log_zd;
        std::vector<double> _log_var_zd;
        std::vector<double> _log_zl;
        std::vector<double> _log_var_zl;
        std::vector<double> _log_ztot;
        std::vector<double> _log_var_ztot;
        std::vector<double> _log_v_min;

        std::vector<double> _var_L_contribution;
        std::vector<double> _max_L_contribution;

        void _printAcceptanceInfo();
    };
}


#endif //LFNS_LFNSLOGGER_H
