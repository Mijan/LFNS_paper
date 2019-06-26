//
// Created by jan on 05/10/18.
//

#ifndef LFNS_LFNS_H
#define LFNS_LFNS_H


#include <functional>
#include "LFNSSettings.h"
#include "LiveParticleSet.h"
#include "DeadParticleSet.h"
#include "LFNSLogger.h"
#include "../sampler/DensityEstimation.h"
#include "LFNSSampler.h"
#include <iostream>

namespace lfns {
    // TODO find better place for typedef
    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class LFNS {

    public:
        explicit LFNS(LFNSSettings &lfns_settings);

        virtual ~LFNS();

        virtual void runLFNS() = 0;

        void resumeRum(std::string previous_log_file);

        double *getPointerToThreshold();

        bool checkIfInitialized(std::ostream &os);

        void
        setSampler(sampler::Sampler_ptr prior, sampler::DensityEstimation_ptr density_estimation, base::RngPtr rng);

        void setLogParams(std::vector<int> log_params);

        void setThresholdPointer(double * epsilon_ptr);

    protected:
        LFNSSettings _settings;

        LiveParticleSet _live_points;
        DeadParticleSet _dead_points;

        PosteriorEstimator_ptr _post_estimator;
        LFNSSampler_ptr _sampler;

        LFNSLogger _logger;
        bool _resume_run;
        double *_epsilon_ptr;
        double _epsilon;

        int _num_parameters;

        bool _testTermination(PosteriorQuantitites &post_quant);

        bool _postIteration();
    };
};


#endif //LFNS_LFNS_H
