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
    using namespace std::placeholders;
    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class MultLikelihoodEval {
    public:
        MultLikelihoodEval() : _log_like_fun() {}

        ~MultLikelihoodEval() {}

        double compute_log_like(const std::vector<double> &theta) {
            double log_like = 0;
            for (LogLikelihodEvalFct_ptr fun : _log_like_fun) {
                log_like += (*fun)(theta);
            }
            return log_like;
        }

        void addLogLikeFun(LogLikelihodEvalFct_ptr log_like_fun) { _log_like_fun.push_back(log_like_fun); }

        LogLikelihodEvalFct_ptr getLogLikeFun() {
            return std::make_shared<LogLikelihodEvalFct>(std::bind(&MultLikelihoodEval::compute_log_like, this, _1));
        }

    private:
        std::vector<LogLikelihodEvalFct_ptr> _log_like_fun;
    };

    class LFNS {

    public:
        explicit LFNS(LFNSSettings &settings, base::RngPtr rng);

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

        bool _testTermination(PosteriorQuantitites &post_quant);

        bool _postIteration();
    };
};


#endif //LFNS_LFNS_H
