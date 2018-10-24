//
// Created by jan on 09/10/18.
//

#ifndef LFNS_UNIFORMSAMPLER_H
#define LFNS_UNIFORMSAMPLER_H


#include <boost/serialization/base_object.hpp>
#include "Sampler.h"

namespace sampler {
    class UniformSampler;
}

namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::UniformSampler *t,
                                        const unsigned int file_version);
    } /* namespace serialization */
} /* namespace boost */

namespace sampler {

    class UniformSamplerData : public SamplerData {
    public:
        UniformSamplerData(int n) : SamplerData(n), widths(n, 1.0) {}

        UniformSamplerData(const SamplerData &rhs) : SamplerData(rhs), widths(rhs.size(), 1.0) {}

        UniformSamplerData &operator=(const SamplerData &rhs) {
            SamplerData::operator=(rhs);
            return *this;
        }

        virtual ~UniformSamplerData() {};
        std::vector<double> widths;
    };

    typedef std::shared_ptr<UniformSamplerData> UniformSamplerData_ptr;

    class UniformSampler : public Sampler, public virtual KernelSampler {
    public:
        UniformSampler(base::RngPtr rng, UniformSamplerData sampler_data);

        virtual ~UniformSampler();

        std::vector<double> &sample() override;

        virtual double getLogLikelihood(const std::vector<double> &sample);

        virtual std::vector<double> &sample(const std::vector<double> &kernel_center);

        virtual void sample(base::EiVector &sample, const std::vector<double> &kernel_center);

        virtual double getLogLikelihood(const std::vector<double> &sample,
                                        const std::vector<double> &kernel_center);

        virtual double getLogLikelihood(const base::EiVector &sample, const std::vector<double> &kernel_center);

        void setWidths(std::vector<double> width);

        friend class ::boost::serialization::access;

        template<class Archive>
        friend void ::boost::serialization::save_construct_data(
                Archive &ar, const ::sampler::UniformSampler *t,
                const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int file_version) {
            ar & boost::serialization::base_object<Sampler>(*this);
        }


        virtual void writeToStream(std::ostream &output_file);

    private:
        std::vector<double> _widths;

        base::UniformRealDistribution _uniform_dist;

        double _getRndInBounds(double l_bound, double r_bound);

        double _getLogLikelihood(double l_bound, double r_bound);

    };

    typedef std::shared_ptr<UniformSampler> UniformSampler_ptr;
}


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar,
                                        const sampler::UniformSampler *t,
                                        const unsigned int file_version) {
            int num_parameters = (t->_bounds).size();
            ar << num_parameters;
            for (int i = 0; i < t->_bounds.size(); i++) {
                ar << t->_bounds[i].first;
                ar << t->_bounds[i].second;
            }

            int width_size = t->_widths.size();
            ar << width_size;

            for (int j = 0; j < width_size; j++) {
                ar << t->_widths[j];
            }
        }

        template<class Archive>
        inline void load_construct_data(Archive &ar, sampler::UniformSampler *t,
                                        const unsigned int file_version) {


            size_t num_parameters = 0;
            ar >> num_parameters;
            std::vector<std::pair<double, double> > bounds(0);
            for (size_t i = 0; i < num_parameters; i++) {
                std::pair<double, double> bound_pair = std::make_pair(0, 0);
                ar >> bound_pair.first;
                ar >> bound_pair.second;
                bounds.push_back(bound_pair);
            }

            std::size_t size_widths;
            ar >> size_widths;


            sampler::UniformSamplerData data(bounds.size());
            data.bounds = bounds;

            if (size_widths > 0) {
                std::vector<double> widths(size_widths);
                for (std::size_t i = 0; i < size_widths; i++) {
                    ar >> widths[i];
                }
                data.widths = widths;
            }

            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>();
            // invoke inplace constructor to _initialize instance of my_class
            ::new(t) sampler::UniformSampler(rng, data);
        }
    }
}


#endif //LFNS_UNIFORMSAMPLER_H
