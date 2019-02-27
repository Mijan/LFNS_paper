//
// Created by jan on 09/10/18.
//

#ifndef LFNS_KERNELSUPPORTESTIMATION_H
#define LFNS_KERNELSUPPORTESTIMATION_H

#include "KernelDensityEstimation.h"

namespace sampler {
    class KernelSupportEstimation : public KernelDensityEstimation {
    public:
        KernelSupportEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data);

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        friend class boost::serialization::access;


        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & boost::serialization::base_object<sampler::KernelDensityEstimation>(*this);
        }

    };
}


#endif //LFNS_KERNELSUPPORTESTIMATION_H
