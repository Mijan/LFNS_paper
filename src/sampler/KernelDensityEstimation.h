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
    class KernelDensityEstimation;
} /* namespace sampler */

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::KernelDensityEstimation *t,
                                        const unsigned int file_version);
    } /* namespace boost */
}

namespace sampler {
    class KernelDensityEstimation : public DensityEstimation {
    public:
        KernelDensityEstimation(base::RngPtr rng, KernelSampler_ptr kernel, SamplerData data);

        virtual ~KernelDensityEstimation();

        friend class ::boost::serialization::access;

        virtual void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples);

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(
                Archive &ar, const ::sampler::KernelDensityEstimation *t,
                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int file_version) {
            ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(sampler::Sampler);
        }

    protected:
        std::vector<std::vector<double> > _means;
        base::EiVector _weights;
        KernelSampler_ptr _kernel;

        base::EiVector _log_likes_tmp;

        base::UniformRealDistribution _uniform_dist;
    };

    typedef std::shared_ptr<KernelDensityEstimation> KernelDensityEstimation_ptr;
}


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::KernelDensityEstimation *t,
                                        const unsigned int file_version) {

            std::size_t num_means = (t->_means).size();
            ar << num_means;

            int num_parameters = (t->_bounds).size();
            ar << num_parameters;

            for (std::size_t i = 0; i < num_means; i++) {
                std::vector<double> mean = t->_means[i];
                for (std::size_t j = 0; j < num_parameters; j++) {
                    double entry = mean[j];
                    ar << entry;
                }
            }

            int weight_size = t->_weights.size();

            ar << weight_size;
            std::vector<double> weights = t->_weights;
            for (std::size_t i = 0; i < weight_size; i++) {
                double weight = weights[i];
                if (std::isnan(weight) || std::isinf(weight)) {
                    std::cerr << "weight is " << weight << ", setting it to 0." << std::endl;
                    weight = 0;
                }
                ar << weight;
            }

            sampler::KernelSampler_ptr kernel = t->_kernel;
            sampler::KernelSampler *kernel_ptr = kernel.get();
            ar << kernel_ptr;


            for (int i = 0; i < t->_bounds.size(); i++) {
                ar << t->_bounds[i].first;
                ar << t->_bounds[i].second;
            }

        }

        template<class Archive>
        inline void load_construct_data(Archive &ar,
                                        sampler::KernelDensityEstimation *t, const unsigned int file_version) {

            std::size_t num_means;
            ar >> num_means;

            int num_dimensions;
            ar >> num_dimensions;

            base::EiMatrix means(num_means, num_dimensions);
            for (std::size_t i = 0; i < num_means; i++) {
                for (std::size_t j = 0; j < num_dimensions; j++) {
                    double mean;
                    ar >> mean;
                    means(i, j) = mean;
                }

            }

            int weight_number;
            ar >> weight_number;
            std::vector<double> weights(weight_number);
            for (std::size_t i = 0; i < weight_number; i++) {
                double weight;
                ar >> weight;
                weights[i] = weight;
            }

            sampler::KernelSampler *kernel_ptr;
            ar >> kernel_ptr;
            sampler::KernelSampler_ptr kernel(kernel_ptr);

            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>();

            std::vector<std::pair<double, double> > bounds(0);
            for (size_t i = 0; i < num_dimensions; i++) {
                std::pair<double, double> bound_pair = std::make_pair(0, 0);
                ar >> bound_pair.first;
                ar >> bound_pair.second;
                bounds.push_back(bound_pair);
            }

            sampler::SamplerData data(bounds.size());
            data.bounds = bounds;

            ::new(t) sampler::KernelDensityEstimation(rng, kernel, data);
            t->updateDensitySamples(means);

        }
    } /* namespace boost */
} /* namespace serialization */



#endif //LFNS_KERNELDENSITYESTIMATION_H
