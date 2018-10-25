//
// Created by jan on 09/10/18.
//

#include "EllipsoidSampler.h"
#include <iostream>
#include <math.h>

namespace sampler {
    EllipsoidSampler::EllipsoidSampler(base::RngPtr rng, SamplerData data) : DensityEstimation(rng, data),
                                                                             _mean(data.size()),
                                                                             _evs(data.size(),
                                                                                  data.size()),
                                                                             _evals(data.size()),
                                                                             _scale(0), _uniform_dist(0, 1) {}

    EllipsoidSampler::~EllipsoidSampler() {}

    void EllipsoidSampler::setScale(double scale_var) { _scale = scale_var; }

    void EllipsoidSampler::updateTransformedDensitySamples(base::EiMatrix transformed_samples) {

        _scale = -1;
        for (std::size_t row = 0; row < transformed_samples.rows(); row++) {
            double max_coeff = transformed_samples.row(row).real().norm();
            _scale = _scale < max_coeff ? max_coeff : _scale;
        }
    }

    void EllipsoidSampler::sampleTransformed(base::EiVector &trans_sample) {
        double angle;
        double r = _uniform_dist(*_rng) * _scale;
        double sin_prod = 1;
        for (std::size_t i = 0; i < _sample.size() - 1; i++) {
            angle = _uniform_dist(*_rng) * 2 * M_PI;
            trans_sample(i) = r * sin_prod * std::cos(angle);
            sin_prod = sin_prod * std::sin(angle);
        }
        trans_sample(_sample.size() - 1) = r * sin_prod;
    }

    double EllipsoidSampler::getTransformedLogLikelihood(const base::EiVector &trans_sample) {
        if (trans_sample.norm() <= _scale) { return 1.0; } else { return 0.0; }
    }

}