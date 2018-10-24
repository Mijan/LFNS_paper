//
// Created by jan on 09/10/18.
//

#ifndef LFNS_DPGMMSAMPLER_H
#define LFNS_DPGMMSAMPLER_H


#include <GaussMixtureComponent.h>
#include <DPGMMEstimator.h>
#include "DensityEstimation.h"

namespace sampler {
    class DpGmmSampler;
} /* namespace sampler */

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::DpGmmSampler *t, const unsigned int file_version);
    } /* namespace boost */
} /* namespace serialization */

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
                     DP_GMM::GaussMixtureComponentSet mixture_components);

        virtual ~DpGmmSampler();

        void writeToStream(std::ostream &stream);

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        friend class ::boost::serialization::access;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(
                Archive &ar, const ::sampler::DpGmmSampler *t,
                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int file_version) {
            ar & boost::serialization::base_object<DensityEstimation>(*this);
        }

    protected:
        DP_GMM::GaussMixtureComponentSet _mixture_components;
        base::UniformRealDistribution _dist;
        base::EiVector _sample_ei_vector;

        bool _hyper_params_set;
        DP_GMM::DPGMMEstimator _estimator;
        DP_GMM::HyperParameters _hyper_parameters;
        int _num_dp_gmm_iteration;
    };

    typedef std::shared_ptr<DpGmmSampler> DpGmmSampler_ptr;
} /* namespace sampler */

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::DpGmmSampler *t,
                                        const unsigned int file_version) {
            int num_parameters = (t->_bounds).size();
            ar << num_parameters;

            for (int i = 0; i < t->_bounds.size(); i++) {
                ar << t->_bounds[i].first;
                ar << t->_bounds[i].second;
            }

            int num_comps = t->_mixture_components.size();
            ar << num_comps;

            for (DP_GMM::GaussMixtureComponentSet::const_iterator it =
                    t->_mixture_components.begin(); it != t->_mixture_components.end();
                 it++) {
                DP_GMM::GaussMixtureComponentPtr comp = *it;

                double comp_weight = comp->comp_weight;
                ar << comp_weight;

                double precision_det = comp->precision_det;
                ar << precision_det;

                const base::EiVector &mean = comp->mean;
                for (int k = 0; k < mean.size(); k++) {
                    ar << mean(k);
                }

                const base::EiMatrix &cov = comp->cov;
                const base::EiMatrix &precision = comp->precision;
                for (int row = 0; row < cov.rows(); row++) {
                    for (int col = 0; col < cov.cols(); col++) {
                        ar << cov(row, col);
                        ar << precision(row, col);
                    }
                }
            }
            int num_dp_gmm_iteartion = t->_num_dp_gmm_iteration;
            ar << num_dp_gmm_iteartion;

        }

        template<class Archive>
        inline void load_construct_data(Archive &ar, sampler::DpGmmSampler *t,
                                        const unsigned int file_version) {
            int num_parameters = 0.0;
            ar >> num_parameters;

            std::vector<std::pair<double, double> > bounds(0);
            for (size_t i = 0; i < num_parameters; i++) {
                std::pair<double, double> bound_pair = std::make_pair(0, 0);
                ar >> bound_pair.first;
                ar >> bound_pair.second;
                bounds.push_back(bound_pair);
            }

            int num_comps = 0;
            ar >> num_comps;
            DP_GMM::GaussMixtureComponentSet new_components;
            for (int j = 0; j < num_comps; j++) {

                double comp_weight;
                ar >> comp_weight;


                double precision_det;
                ar >> precision_det;

                base::EiVector mean(num_parameters);
                for (int k = 0; k < num_parameters; k++) {
                    double entry = 0.0;
                    ar >> entry;
                    mean(k) = entry;
                }

                base::EiMatrix cov(num_parameters, num_parameters);
                base::EiMatrix precision(num_parameters, num_parameters);
                for (int row = 0; row < cov.rows(); row++) {
                    for (int col = 0; col < cov.cols(); col++) {
                        double entry = 0.0;
                        ar >> entry;
                        cov(row, col) = entry;
                        ar >> entry;
                        precision(row, col) = entry;
                    }
                }
                new_components.insert(
                        std::make_shared<DP_GMM::GaussMixtureComponent>(mean, cov,
                                                                        precision, precision_det, comp_weight));
            }

            int num_dp_gmm_iterations;
            ar >> num_dp_gmm_iterations;

            sampler::DpGmmSamplerData data(num_parameters);
            data.bounds = bounds;
            data.num_dp_iterations = num_dp_gmm_iterations;

            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>();
            // invoke inplace constructor to _initialize instance of my_class
            ::new(t) sampler::DpGmmSampler(rng, data, new_components);
        }
    } /* namespace serialization */
} /* namespace boost */



#endif //LFNS_DPGMMSAMPLER_H
