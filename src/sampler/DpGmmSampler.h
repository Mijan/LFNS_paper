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
    class DpGmmSampler;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::DpGmmSampler *t,
                                        const unsigned int file_version);
    }
}


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


    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & num_dp_iterations;
        }
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
        friend void ::boost::serialization::save_construct_data(Archive &ar, const ::sampler::DpGmmSampler *t,
                                                                const unsigned int file_version);

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

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::DpGmmSampler *t, const unsigned int file_version) {
            // save data required to construct instance

            int sample_size = t->getSamplerDimension();
            int num_dp_it = t->_num_dp_gmm_iteration;
            ar << sample_size;
            ar << num_dp_it;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::DpGmmSampler *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

            int sample_size;
            ar >> sample_size;
            int num_dp_iterations;
            ar >> num_dp_iterations;
            sampler::DpGmmSamplerData data(sample_size);
            data.num_dp_iterations = num_dp_iterations;

            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::DpGmmSampler(rng, data);
        }
    }
}


#endif //LFNS_DPGMMSAMPLER_H
