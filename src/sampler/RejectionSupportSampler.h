//
// Created by jan on 09/10/18.
//

#ifndef LFNS_REJECTIONSAMPLER_H
#define LFNS_REJECTIONSAMPLER_H


#include <boost/serialization/base_object.hpp>
#include <iostream>
#include "Sampler.h"
#include "DensityEstimation.h"

namespace sampler {
    class RejectionSamplerData : public SamplerData {
    public:
        explicit RejectionSamplerData(int n) : SamplerData(n), rejection_quantile(-1),
                                               rejection_quantile_low_accept(-1), log_rejection_const(-1),
                                               thresh_accept_rate(-1), max_rejection_mag(-1) {}

        virtual ~RejectionSamplerData() = default;

        RejectionSamplerData(const RejectionSamplerData &rhs) : SamplerData(rhs),
                                                                rejection_quantile(rhs.rejection_quantile),
                                                                rejection_quantile_low_accept(
                                                                        rhs.rejection_quantile_low_accept),
                                                                log_rejection_const(rhs.log_rejection_const),
                                                                thresh_accept_rate(rhs.thresh_accept_rate),
                                                                max_rejection_mag(rhs.max_rejection_mag) {}

        RejectionSamplerData(const SamplerData &rhs) : SamplerData(rhs), rejection_quantile(-1),
                                                       rejection_quantile_low_accept(-1), log_rejection_const(-1),
                                                       thresh_accept_rate(-1), max_rejection_mag(-1) {}

        RejectionSamplerData &operator=(const SamplerData &rhs) {
            if (this == &rhs) { return *this; }
            SamplerData::operator=(rhs);
            rejection_quantile = -1;
            rejection_quantile_low_accept = -1;
            log_rejection_const = -1;
            thresh_accept_rate = -1;
            max_rejection_mag = -1;
            return *this;
        }

        double rejection_quantile;
        double rejection_quantile_low_accept;
        double log_rejection_const;
        double thresh_accept_rate;
        double max_rejection_mag;
    };

    class RejectionSupportSampler : public DensityEstimation {
    public:
        RejectionSupportSampler(base::RngPtr rng, DensityEstimation_ptr sampler, RejectionSamplerData data);

        ~RejectionSupportSampler() override;

        void writeToStream(std::ostream &stream) override;

        void computeRejectionConst(const base::EiMatrix &transformed_samples);

        void updateSeed(int seed) override;

        void updateAcceptanceRate(double acceptance_rate);

        virtual void updateTransformedDensitySamples(base::EiMatrix transformed_samples);

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

    private:
        base::UniformRealDistribution _dist;
        DensityEstimation_ptr _current_sampler;
        RejectionSamplerData _rejection_data;

        double _log_rejection_const;
        double _rejection_quantile;

    };

    typedef std::shared_ptr<sampler::RejectionSupportSampler> RejectionSupportSampler_ptr;
} /* namespace sampler */

#endif //LFNS_REJECTIONSAMPLER_H
