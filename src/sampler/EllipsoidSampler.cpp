//
// Created by jan on 09/10/18.
//

#include "EllipsoidSampler.h"
#include <iostream>
#include <math.h>

namespace sampler {
    EllipsoidSampler::EllipsoidSampler(base::RngPtr rng, SamplerData data) : DensityEstimation(rng, data),
                                                                             _scale(0), _uniform_dist(0, 1) {}

    EllipsoidSampler::~EllipsoidSampler() {}

    void EllipsoidSampler::setScale(double scale_var) { _scale = scale_var; }

    void EllipsoidSampler::updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) {
        _scale = -1;
        for (std::size_t row = 0; row < transformed_samples.rows(); row++) {
            double max_coeff = transformed_samples.row(row).real().norm();
            _scale = _scale < max_coeff ? max_coeff : _scale;
        }
    }

    void EllipsoidSampler::sampleTransformed(base::EiVector &trans_sample) {
        double angle = 0.0;
        double rand_scale = std::pow(_uniform_dist(*_rng), 1.0 / getSamplerDimension());
        double r = rand_scale * _scale;
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



    void EllipsoidSampler::writeToStream(std::ostream &stream) {
        stream << "Ellipsoid Sampler" << std::endl;
        Sampler::writeToStream(stream);
    }

}