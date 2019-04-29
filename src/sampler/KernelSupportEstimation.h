//
// Created by jan on 09/10/18.
//

#ifndef LFNS_KERNELSUPPORTESTIMATION_H
#define LFNS_KERNELSUPPORTESTIMATION_H

#include "KernelDensityEstimation.h"

namespace sampler {
    class KernelSupportEstimation;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::KernelSupportEstimation *t,
                                        const unsigned int file_version);
    }
}

namespace sampler {
    class KernelSupportEstimation : public KernelDensityEstimation {
    public:
        KernelSupportEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data);

        virtual ~KernelSupportEstimation() {}

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        friend class boost::serialization::access;

        template<class Archive>
        friend void
        ::boost::serialization::save_construct_data(Archive &ar, const ::sampler::KernelSupportEstimation *t,
                                                    const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & boost::serialization::base_object<sampler::KernelDensityEstimation>(*this);
        }

    };
}

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::KernelSupportEstimation *t, const unsigned int file_version) {
            // save data required to construct instance

            sampler::SamplerData data(t->getSamplerDimension());
            data.bounds = t->_bounds;
            ar << data;


            std::size_t size_of_kernel = sizeof(*t->_kernel.get());
            ar << size_of_kernel;
            ar << t->_kernel.get();
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::KernelSupportEstimation *t, const unsigned int file_version
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
            ::new(t)sampler::KernelSupportEstimation(rng, kernel_ptr, data);
        }
    }
}

#endif //LFNS_KERNELSUPPORTESTIMATION_H
