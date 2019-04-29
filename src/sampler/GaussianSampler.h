//
// Created by jan on 09/10/18.
//

#ifndef LFNS_GAUSSIANSAMPLER_H
#define LFNS_GAUSSIANSAMPLER_H

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <iostream>
#include "Sampler.h"
#include "../base/EigenMatrices.h"

namespace sampler {
    class GaussianSampler;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::GaussianSampler *t,
                                        const unsigned int file_version);
    }
}

namespace sampler {

    class NormalSamplerData : public SamplerData {
    public:
        NormalSamplerData(int n) : SamplerData(n), mean(base::EiVector::Zero(n)),
                                   cov(base::EiMatrix::Identity(n, n) * 0.1) {}

        NormalSamplerData(const NormalSamplerData &rhs) : SamplerData(rhs), mean(rhs.mean), cov(rhs.cov) {}

        NormalSamplerData(const SamplerData &rhs) : SamplerData(rhs), mean(base::EiVector::Zero(rhs.size())),
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

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & boost::serialization::base_object<sampler::SamplerData>(*this);
            ar & bounds;
            ar & mean;
            ar & cov;
        }
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

        void updateKernel(const base::EiMatrix &transformed_samples) override;

        virtual void writeToStream(std::ostream &stream);

    private:
        base::EiVector _mean;
        base::EiMatrix _inverse_cov_matrix;
        base::EiMatrix _decomposed_var;
        base::EiMatrix _cov;
        double _det_var;

        int _n;

        friend class boost::serialization::access;

        template<class Archive>
        friend void
        ::boost::serialization::save_construct_data(Archive &ar, const ::sampler::GaussianSampler *t,
                                                    const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            // serialize base class information
            ar & boost::serialization::base_object<sampler::KernelSampler>(*this);
            ar & boost::serialization::base_object<sampler::Sampler>(*this);
            ar & _inverse_cov_matrix;
            ar & _decomposed_var;
            ar & _det_var;
            ar & _n;
        }
    };

    typedef std::shared_ptr<GaussianSampler> GaussianSampler_ptr;
}

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::GaussianSampler *t, const unsigned int file_version) {
            // save data required to construct instance
            sampler::NormalSamplerData data(t->getSamplerDimension());
            data.mean = t->_mean;
            data.cov = t->_cov;
            ar << data;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::GaussianSampler *t, const unsigned int file_version
        ) {
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
            sampler::NormalSamplerData data(1);
            ar >> data;
            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::GaussianSampler(rng, data);
        }
    }
}

#endif //LFNS_GAUSSIANSAMPLER_H
