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
    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;
    class LFNS {

    public:
        explicit LFNS(LFNSSettings &lfns_settings, sampler::SamplerSettings &sampler_settings, base::RngPtr rng);

        virtual ~LFNS();

        virtual void runLFNS() = 0;

        void resumeRum(std::string previous_log_file);

        double* getPointerToThreshold();

    protected:
        LFNSSettings _settings;

        LiveParticleSet _live_points;
        DeadParticleSet _dead_points;

        PosteriorEstimator _post_estimator;
        LFNSSampler _sampler;

        LFNSLogger _logger;
        bool _resume_run;
        double _epsilon;

        int _num_parameters;

        bool _testTermination(PosteriorQuantitites &post_quant);

        bool _postIteration();
    };
};


#endif //LFNS_LFNS_H
