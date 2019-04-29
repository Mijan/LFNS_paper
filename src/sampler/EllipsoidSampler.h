//
// Created by jan on 09/10/18.
//

#ifndef LFNS_ELLIPSOIDSAMPLER_H
#define LFNS_ELLIPSOIDSAMPLER_H


#include <boost/serialization/nvp.hpp>
#include "DensityEstimation.h"

namespace sampler {
    class EllipsoidSampler;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::EllipsoidSampler *t,
                                        const unsigned int file_version);
    }
}


namespace sampler {
    class EllipsoidSampler : public DensityEstimation {
    public:
        EllipsoidSampler(base::RngPtr rng, SamplerData data);

        virtual ~EllipsoidSampler();

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        void setScale(double scale_var);

        virtual void writeToStream(std::ostream &stream);

    protected:
        double _scale;
        base::UniformRealDistribution _uniform_dist;

        friend class boost::serialization::access;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(Archive &ar, const ::sampler::EllipsoidSampler *t,
                                                                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            // serialize base class information
            ar & boost::serialization::base_object<sampler::DensityEstimation>(*this);
            ar & _scale;
        }
    };

    typedef std::shared_ptr<EllipsoidSampler> EllipsoidSampler_ptr;

}

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::EllipsoidSampler *t, const unsigned int file_version) {
            // save data required to construct instance


            int sample_size = t->getSamplerDimension();
            sampler::SamplerData data(sample_size);
            data.bounds = t->_bounds;
            ar << data;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::EllipsoidSampler *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

            sampler::SamplerData data(1);
            ar >> data;

            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::EllipsoidSampler(rng, data);
        }
    }
}

#endif //LFNS_ELLIPSOIDSAMPLER_H
