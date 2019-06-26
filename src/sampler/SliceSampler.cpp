//
// Created by jan on 28/02/19.
//

#include "SliceSampler.h"
#include "../base/MathUtils.h"
#include "../base/MultivariateNormal.h"


namespace sampler {
    SliceSampler::SliceSampler(base::RngPtr rng, SamplerData data, LogLikelihodEvalFct_ptr log_like_fun,
                               double *epsilon, int num_steps)
            : DensityEstimation(rng, data), _log_like(log_like_fun), _transformed_samples(1, data.size()),
              _num_steps(num_steps), _sample_dist(0, 1), _uniform_dist(0, 1), _decomposed_cov(data.size(), data.size()),
              _lower_bound(data.size()), _upper_bound(data.size()), _epsilon(epsilon) {}

    SliceSampler::~SliceSampler() {}

    void SliceSampler::updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) {
        _transformed_samples = transformed_samples;
        _sample_dist = base::UniformIntDistribution(0, _transformed_samples.rows() - 1);


        base::EiMatrix centered = transformed_samples.rowwise() - transformed_samples.colwise().mean();
        base::EiMatrix trans_cov = (centered.adjoint() * centered) / double(transformed_samples.rows() - 1);
        base::MultivariateNormal::decomposeVar(trans_cov, &_decomposed_cov);
    }

    void SliceSampler::sampleTransformed(base::EiVector &trans_sample) {
        int first_sample_index = _sample_dist(*_rng);
        base::EiVector original_sample = _transformed_samples.row(first_sample_index);
        base::EiVector sample_direction(original_sample.size());
        double test_likelihood = _log_like_eigen(original_sample);

        for (int i = 0; i < _num_steps; i++) {
            base::MultivariateNormal::mvnormRndWithDecomposedVar(_rng, _transformed_samples.cols(), _mean,
                                                                 _decomposed_cov, &sample_direction);
            _lower_bound = original_sample - sample_direction;
            _upper_bound = original_sample + sample_direction;

            double log_likelihood = _log_like_eigen(_lower_bound);
            int scale = 1;
            while (log_likelihood > *_epsilon) {
                scale *= 2;
                _lower_bound = original_sample - (scale * sample_direction);
                log_likelihood = _log_like_eigen(_lower_bound);
            }

            log_likelihood = _log_like_eigen(_upper_bound);
            scale = 1;
            while (log_likelihood > *_epsilon) {
                scale *= 2;
                _upper_bound = original_sample + (scale * sample_direction);
                log_likelihood = _log_like_eigen(_upper_bound);
            }


            do {
                double u = _uniform_dist(*_rng);
                double length = (_upper_bound - _lower_bound).norm();
                double middle_length = (original_sample - _lower_bound).norm() / length;

                trans_sample = _lower_bound + u * (length / sample_direction.norm()) * sample_direction;
                log_likelihood = _log_like_eigen(trans_sample);

                if (u < middle_length) {
                    _lower_bound = trans_sample;
                } else {
                    _upper_bound = trans_sample;
                }
            } while (log_likelihood < *_epsilon);

        }
    }

    double SliceSampler::getTransformedLogLikelihood(const base::EiVector &trans_sample) { return -DBL_MAX; }


    void SliceSampler::updateLogLikelihoodFct(LogLikelihodEvalFct_ptr fct_ptr) {
        _log_like = fct_ptr;
    }



    void SliceSampler::setLogScaleIndices(std::vector<int> log_scale_indices){
        _log_scale_indices = log_scale_indices;
    }

    double SliceSampler::_log_like_eigen(base::EiVectorRef sample) {
        base::EiVector ei_vector = sample;
        retransformSample(ei_vector);
        for(int log_index: _log_scale_indices){
            ei_vector(log_index) = std::pow(10, ei_vector(log_index));
        }
        std::vector<double> vec(_transformed_samples.cols());
        Eigen::VectorXd::Map(&vec[0], ei_vector.size()) = ei_vector;
        return (*_log_like)(vec);
    }

}