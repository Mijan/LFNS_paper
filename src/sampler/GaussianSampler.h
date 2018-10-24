//
// Created by jan on 09/10/18.
//

#ifndef LFNS_GAUSSIANSAMPLER_H
#define LFNS_GAUSSIANSAMPLER_H

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/nvp.hpp>
#include <iostream>
#include "Sampler.h"
#include "../base/EigenMatrices.h"

namespace sampler {
    class GaussianSampler;
}

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::GaussianSampler *t,
                                        const unsigned int file_version);
    } /* namespace serialization */
} /* namespace boost */

namespace sampler {

    class NormalSamplerData : public SamplerData {
    public:
        NormalSamplerData(int n) : SamplerData(n), mean(base::EiVector::Zero(n)),
                                   cov(base::EiMatrix::Identity(n, n) * 0.1) {}

        NormalSamplerData(const SamplerData &rhs) : SamplerData(rhs),
                                                    mean(base::EiVector::Zero(rhs.size())),
                                                    cov(base::EiMatrix::Identity(rhs.size(), rhs.size())) {}

        virtual ~NormalSamplerData() {};

        NormalSamplerData &operator=(const NormalSamplerData &rhs) {
            if (this == &rhs) {
                return *this;
            }
            SamplerData::operator=(rhs);
            mean = rhs.mean;
            cov = rhs.cov;
            return *this;
        }

        NormalSamplerData &operator=(const SamplerData &rhs) {
            if (this == &rhs) {
                return *this;
            }
            SamplerData::operator=(rhs);
            mean = base::EiVector::Zero(rhs.bounds.size());
            cov = base::EiMatrix::Identity(rhs.bounds.size(), rhs.bounds.size());
            return *this;
        }

        base::EiVector mean;
        base::EiMatrix cov;
    };

    typedef std::shared_ptr<NormalSamplerData> NormalSamplerData_ptr;

    class GaussianSampler : public Sampler, public KernelSampler {
    public:
        GaussianSampler(base::RngPtr rng, NormalSamplerData data);

        virtual ~GaussianSampler();


        virtual std::vector<double> &sample();

        virtual double getLogLikelihood(const std::vector<double> &sample);

        virtual std::vector<double> &sample(const std::vector<double> &kernel_center);

        virtual void sample(base::EiVector &sample, const std::vector<double> &kernel_center);

        virtual double getLogLikelihood(const std::vector<double> &sample,
                                        const std::vector<double> &kernel_center);

        virtual double getLogLikelihood(const base::EiVector &sample, const std::vector<double> &kernel_center);

        void setMean(const base::EiVectorRef &mean);

        void setVar(const base::EiMatrixRef &var);

        virtual void setBounds(std::vector<std::pair<double, double> > bounds);

        virtual void writeToStream(std::ostream &stream);

        friend class ::boost::serialization::access;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(
                Archive &ar, const ::sampler::GaussianSampler *t,
                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int file_version) {
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(sampler::Sampler);
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(sampler::KernelSampler);
        }


    private:
        base::EiVector _mean;
        base::EiMatrix _inverse_cov_matrix;
        base::EiMatrix _decomposed_var;
        base::EiMatrix _cov;
        double _det_var;

        int _n;
    };

    typedef std::shared_ptr<GaussianSampler> GaussianSampler_ptr;
}


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::GaussianSampler *t,
                                        const unsigned int file_version) {

            int num_dimensions = t->getSamplerDimension();
            ar << num_dimensions;
            for (int i = 0; i < num_dimensions; i++) {
                ar << t->_bounds[i].first;
                ar << t->_bounds[i].second;
            }

            for (int i = 0; i < num_dimensions; i++) {
                ar << t->_mean(i);
                for (int j = 0; j < num_dimensions; j++) {
                    ar << t->_cov(i, j);
                }
            }
        }

        template<class Archive>
        inline void load_construct_data(Archive &ar, sampler::GaussianSampler *t,
                                        const unsigned int file_version) {

            size_t num_dimensions = 0;
            ar >> num_dimensions;
            std::vector<std::pair<double, double> > bounds(0);
            for (size_t i = 0; i < num_dimensions; i++) {
                std::pair<double, double> bound_pair = std::make_pair(0, 0);
                ar >> bound_pair.first;
                ar >> bound_pair.second;
                bounds.push_back(bound_pair);
            }

            base::EiVector mean(num_dimensions);
            for (int j = 0; j < num_dimensions; j++) {
                ar >> mean(j);
            }

            base::EiMatrix cov(num_dimensions, num_dimensions);
            for (std::size_t i = 0; i < num_dimensions; i++) {
                for (std::size_t j = 0; j < num_dimensions; j++) {
                    double var;
                    ar >> var;
                    cov(i, j) = var;
                }
            }

            sampler::NormalSamplerData data(bounds.size());
            data.bounds = bounds;
            data.mean = mean;
            data.cov = cov;

            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>();
            // invoke inplace constructor to _initialize instance of my_class
            ::new(t) sampler::GaussianSampler(rng, data);
        }
    }
}


#endif //LFNS_GAUSSIANSAMPLER_H
