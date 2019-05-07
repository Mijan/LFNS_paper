/*
 * GaussMixtureComponent.h
 *
 *  Created on: Mar 17, 2014
 *      Author: jan
 */

#ifndef DP_GMM2_GAUSSMIXTURECOMPONENT_H_
#define DP_GMM2_GAUSSMIXTURECOMPONENT_H_

#include <unordered_set>

#include "MatrixTypes.h"
#include "MultivariateNormalDistribution.h"

namespace DP_GMM {

    class GaussMixtureComponent {
    public:
        GaussMixtureComponent(EiVector mean_, EiMatrix cov_, EiMatrix precision_,
                              double precision_det_, double comp_weight_) :
                mean(mean_), cov(cov_),
                decomposed_cov(cov_.rows(), cov_.cols()), precision(precision_), precision_det(
                precision_det_), comp_weight(comp_weight_) {
            MultivariateNormalDistribution::decomposeVar(cov, &decomposed_cov);
        }

        virtual ~GaussMixtureComponent() { }

        const EiVector mean;
        const EiMatrix cov;
        EiMatrix decomposed_cov;
        const EiMatrix precision;
        const double precision_det;
        const double comp_weight;
    };

    typedef std::shared_ptr<GaussMixtureComponent> GaussMixtureComponentPtr;
    typedef std::unordered_set<GaussMixtureComponentPtr> GaussMixtureComponentSet;

} /* namespace DP_GMM2 */

#endif /* DP_GMM2_GAUSSMIXTURECOMPONENT_H_ */
