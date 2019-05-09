/*
 * GaussMixtureComponent.h
 *
 *  Created on: Mar 17, 2014
 *      Author: jan
 */

#ifndef DP_GMM2_GAUSSMIXTURECOMPONENT_H_
#define DP_GMM2_GAUSSMIXTURECOMPONENT_H_


#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/complex.hpp>

#include <unordered_set>
#include <iostream>
#include "MatrixTypes.h"
#include "MultivariateNormalDistribution.h"

namespace DP_GMM {

    class GaussMixtureComponent {
    public:
        GaussMixtureComponent(EiVector mean_, EiMatrix cov_, EiMatrix precision_,
                              double precision_det_, double comp_weight_) :
                mean(mean_), cov(cov_), decomposed_cov(cov_.rows(), cov_.cols()), precision(precision_),
                precision_det(precision_det_), comp_weight(comp_weight_) {
            MultivariateNormalDistribution::decomposeVar(cov, &decomposed_cov);
        }

        virtual ~GaussMixtureComponent() {}

        const EiVector mean;
        const EiMatrix cov;
        EiMatrix decomposed_cov;
        const EiMatrix precision;
        const double precision_det;
        const double comp_weight;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {}
    };

    typedef std::shared_ptr<GaussMixtureComponent> GaussMixtureComponentPtr;
    typedef std::unordered_set<GaussMixtureComponentPtr> GaussMixtureComponentSet;

} /* namespace DP_GMM2 */



namespace boost {
    namespace serialization {
        template<class Archive>
        inline void
        save_construct_data(Archive &ar, const DP_GMM::GaussMixtureComponent *t, const unsigned int file_version) {
            // save data required to construct instance
            int size = t->mean.size();
            DP_GMM::EiVector mean = t->mean;
            DP_GMM::EiMatrix cov = t->cov;
            DP_GMM::EiMatrix precision = t->precision;
            double precision_det = t->precision_det;
            int comp_weight = t->comp_weight;

            ar << size;
            ar << t->mean;
            ar << t->cov;
            ar << t->precision;
            ar << t->precision_det;
            ar << t->comp_weight;
        }

        template<class Archive>
        inline void load_construct_data(
                Archive &ar, DP_GMM::GaussMixtureComponent *t, const unsigned int file_version
        ) {
            // retrieve data from archive required to construct new instance
            int size;
            ar >> size;
            DP_GMM::EiVector mean(size);
            DP_GMM::EiMatrix cov(size, size);
            DP_GMM::EiMatrix precision(size, size);
            double precision_det = 0;
            double comp_weight;

            ar >> mean;
            ar >> cov;
            ar >> precision;
            ar >> precision_det;
            ar >> comp_weight;

            // invoke inplace constructor to initialize instance of my_class
            ::new(t)DP_GMM::GaussMixtureComponent(mean, cov, precision, precision_det, comp_weight);
        }
    }
}

#endif /* DP_GMM2_GAUSSMIXTURECOMPONENT_H_ */
