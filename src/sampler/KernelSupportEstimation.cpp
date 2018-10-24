//
// Created by jan on 09/10/18.
//

#include "KernelSupportEstimation.h"

namespace sampler {

    KernelSupportEstimation::KernelSupportEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data)
            : KernelDensityEstimation(rng, kernel, data) {}

    void KernelSupportEstimation::updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) {
        KernelDensityEstimation::updateTransformedDensitySamples(transformed_samples);

        base::EiVector weights(_means.size());
        for (int i = 0; i < _means.size(); i++) {
            weights(i) = -getTransformedLogLikelihood(transformed_samples.row(i));
        }
        double max = weights.maxCoeff();
        weights = weights - max * base::EiVector::Ones(_means.size());
        weights = weights.array().exp();
        _weights = weights / weights.sum();
    }
}
