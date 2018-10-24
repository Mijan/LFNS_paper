//
// Created by jan on 09/10/18.
//

#ifndef LFNS_SAMPLER_H
#define LFNS_SAMPLER_H

#include <boost/serialization/access.hpp>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <float.h>
#include <boost/serialization/assume_abstract.hpp>
#include "../base/EigenMatrices.h"
#include "../base/RandomDistributions.h"

namespace sampler {
    class SamplerData {
    public:
        SamplerData(int n) : bounds(n, std::pair<double, double>(-DBL_MAX, DBL_MAX)) {}

        virtual ~SamplerData() {};

        std::size_t size() const { return bounds.size(); }

        std::vector<std::pair<double, double> > bounds;
    };

    typedef std::shared_ptr<SamplerData> SamplerData_ptr;

    class Sampler {
    public:
        Sampler(base::RngPtr rng, SamplerData data) : _rng(rng), _sample(data.size()),
                                                      _bounds(data.bounds) {}

        virtual ~Sampler() {}

        virtual std::vector<double> &sample() = 0;

        virtual double getLogLikelihood(const std::vector<double> &sample) = 0;

        virtual void setBounds(std::vector<std::pair<double, double> > bounds) { _bounds = bounds; }

        virtual std::size_t getSamplerDimension() const { return _bounds.size(); }

        virtual void updateSeed(int seed) { _rng->seed(seed); }

        friend class ::boost::serialization::access;

        bool isSampleFeasible(const std::vector<double> &sample) {
            for (std::size_t i = 0; i < sample.size(); i++) {
                if (sample[i] < _bounds[i].first || sample[i] > _bounds[i].second) { return false; }
            }
            return true;
        }

        bool isSampleFeasible(base::EiVector &sample) {
            for (std::size_t i = 0; i < sample.size(); i++) {
                if (sample(i) < _bounds[i].first || sample(i) > _bounds[i].second) { return false; }
            }
            return true;
        }

        virtual void writeToStream(std::ostream &stream) {
            stream << "lower bounds: " << _bounds[0].first;
            for (std::size_t i = 1; i < _bounds.size(); i++) { stream << " " << _bounds[i].first; }
            stream << std::endl;
            stream << "upper bounds: " << _bounds[0].second;
            for (std::size_t i = 1; i < _bounds.size(); i++) { stream << " " << _bounds[i].second; }
            stream << std::endl;
        }

    protected:

        base::RngPtr _rng;
        std::vector<double> _sample;
        std::vector<std::pair<double, double> > _bounds;


        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {}
    };

    typedef std::shared_ptr<Sampler> Sampler_ptr;

    class KernelSampler {
    public:
        virtual const std::vector<double> &sample(const std::vector<double> &kernel_center) = 0;

        virtual void sample(base::EiVector &sample, const std::vector<double> &kernel_center) = 0;

        virtual double
        getLogLikelihood(const std::vector<double> &sample, const std::vector<double> &kernel_center) = 0;

        virtual double getLogLikelihood(const base::EiVector &sample, const std::vector<double> &kernel_center) = 0;

        friend class ::boost::serialization::access;

    protected:

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {}
    };

    typedef std::shared_ptr<KernelSampler> KernelSampler_ptr;
}

#endif //LFNS_SAMPLER_H
