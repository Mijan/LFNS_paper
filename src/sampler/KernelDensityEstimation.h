//
// Created by jan on 09/10/18.
//

#ifndef LFNS_KERNELDENSITYESTIMATION_H
#define LFNS_KERNELDENSITYESTIMATION_H

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/nvp.hpp>
#include <iostream>
#include <stdlib.h>
#include "DensityEstimation.h"

namespace sampler {
    class KernelDensityEstimation;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::KernelDensityEstimation *t,
                                        const unsigned int file_version);
    }
}

namespace sampler {
    class KernelDensityEstimation : public DensityEstimation {
    public:
        KernelDensityEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data);

        virtual ~KernelDensityEstimation();

        virtual void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples);

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        virtual void writeToStream(std::ostream &stream);

        void setRng(base::RngPtr rng) override;

    protected:
        std::vector<std::vector<double> > _means;
        base::EiVector _weights;
        KernelSampler_ptr _kernel;

        base::EiVector _log_likes_tmp;

        base::UniformRealDistribution _uniform_dist;


        friend class boost::serialization::access;

        template<class Archive>
        friend void
        ::boost::serialization::save_construct_data(Archive &ar, const KernelDensityEstimation *t,
                                                    const unsigned int file_version);


        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & boost::serialization::base_object<sampler::DensityEstimation>(*this);
            ar & _means;
            ar & _weights;
            ar & _log_likes_tmp;
        }
    };

    typedef std::shared_ptr<KernelDensityEstimation> KernelDensityEstimation_ptr;
}

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const ::sampler::KernelDensityEstimation *t, const unsigned int file_version) {
            // save data required to construct instance


            int sample_size = t->getSamplerDimension();
            sampler::SamplerData data(sample_size);
            data.bounds = t->_bounds;
            ar << data;

            std::size_t size_of_kernel = sizeof(*t->_kernel.get());
            ar << size_of_kernel;
            ar << t->_kernel.get();

        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, ::sampler::KernelDensityEstimation *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

            sampler::SamplerData data(1);
            ar >> data;

            std::size_t size_of_kernel;
            ar >> size_of_kernel;
            sampler::KernelSampler *target_ptr = (sampler::KernelSampler *) malloc(size_of_kernel);
            ar >> target_ptr;
            sampler::KernelSampler_ptr kernel_ptr(target_ptr);

            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::KernelDensityEstimation(rng, kernel_ptr, data);
        }
    }
}

#endif //LFNS_KERNELDENSITYESTIMATION_H
