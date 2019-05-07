//
// Created by jan on 09/10/18.
//

#ifndef LFNS_UNIFORMSAMPLER_H
#define LFNS_UNIFORMSAMPLER_H


#include "Sampler.h"

namespace sampler {
    class UniformSampler;
}
namespace boost {
    namespace serialization {
        template<class Archive>
        inline void save_construct_data(Archive &ar, const sampler::UniformSampler *t,
                                        const unsigned int file_version);
    }
}


namespace sampler {

    class UniformSamplerData : public SamplerData {
    public:
        UniformSamplerData(int n) : SamplerData(n), widths(n, 1.0) {}

        UniformSamplerData(const SamplerData &rhs) : SamplerData(rhs), widths(rhs.size(), 1.0) {}

        UniformSamplerData(const UniformSamplerData &rhs) : SamplerData(rhs), widths(rhs.widths) {}

        UniformSamplerData &operator=(const SamplerData &rhs) {
            SamplerData::operator=(rhs);
            return *this;
        }

        UniformSamplerData &operator=(const UniformSamplerData &rhs) {
            SamplerData::operator=(rhs);
            widths = rhs.widths;
            return *this;
        }

        virtual ~UniformSamplerData() {};
        std::vector<double> widths;

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) { ar & widths; }
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


        void updateKernel(const base::EiMatrix &transformed_samples) override;

        void setWidths(std::vector<double> width);

        virtual void writeToStream(std::ostream &output_file);

    private:
        std::vector<double> _widths;

        base::UniformRealDistribution _uniform_dist;

        double _getRndInBounds(double l_bound, double r_bound);

        double _getLogLikelihood(double l_bound, double r_bound);

        friend class boost::serialization::access;

        template<class Archive>
        friend void
        ::boost::serialization::save_construct_data(Archive &ar, const ::sampler::UniformSampler *t,
                                                    const unsigned int file_version);

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            // serialize base class information
            ar & boost::serialization::base_object<sampler::Sampler>(*this);
            ar & boost::serialization::base_object<sampler::KernelSampler>(*this);
            ar & _widths;
        }

    };

    typedef std::shared_ptr<UniformSampler> UniformSampler_ptr;
}


namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const sampler::UniformSampler *t, const unsigned int file_version) {
            // save data required to construct instance
            sampler::UniformSamplerData data(t->getSamplerDimension());
            data.widths = t->_widths;
            ar << data;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, sampler::UniformSampler *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
            sampler::UniformSamplerData data(1);
            ar >> data;

            // invoke inplace constructor to initialize instance of my_class
            ::new(t)sampler::UniformSampler(rng, data);
        }
    }
}

#endif //LFNS_UNIFORMSAMPLER_H
