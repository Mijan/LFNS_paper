//
// Created by jan on 28/02/19.
//

#ifndef LFNS_SLICESAMPLER_H
#define LFNS_SLICESAMPLER_H


#include "DensityEstimation.h"

namespace sampler {
    typedef std::function<double(const std::vector<double>&)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class SliceSampler : DensityEstimation {
    public:
        SliceSampler(base::RngPtr rng, SamplerData data, LogLikelihodEvalFct_ptr log_like_fun, int num_steps = 10);

        ~SliceSampler() override;

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        virtual void sampleTransformed(base::EiVector &trans_sample) override;

        virtual double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

    private:
        double _log_like_eigen(base::EiVectorRef sample);

        LogLikelihodEvalFct_ptr _log_like;
        base::EiMatrix _transformed_samples;
        base::EiMatrix _decomposed_cov;
        int _num_steps;
        base::UniformIntDistribution _sample_dist;
        base::UniformRealDistribution _uniform_dist;

        base::EiVector _lower_bound;
        base::EiVector _upper_bound;

        double *_epsilon;


    };
}


#endif //LFNS_SLICESAMPLER_H
