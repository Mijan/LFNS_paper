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
} /* namespace sampler */

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::RejectionSupportSampler *t,
                                        const unsigned int file_version);
    } /* namespace boost */
} /* namespace serialization */

namespace sampler {
    class RejectionSamplerData : public SamplerData {
    public:
        explicit RejectionSamplerData(int n) : SamplerData(n), rejection_quantile(-1),
                                               rejection_quantile_low_accept(-1),
                                               log_rejection_const(-1), thresh_accept_rate(-1), max_rejection_mag(-1) {}

        virtual ~RejectionSamplerData() = default;;

        RejectionSamplerData(const SamplerData &rhs) : SamplerData(rhs), rejection_quantile(-1),
                                                       rejection_quantile_low_accept(-1),
                                                       log_rejection_const(-1), thresh_accept_rate(-1),
                                                       max_rejection_mag(-1) {}

        RejectionSamplerData &operator=(const SamplerData &rhs) {
            if (this == &rhs) {
                return *this;
            }
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
    };

    class RejectionSupportSampler : public DensityEstimation {
    public:
        RejectionSupportSampler(base::RngPtr rng, DensityEstimation_ptr sampler, RejectionSamplerData data);

        ~RejectionSupportSampler() override;

        void writeToStream(std::ostream &stream) override;

        void computeRejectionConst(const base::EiMatrix &transformed_samples);

        friend class ::boost::serialization::access;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(
                Archive &ar, const ::sampler::RejectionSupportSampler *t,
                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int file_version) {
            ar & boost::serialization::base_object<DensityEstimation>(*this);
        }

        void updateSeed(int seed) override;

        void updateAcceptanceRate(double acceptance_rate);

        virtual void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples);

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

    private:
        base::UniformRealDistribution _dist;
        DensityEstimation_ptr _current_sampler;
        RejectionSamplerData _rejection_data;

        double _log_rejection_const;
        double _rejection_quantile;

    };

    typedef std::shared_ptr<sampler::RejectionSupportSampler> RejectionSupportSampler_ptr;
} /* namespace sampler */

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::RejectionSupportSampler *t,
                                        const unsigned int file_version) {
            sampler::DensityEstimation *current_sampler = t->_current_sampler.get();

            sampler::RejectionSamplerData rej_data = t->_rejection_data;
            double rejection_const = t->_log_rejection_const;
            double rejection_quant = t->_rejection_quantile;

            double max_mag = rej_data.max_rejection_mag;
            double rejection_quantile_low_accept = rej_data.rejection_quantile_low_accept;
            double thresh_accept_rate = rej_data.thresh_accept_rate;
            ar << current_sampler;
            ar << rejection_const;
            ar << rejection_quant;
            ar << max_mag;
            ar << rejection_quantile_low_accept;
            ar << thresh_accept_rate;

            int num_parameters = (t->_bounds).size();
            ar << num_parameters;

            for (int i = 0; i < t->_bounds.size(); i++) {
                ar << t->_bounds[i].first;
                ar << t->_bounds[i].second;
            }

        }

        template<class Archive>
        inline void load_construct_data(Archive &ar,
                                        sampler::RejectionSupportSampler *t, const unsigned int file_version) {
            sampler::DensityEstimation *current_sampler;
            double rejection_const = -1.0;
            double rejection_quant = -1.0;
            double max_mag = -1;
            double rejection_quantile_low_accept = -1;
            double thresh_accept_rate = -1;

            ar >> current_sampler;
            ar >> rejection_const;
            ar >> rejection_quant;
            ar >> max_mag;
            ar >> rejection_quantile_low_accept;
            ar >> thresh_accept_rate;

            sampler::DensityEstimation_ptr current_sampler_ptr(current_sampler);

            int num_parameters = 0;
            ar >> num_parameters;
            std::vector<std::pair<double, double> > bounds(0);
            for (size_t i = 0; i < num_parameters; i++) {
                std::pair<double, double> bound_pair = std::make_pair(0, 0);
                ar >> bound_pair.first;
                ar >> bound_pair.second;
                bounds.push_back(bound_pair);
            }

            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>();

            sampler::RejectionSamplerData data(bounds.size());
            data.bounds = bounds;
            data.log_rejection_const = rejection_const;
            data.rejection_quantile = rejection_quant;
            data.max_rejection_mag = max_mag;
            data.rejection_quantile_low_accept = rejection_quantile_low_accept;
            data.thresh_accept_rate = thresh_accept_rate;

            ::new(t) sampler::RejectionSupportSampler(rng, current_sampler_ptr, data);
        }
    } /* namespace boost */
} /* namespace serialization */

#endif //LFNS_REJECTIONSAMPLER_H
