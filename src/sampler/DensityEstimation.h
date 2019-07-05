//
// Created by jan on 09/10/18.
//

#ifndef LFNS_SAMPLERPARTICLES_H
#define LFNS_SAMPLERPARTICLES_H

#include <boost/serialization/base_object.hpp>
#include "Sampler.h"
#include "../base/EigenMatrices.h"
#include <iostream>

namespace sampler {
    class DensityEstimation;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::DensityEstimation *t,
                                        const unsigned int file_version);
    }
}


namespace sampler {
    class DensityEstimation : public Sampler {
    public:

        DensityEstimation(base::RngPtr rng, SamplerData data);

        virtual ~DensityEstimation() {}

        void updateDensitySamples(base::EiMatrix &samples);

        virtual std::vector<double> &sample() override;

        virtual double getLogLikelihood(const std::vector<double> &sample);

        virtual void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) = 0;

        virtual void sampleTransformed(base::EiVector &trans_sample) = 0;

        virtual double getTransformedLogLikelihood(const base::EiVector &trans_sample) = 0;

        virtual void updateAcceptanceRate(double acceptance_rate){};

    private:
        base::EiVector _trans_sample;

    protected:
        base::EiVector _mean;
        base::EiMatrixC _evs;
        base::EiMatrixC _inv_evs;
        base::EiVectorC _evals;
        virtual void transformSample(base::EiVector &sample);

        virtual void retransformSample(base::EiVector &sample);

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            // serialize base class information
            ar & boost::serialization::base_object<sampler::Sampler>(*this);
            ar & _inv_evs;
            ar & _mean;
            ar & _evs;
            ar & _evals;
            ar & _trans_sample;
        }

    };

    typedef std::shared_ptr<DensityEstimation> DensityEstimation_ptr;

}

#endif //LFNS_SAMPLERPARTICLES_H
