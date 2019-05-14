//
// Created by jan on 09/10/18.
//

#include "KernelDensityEstimation.h"

namespace sampler {
    KernelDensityEstimation::KernelDensityEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data)
            : DensityEstimation(rng, data), _means(), _weights(), _kernel(kernel), _log_likes_tmp(),
              _uniform_dist(0, 1) {}

    KernelDensityEstimation::~KernelDensityEstimation() {}


    void KernelDensityEstimation::updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) {

        _means.resize(transformed_samples.rows());
        _log_likes_tmp = base::EiVector(transformed_samples.rows());
        _weights = base::EiVector::Ones(transformed_samples.rows()) * (1.0 / (double) transformed_samples.rows());
        for (int i = 0; i < transformed_samples.rows(); i++) {

            base::EiConstSubVectorRow row = transformed_samples.row(i);

            // TODO find better solution so far each row is copied once into the Eigen vector and then into the std vector
            base::EiVector r = row;
            std::vector<double> sample(r.size());
            base::EiVector::Map(&sample[0], r.size()) = row;

            _means[i] = sample;
        }
        _kernel->updateKernel(transformed_samples);
    }

    void KernelDensityEstimation::sampleTransformed(base::EiVector &trans_sample) {
        double u = _uniform_dist(*_rng);
        int index = 0;
        double cum_weight = _weights[0];
        while (cum_weight < u) {
            cum_weight += _weights[++index];
        }
        _kernel->sample(trans_sample, _means[index]);
    }

    double KernelDensityEstimation::getTransformedLogLikelihood(const base::EiVector &trans_sample) {
        double log_likelihood = 0;
        double max_log_like = -DBL_MAX;
        for (std::size_t i = 0; i < _means.size(); i++) {
            std::vector<double> sample(trans_sample.size());
            _log_likes_tmp(i) = _kernel->getLogLikelihood(trans_sample, _means[i]);
            max_log_like = max_log_like > _log_likes_tmp(i) ? max_log_like : _log_likes_tmp(i);
        }

        _log_likes_tmp = _log_likes_tmp - max_log_like * base::EiVector::Ones(_means.size());
        _log_likes_tmp = _log_likes_tmp.array().exp();
        log_likelihood = std::log(_weights.dot(_log_likes_tmp)) + max_log_like;

        return log_likelihood;
    }


    void KernelDensityEstimation::writeToStream(std::ostream &stream) {
        stream << "KernelDensityEstimation with kernel:" << std::endl;
        _kernel->writeToStream(stream);
    }

    void KernelDensityEstimation::setRng(base::RngPtr rng){
        DensityEstimation::setRng(rng);
    }
}