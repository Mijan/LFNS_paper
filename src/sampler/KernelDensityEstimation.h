//
// Created by jan on 09/10/18.
//

#ifndef LFNS_KERNELDENSITYESTIMATION_H
#define LFNS_KERNELDENSITYESTIMATION_H

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/nvp.hpp>
#include <iostream>
#include "DensityEstimation.h"


namespace sampler {
    class KernelDensityEstimation : public DensityEstimation {
    public:
        KernelDensityEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data);

        virtual ~KernelDensityEstimation();

        friend class ::boost::serialization::access;

        virtual void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples);

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

    protected:
        std::vector<std::vector<double> > _means;
        base::EiVector _weights;
        KernelSampler_ptr _kernel;

        base::EiVector _log_likes_tmp;

        base::UniformRealDistribution _uniform_dist;
    };

    typedef std::shared_ptr<KernelDensityEstimation> KernelDensityEstimation_ptr;
}



#endif //LFNS_KERNELDENSITYESTIMATION_H
