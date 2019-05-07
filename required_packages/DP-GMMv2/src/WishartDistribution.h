/*
 * WishartDistribution.h
 *
 *  Created on: Jul 25, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_WISHARTDISTRIBUTION_H_
#define DP_GMM2_WISHARTDISTRIBUTION_H_

#include "BoostDistributions.h"
#include "MatrixTypes.h"

namespace DP_GMM {

    class WishartDistribution {
    public:
        WishartDistribution();

        virtual ~WishartDistribution();

        void wishRnd(const EiMatrix &Sigma, const int D, const double p,
                     RngPtr r, EiMatrix *W);

        void wishRndDecomp(const EiMatrix &A, const int D, const double p,
                           RngPtr r, EiMatrix *W);

    private:
        NormalDistribution _dist_normal;
        DistChiSqrt _dist_chi_sqr;
    };

} /* namespace DP_GMM2*/

#endif /* DP_GMM2_WISHARTDISTRIBUTION_H_ */
