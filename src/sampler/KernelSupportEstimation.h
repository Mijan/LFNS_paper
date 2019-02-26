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
    };
}


#endif //LFNS_KERNELSUPPORTESTIMATION_H
