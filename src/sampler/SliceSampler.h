//
// Created by jan on 28/02/19.
//

#ifndef LFNS_SLICESAMPLER_H
#define LFNS_SLICESAMPLER_H


#include "DensityEstimation.h"


namespace sampler {
    class SliceSampler;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::SliceSampler *t,
                                        const unsigned int file_version);
    }
}


namespace sampler {

    class SliceSampler : public DensityEstimation {
    public:
        SliceSampler(base::RngPtr rng, SamplerData data, LogLikelihodEvalFct_ptr log_like_fun, double *epsilon, int num_steps = 10);

        ~SliceSampler() override;

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        virtual void sampleTransformed(base::EiVector &trans_sample) override;

        virtual double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        virtual void updateLogLikelihoodFct(LogLikelihodEvalFct_ptr fct_ptr) override;

        void setLogScaleIndices(std::vector<int> log_scale_indices);


    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & _transformed_samples;
            ar & _decomposed_cov;
            ar & _num_steps;
            ar & _lower_bound;
            ar & _upper_bound;
            ar & _log_scale_indices;
        }

        double _log_like_eigen(base::EiVectorRef sample);

        void _stayWithinBounds(base::EiVector & sample);

        LogLikelihodEvalFct_ptr _log_like;
        base::EiMatrix _transformed_samples;
        base::EiMatrix _decomposed_cov;
        int _num_steps;
        base::UniformIntDistribution _sample_dist;
        base::UniformRealDistribution _uniform_dist;

        base::EiVector _lower_bound;
        base::EiVector _upper_bound;

        std::vector<int> _log_scale_indices;

        double *_epsilon;
    };
    typedef std::shared_ptr<SliceSampler> SliceSampler_ptr;
}


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::SliceSampler *t, const unsigned int file_version) {
            // save data required to construct instance

            int sample_size = t->getSamplerDimension();
            ar << sample_size;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::SliceSampler *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

            int sample_size;
            ar >> sample_size;
            sampler::SamplerData data(sample_size);

            sampler::LogLikelihodEvalFct_ptr fct_ptr(nullptr);
            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::SliceSampler(rng, data, fct_ptr, nullptr);
        }
    }
}


#endif //LFNS_SLICESAMPLER_H
