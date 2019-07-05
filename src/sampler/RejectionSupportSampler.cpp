//
// Created by jan on 09/10/18.
//

#include <algorithm>
#include "RejectionSupportSampler.h"
#include "../base/IoUtils.h"
#include "../base/MathUtils.h"

namespace sampler {

    RejectionSupportSampler::RejectionSupportSampler(base::RngPtr rng, DensityEstimation_ptr sampler,
                                                     RejectionSamplerData data) : DensityEstimation(rng, data),
                                                                                  _dist(0, 1),
                                                                                  _current_sampler(sampler),
                                                                                  _rejection_data(data),
                                                                                  _log_rejection_const(
                                                                                          data.log_rejection_const),
                                                                                  _rejection_quantile(
                                                                                          data.rejection_quantile) {
        if (data.max_rejection_mag > 0 && _rejection_quantile > 0) {
            std::stringstream os;
            os << "Either rejection quantile or the maximal rejection magnitude can be provided, but not both! "
               << std::endl;
            throw std::runtime_error(os.str());
        }
        if (data.max_rejection_mag < 0 && _rejection_quantile < 0) {
            std::stringstream os;
            os << "Either rejection quantile or the maximal rejection magnitude must be provided!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

    }

    RejectionSupportSampler::~RejectionSupportSampler() = default;

    void RejectionSupportSampler::writeToStream(std::ostream &stream) {
        stream << "Rejection support sampler" << std::endl;
        stream << "Basic sampler:" << std::endl;
        _current_sampler->writeToStream(stream);
    }


    void RejectionSupportSampler::updateSeed(int seed) {
        Sampler::updateSeed(seed);
        _current_sampler->updateSeed(2 * seed);
    }

    void RejectionSupportSampler::updateAcceptanceRate(double acceptance_rate) {
        if (acceptance_rate < _rejection_data.thresh_accept_rate) {
            _rejection_quantile = _rejection_data.rejection_quantile_low_accept;
        } else {
            _rejection_quantile = _rejection_data.rejection_quantile;
        }
    }

    void RejectionSupportSampler::computeRejectionConst(const base::EiMatrix &transformed_samples) {

        int num_runs = 1000;
        base::EiVector sample(transformed_samples.cols());
        std::vector<double> likes;
        likes.reserve(num_runs);
        double min = DBL_MAX;
        double max = -DBL_MAX;
        for (int i = 0; i < num_runs; i++) {
            double like = 0;
            do {
                _current_sampler->sampleTransformed(sample);
                like = std::exp(_current_sampler->getTransformedLogLikelihood(sample));
                sample = (_evs * sample.cast<std::complex<double> >()).real();
                sample += _mean;
            } while (!isSampleFeasible(sample));
            likes.push_back(like);
            max = max > like ? max : like;
            min = min < like ? min : like;
        }
        std::sort(likes.begin(), likes.end());
        double rejection_const = likes[floor(_rejection_quantile * num_runs)];

        std::cout << "rejection quantile " << _rejection_quantile << std::endl;
        std::cout << "rejection const " << rejection_const << std::endl;
        std::cout << "max log like " << max << std::endl;
        std::cout << "min log like " << min << std::endl;
        _log_rejection_const = std::log(rejection_const);
    }


    void RejectionSupportSampler::updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) {
        _current_sampler->updateTransformedDensitySamples(transformed_samples);
        computeRejectionConst(transformed_samples);
    }

    void RejectionSupportSampler::sampleTransformed(base::EiVector &trans_sample) {
        if (!_current_sampler) {
            throw std::runtime_error(
                    "Tried to sample from rejection sampler without having set the current sampler!");
        }
        _current_sampler->sampleTransformed(trans_sample);
        double log_like = _current_sampler->getTransformedLogLikelihood(trans_sample);

        if (_log_rejection_const > log_like) {
            sampleTransformed(trans_sample);
        } else {
            double u;
            u = std::exp(_log_rejection_const - log_like);
            double r = _dist(*_rng);
            if (u < r) {
                sampleTransformed(trans_sample);
            }
        }
    }

    double RejectionSupportSampler::getTransformedLogLikelihood(const base::EiVector &trans_sample) {
        if (!_current_sampler) {
            throw std::runtime_error(
                    "Tried to compute the log-likelihood from rejection sampler without having set the current sampler!");
        }
        double log_likelihood;
        double log_like = _current_sampler->getTransformedLogLikelihood(trans_sample);
        if (log_like < _log_rejection_const) { log_likelihood = -DBL_MAX; }
        else { log_likelihood = _log_rejection_const; }
        return log_likelihood;
    }


    void RejectionSupportSampler::setRng(base::RngPtr rng) {
        DensityEstimation::setRng(rng);
        _current_sampler->setRng(rng);
    }


    void RejectionSupportSampler::setLogScale(int param_index) {
        Sampler::setLogScale(param_index);
        _current_sampler->setLogScale(param_index);
    }

} /* namespace sampler */