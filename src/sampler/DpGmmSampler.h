//
// Created by jan on 09/10/18.
//

#ifndef LFNS_DPGMMSAMPLER_H
#define LFNS_DPGMMSAMPLER_H


#include <GaussMixtureComponent.h>
#include <DPGMMEstimator.h>
#include <set>
#include "DensityEstimation.h"

namespace sampler {
    class DpGmmSamplerData : public SamplerData {
    public:
        DpGmmSamplerData(int n) : SamplerData(n), num_dp_iterations(0) {}

        virtual ~DpGmmSamplerData() {};

        DpGmmSamplerData(const SamplerData &rhs) : SamplerData(rhs), num_dp_iterations(0) {}

        DpGmmSamplerData &operator=(const SamplerData &rhs) {
            if (this == &rhs) {
                return *this;
            }
            SamplerData::operator=(rhs);
            return *this;
        }

        int num_dp_iterations;
    };

    class DpGmmSampler : public DensityEstimation {
    public:
        DpGmmSampler(base::RngPtr rng, DpGmmSamplerData data);

        DpGmmSampler(base::RngPtr rng, DpGmmSamplerData data,
                     std::vector<DP_GMM::GaussMixtureComponent> mixture_components);

        virtual ~DpGmmSampler();

        void writeToStream(std::ostream &stream);

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

    protected:
        std::vector<DP_GMM::GaussMixtureComponent> _mixture_components;
        base::UniformRealDistribution _dist;

        bool _hyper_params_set;
        DP_GMM::DPGMMEstimator _estimator;
        DP_GMM::HyperParameters _hyper_parameters;
        int _num_dp_gmm_iteration;


        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            // serialize base class information
            ar & boost::serialization::base_object<sampler::DensityEstimation>(*this);
            ar & _mixture_components;
            ar & _num_dp_gmm_iteration;
        }
    };

    typedef std::shared_ptr<DpGmmSampler> DpGmmSampler_ptr;
} /* namespace sampler */



#endif //LFNS_DPGMMSAMPLER_H
