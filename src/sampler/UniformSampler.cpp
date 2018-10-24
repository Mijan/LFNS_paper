//
// Created by jan on 09/10/18.
//

#include <cfloat>
#include <fstream>
#include "UniformSampler.h"

namespace sampler {

    UniformSampler::UniformSampler(base::RngPtr rng, UniformSamplerData data)
            : Sampler(rng, data), _widths(data.widths), _uniform_dist(0, 1) {}

    UniformSampler::~UniformSampler() {}

    std::vector<double> &UniformSampler::sample() {
        for (std::size_t i = 0; i < _sample.size(); i++) {
            _sample[i] = _getRndInBounds(_bounds[i].first, _bounds[i].second);
        }

        return _sample;
    }


    void UniformSampler::sample(base::EiVector &sample, const std::vector<double> &kernel_center) {
        for (std::size_t i = 0; i < _sample.size(); i++) {
            sample(i) = _getRndInBounds(_bounds[i].first, _bounds[i].second);
        }
    }


    double UniformSampler::getLogLikelihood(const std::vector<double> &sample) {
        double log_likelihood = 0.0;
        for (std::size_t i = 0; i < sample.size(); i++) {
            double entry = sample[i];
            if (entry <= _bounds[i].second && entry >= _bounds[i].first) {
                log_likelihood += _getLogLikelihood(_bounds[i].first, _bounds[i].second);
            } else {
                return -DBL_MAX;
            }
        }
        return log_likelihood;
    }

    std::vector<double> &UniformSampler::sample(const std::vector<double> &kernel_center) {
        do {
            for (std::size_t i = 0; i < _sample.size(); i++) {
                _sample[i] = _getRndInBounds(kernel_center[i] - (_widths[i] / 2.0),
                                             kernel_center[i] + (_widths[i] / 2.0));
            }
        } while (!isSampleFeasible(_sample));
        return _sample;
    }

    double UniformSampler::getLogLikelihood(const std::vector<double> &sample,
                                            const std::vector<double> &kernel_center) {

        double log_likelihood = 0.0;
        for (std::size_t i = 0; i < _sample.size(); i++) {
            double entry = sample[i];
            double prev_entry = kernel_center[i];
            if (entry <= prev_entry + (_widths[i] / 2.0) && entry >= prev_entry - (_widths[i] / 2.0)) {
                log_likelihood += _getLogLikelihood(prev_entry - (_widths[i] / 2.0),
                                                    prev_entry + (_widths[i] / 2.0));
            } else {
                return -DBL_MAX;
            }

        }
        return log_likelihood;
    }


    double UniformSampler::getLogLikelihood(const base::EiVector &sample, const std::vector<double> &kernel_center){

        double log_likelihood = 0.0;
        for (std::size_t i = 0; i < sample.size(); i++) {
            double entry = sample(i);
            double prev_entry = kernel_center[i];
            if (entry <= prev_entry + (_widths[i] / 2.0) && entry >= prev_entry - (_widths[i] / 2.0)) {
                log_likelihood += _getLogLikelihood(prev_entry - (_widths[i] / 2.0),
                                                    prev_entry + (_widths[i] / 2.0));
            } else {
                return -DBL_MAX;
            }

        }
        return log_likelihood;
    }


    void UniformSampler::setWidths(std::vector<double> widths) {
        _widths = widths;
        _sample.resize(widths.size());
    }


    void UniformSampler::writeToStream(std::ostream &stream) {
        stream << "Uniform Sampler" << std::endl;
        Sampler::writeToStream(stream);
    }

    double UniformSampler::_getRndInBounds(double l_bound, double r_bound) {
        return l_bound + _uniform_dist(*_rng) * (r_bound - l_bound);
    }

    double UniformSampler::_getLogLikelihood(double l_bound, double r_bound) {
        return -log(r_bound - l_bound);
    }


}