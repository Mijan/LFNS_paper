/*
 * WishartDistribution.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: jan
 */

#include "WishartDistribution.h"

#include <iostream>
#include <cmath>

namespace DP_GMM {

    WishartDistribution::WishartDistribution() : _dist_normal(0, 1), _dist_chi_sqr() { }

    WishartDistribution::~WishartDistribution() { }

    void WishartDistribution::wishRnd(const EiMatrix &Sigma, const int D,
                                      const double p, RngPtr r, EiMatrix *W) {
        double max = fabs(Sigma.maxCoeff());
        double min = fabs(Sigma.minCoeff());
        double scale = fmax(max, min);

        EiCholesky cholesky_sigma(Sigma * (1.0 / scale));
        wishRndDecomp(cholesky_sigma.matrixL(), D, p, r, W);
        (*W) = (*W) * scale;
    }

    void WishartDistribution::wishRndDecomp(const EiMatrix &A, const int D,
                                            const double p, RngPtr r, EiMatrix *W) {
        EiMatrix n_matrix(D, D);
        for (int i = 0; i < D; i++) {

            double v = _dist_chi_sqr(*r, p - i);
            double nTmp1 = 0.0;
            if (i > 0) {
                for (int k = 0; k < i; k++) {
                    nTmp1 += pow(n_matrix(k, i), 2.0);
                }
            }
            (*W)(i, i) = v + nTmp1;

            for (int j = i + 1; j < D; j++) {
                double u = _dist_normal(*r);
                n_matrix(i, j) = u;

                double nTmp2 = 0.0;
                if (i > 0) {

                    for (int k = 0; k < i; k++) {
                        nTmp2 += n_matrix(k, i) * n_matrix(k, j);
                    }
                }
                double b = n_matrix(i, j) * sqrt(v) + nTmp2;
                (*W)(i, j) = b;
                (*W)(j, i) = b;
            }
        }

        *W = (*W) * A.transpose();
        *W = A * (*W);
    }

} /* namespace DP_GMM */
