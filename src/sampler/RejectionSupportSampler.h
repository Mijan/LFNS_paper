//
// Created by jan on 09/10/18.
//

#ifndef LFNS_REJECTIONSAMPLER_H
#define LFNS_REJECTIONSAMPLER_H


#include <boost/serialization/base_object.hpp>
#include <iostream>
#include "Sampler.h"
#include "DensityEstimation.h"


namespace sampler {
    class RejectionSupportSampler;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::RejectionSupportSampler *t,
                                        const unsigned int file_version);
    }
}


namespace sampler {
    class RejectionSamplerData : public SamplerData {
    public:
        explicit RejectionSamplerData(int n) : SamplerData(n), rejection_quantile(-1),
                                               rejection_quantile_low_accept(-1), log_rejection_const(-1),
                                               thresh_accept_rate(-1), max_rejection_mag(-1) {}

        virtual ~RejectionSamplerData() = default;

        RejectionSamplerData(const RejectionSamplerData &rhs) : SamplerData(rhs),
                                                                rejection_quantile(rhs.rejection_quantile),
                                                                rejection_quantile_low_accept(
                                                                        rhs.rejection_quantile_low_accept),
                                                                log_rejection_const(rhs.log_rejection_const),
                                                                thresh_accept_rate(rhs.thresh_accept_rate),
                                                                max_rejection_mag(rhs.max_rejection_mag) {}

        RejectionSamplerData(const SamplerData &rhs) : SamplerData(rhs), rejection_quantile(-1),
                                                       rejection_quantile_low_accept(-1), log_rejection_const(-1),
                                                       thresh_accept_rate(-1), max_rejection_mag(-1) {}

        RejectionSamplerData &operator=(const SamplerData &rhs) {
            if (this == &rhs) { return *this; }
            SamplerData::operator=(rhs);
            rejection_quantile = -1;
            rejection_quantile_low_accept = -1;
            log_rejection_const = -1;
            thresh_accept_rate = -1;
            max_rejection_mag = -1;
            return *this;
        }

        double rejection_quantile;
        double rejection_quantile_low_accept;
        double log_rejection_const;
        double thresh_accept_rate;
        double max_rejection_mag;

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & boost::serialization::base_object<sampler::SamplerData>(*this);
            ar & rejection_quantile;
            ar & rejection_quantile_low_accept;
            ar & log_rejection_const;
            ar & thresh_accept_rate;
            ar & max_rejection_mag;
        }
    };

    class RejectionSupportSampler : public DensityEstimation {
    public:
        RejectionSupportSampler(base::RngPtr rng, DensityEstimation_ptr sampler, RejectionSamplerData data);

        ~RejectionSupportSampler() override;

        void writeToStream(std::ostream &stream) override;

        void computeRejectionConst(const base::EiMatrix &transformed_samples);

        void updateSeed(int seed) override;

        void updateAcceptanceRate(double acceptance_rate);

        virtual void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples);

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        void setRng(base::RngPtr rng) override;


        virtual void setLogScale(int param_index) override;

    private:
        base::UniformRealDistribution _dist;
        DensityEstimation_ptr _current_sampler;
        RejectionSamplerData _rejection_data;

        double _log_rejection_const;
        double _rejection_quantile;


        friend class boost::serialization::access;

        template<class Archive>
        friend void
        ::boost::serialization::save_construct_data(Archive &ar, const ::sampler::RejectionSupportSampler *t,
                                                    const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & boost::serialization::base_object<sampler::DensityEstimation>(*this);
            ar & _log_rejection_const;
            ar & _rejection_quantile;
        }

    };

    typedef std::shared_ptr<sampler::RejectionSupportSampler> RejectionSupportSampler_ptr;
} /* namespace sampler */


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::RejectionSupportSampler *t, const unsigned int file_version) {
            // save data required to construct instance


            ar << t->_rejection_data;
            ar << t->_current_sampler.get();

        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::RejectionSupportSampler *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
            sampler::RejectionSamplerData data(1);
            ar >> data;

            sampler::DensityEstimation * sampler;
            ar >> sampler;

            sampler::DensityEstimation_ptr sampler_ptr(sampler);
            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::RejectionSupportSampler(rng, sampler_ptr, data);
        }
    }
}

#endif //LFNS_REJECTIONSAMPLER_H
