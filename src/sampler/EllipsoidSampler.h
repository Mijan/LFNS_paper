//
// Created by jan on 09/10/18.
//

#ifndef LFNS_ELLIPSOIDSAMPLER_H
#define LFNS_ELLIPSOIDSAMPLER_H


#include <boost/serialization/nvp.hpp>
#include "DensityEstimation.h"

namespace sampler {
    class EllipsoidSampler : public DensityEstimation {
    public:
        EllipsoidSampler(base::RngPtr rng, SamplerData data);

        virtual ~EllipsoidSampler();

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        void setScale(double scale_var);

    protected:
        double _scale;
        base::UniformRealDistribution _uniform_dist;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            // serialize base class information
            ar & boost::serialization::base_object<sampler::DensityEstimation>(*this);
            ar & _scale;
        }
    };

    typedef std::shared_ptr<EllipsoidSampler> EllipsoidSampler_ptr;

}

#endif //LFNS_ELLIPSOIDSAMPLER_H
