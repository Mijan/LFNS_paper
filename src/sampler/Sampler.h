//
// Created by jan on 09/10/18.
//

#ifndef LFNS_SAMPLER_H
#define LFNS_SAMPLER_H


#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/complex.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>

#include "../base/EigenSerialization.h"


#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <float.h>
#include <iostream>
#include "../base/EigenMatrices.h"
#include "../base/RandomDistributions.h"


namespace sampler {
    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;


    class SamplerData {
    public:
        SamplerData(int n) : bounds(n, std::pair<double, double>(-DBL_MAX, DBL_MAX)) {}

        SamplerData(const SamplerData &rhs) : bounds(rhs.bounds) {}

        virtual ~SamplerData() {};

        std::size_t size() const { return bounds.size(); }

        std::vector<std::pair<double, double> > bounds;

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) { ar & bounds; }
    };

    typedef std::shared_ptr<SamplerData> SamplerData_ptr;

    class Sampler {
    public:
        Sampler(base::RngPtr rng, SamplerData data) : _rng(rng), _sample(data.size()), _bounds(data.bounds) {}

        virtual ~Sampler() {}

        virtual std::vector<double> &sample() = 0;

        virtual double getLogLikelihood(const std::vector<double> &sample) = 0;

        virtual void setBounds(std::vector<std::pair<double, double> > bounds) { _bounds = bounds; }

        virtual std::size_t getSamplerDimension() const { return _bounds.size(); }

        virtual void updateSeed(int seed) { _rng->seed(seed); }

        virtual void updateLogLikelihoodFct(LogLikelihodEvalFct_ptr fct_ptr) {}

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

        virtual void setRng(base::RngPtr rng) { _rng = rng; }

        virtual void setLogScale(int param_index) {
            if (_bounds[param_index].first <= 0 || _bounds[param_index].second <= 0) {
                std::stringstream ss;
                ss << "Tried to set scale for parameter number " << param_index
                   << " to log, but original bounds is already " << _bounds[param_index].first << ", "
                   << _bounds[param_index].second << std::endl;
                throw std::runtime_error(ss.str());
            }
            _bounds[param_index].first = std::log10(_bounds[param_index].first);
            _bounds[param_index].second = std::log10(_bounds[param_index].second);
        }

    protected:

        base::RngPtr _rng;
        std::vector<double> _sample;
        std::vector<std::pair<double, double> > _bounds;

        friend class ::boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & _sample;
            ar & _bounds;
        }
    };

    typedef std::shared_ptr<Sampler> Sampler_ptr;

    class KernelSampler {
    public:

        virtual ~KernelSampler() {}

        virtual std::vector<double> &sample(const std::vector<double> &kernel_center) = 0;

        virtual void sample(base::EiVector &sample, const std::vector<double> &kernel_center) = 0;

        virtual double
        getLogLikelihood(const std::vector<double> &sample, const std::vector<double> &kernel_center) = 0;

        virtual double getLogLikelihood(const base::EiVector &sample, const std::vector<double> &kernel_center) = 0;

        virtual void updateKernel(const base::EiMatrix &transformed_samples) = 0;

        virtual void writeToStream(std::ostream &stream) = 0;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {}
    };

    typedef std::shared_ptr<KernelSampler> KernelSampler_ptr;
}

#endif //LFNS_SAMPLER_H
