//
// Created by jan on 09/10/18.
//

#ifndef LFNS_ELLIPSOIDSAMPLER_H
#define LFNS_ELLIPSOIDSAMPLER_H


#include <boost/serialization/nvp.hpp>
#include "DensityEstimation.h"

namespace sampler {
    class EllipsoidSampler;
} /* namespace sampler */

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::EllipsoidSampler *t,
                                        const unsigned int file_version);
    } /* namespace boost */
}

namespace sampler {
    class EllipsoidSampler : public DensityEstimation {
    public:
        EllipsoidSampler(base::RngPtr rng, SamplerData data);

        virtual ~EllipsoidSampler();

        void updateTransformedDensitySamples(const base::EiMatrix &transformed_samples) override;

        void sampleTransformed(base::EiVector &trans_sample) override;

        double getTransformedLogLikelihood(const base::EiVector &trans_sample) override;

        friend class ::boost::serialization::access;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(
                Archive &ar, const ::sampler::EllipsoidSampler *t,
                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int file_version) {
            ar & boost::serialization::base_object<DensityEstimation>(*this);
        }

        void setScale(double scale_var);

    protected:
        base::EiVector _mean;
        base::EiMatrixC _evs;
        base::EiVectorC _evals;
        double _scale;
        base::UniformRealDistribution _uniform_dist;
    };

    typedef std::shared_ptr<EllipsoidSampler> EllipsoidSampler_ptr;

}


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::EllipsoidSampler *t,
                                        const unsigned int file_version) {

//            int num_parameters = (t->_bounds).size();
//            ar << num_parameters;
//
//
//            double scale_var = t->_scale_var;
//
//            ar << scale_var;
//
//
//            base::EiMatrixC evs = t->_evs;
//            base::EiVectorC evals = t->_evals;
//
//            base::EiVector mean = t->_mean;
//            for (std::size_t j = 0; j < num_parameters; j++) {
//                double entry = mean(j);
//                ar << entry;
//
//                std::complex<double> comp = evals[j];
//                ar << comp.real();
//                ar << comp.imag();
//
//                for (std::size_t i = 0; i < num_parameters; i++) {
//                    comp = evs(j, i);
//                    ar << comp.real();
//                    ar << comp.imag();
//                }
//            }
//
//
//            for (int i = 0; i < t->_bounds.size(); i++) {
//                ar << t->_bounds[i].first;
//                ar << t->_bounds[i].second;
//            }

        }

        template<class Archive>
        inline void
        load_construct_data(Archive &ar, sampler::EllipsoidSampler *t, const unsigned int file_version) {

//            int num_parameters;
//            ar >> num_parameters;
//
//            double scale_var;
//            ar >> scale_var;
//
//
//            base::EiMatrixC evs(num_parameters, num_parameters);
//            base::EiVectorC evals(num_parameters);
//
//            base::EiVector mean(num_parameters);
//
//
//            for (std::size_t j = 0; j < num_parameters; j++) {
//                double entry;
//                ar >> entry;
//                mean(j) = entry;
//
//                double real;
//                double imag;
//
//                ar >> real;
//                ar >> imag;
//                evals(j).real(real);
//                evals(j).imag(imag);
//
//                for (std::size_t i = 0; i < num_parameters; i++) {
//
//                    ar >> real;
//                    ar >> imag;
//                    evs(j, i).real(real);
//                    evs(j, i).imag(imag);
//                }
//            }
//
//
//            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>();
//
//            std::vector<std::pair<double, double> > bounds(0);
//            for (size_t i = 0; i < num_parameters; i++) {
//                std::pair<double, double> bound_pair = std::make_pair(0, 0);
//                ar >> bound_pair.first;
//                ar >> bound_pair.second;
//                bounds.push_back(bound_pair);
//            }
//
//            sampler::SamplerData data(bounds.size());
//            data.bounds = bounds;
//
//            ::new(t) sampler::EllipsoidSampler(rng, data);
//            t->setMean(mean);
//            t->setEV(evs, evals);
//            t->setScale(scale_var);

        }
    } /* namespace boost */
} /* namespace serialization */

#endif //LFNS_ELLIPSOIDSAMPLER_H
