/*
 * PostLogBeta.h
 *
 *  Created on: Jul 31, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_POSTLOGBETA_H_
#define DP_GMM2_POSTLOGBETA_H_

#include "LogConcaveFunction.h"

#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>

#include "EstimationMixtureComponent.h"
#include "MatrixTypes.h"

namespace DP_GMM {

    class PostLogBeta : public LogConcaveFunction {
    public:
        PostLogBeta(const EstimationMixtureComponentSet &components, const EiMatrix &W, int D);

        PostLogBeta(const EstimationMixtureComponentSet &components, const EiMatrix &W, int D, double feasible_point);

        virtual ~PostLogBeta();

        double operator()(double beta);

    private:
        int _K;
        const EstimationMixtureComponentSet &_components;
        const EiMatrix &_W;
        EiLU _lu_W;
        int _D;

        double _logEvaluate(double beta) const;
    };

} /* namespace DP_GMM2 */

#endif /* DP_GMM_POSTLOGBETA_H_ */
