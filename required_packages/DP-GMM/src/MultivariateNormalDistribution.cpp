/*
 * mvnorm.c
 *
 *  Created on: Dec 4, 2012
 *      Author: jan
 */

#include "MultivariateNormalDistribution.h"

#include <cmath>
#include <iostream>

#include <stdio.h>

namespace DP_GMM {

    double MultivariateNormalDistribution::mvnorm(const int n, const EiVector &x,
                                                  const EiVector &mean, const EiMatrix &var) {
        /* multivariate normal density function    */
        /*
         *	n	dimension of the random vetor
         *	mean	vector of means of size n
         *	var	variance matrix of dimension n x n
         */

        EiMatrix work_inverse = var.lu().inverse();
        double detVar = var.lu().determinant();

        return mvnormPrepared(n, x, mean, work_inverse, detVar);
    }

    double MultivariateNormalDistribution::mvnormPrepared(const int n,
                                                          const EiVector &x, const EiVector &mean, const EiMatrix &inv,
                                                          double det_var) {
        /* multivariate normal density function    */
        /*
         *	n	dimension of the random vector
         *	mean	vector of means of size n
         *	var	variance matrix of dimension n x n
         */

        return _mvnormPreparedTransformedVar(n, (x - mean), inv, det_var);
    }

    double MultivariateNormalDistribution::mvnormPrepared(const int n,
                                                          const std::vector<double> &x, const EiVector &mean,
                                                          const EiMatrix &inv,
                                                          double det_var) {
        /* multivariate normal density function    */
        /*
         *	n	dimension of the random vector
         *	mean	vector of means of size n
         *	var	variance matrix of dimension n x n
         */

        EiVector transformed_x(n);
        for (int i = 0; i < n; i++) {
            transformed_x(i) = x[i] - mean(i);
        }

        return _mvnormPreparedTransformedVar(n, transformed_x, inv, det_var);
    }

    double MultivariateNormalDistribution::_mvnormPreparedTransformedVar(
            const int n, const EiVector &transformed_x, const EiMatrix &inv,
            double det_var) {
        double ay = transformed_x.transpose() * inv * transformed_x;
        ay = exp(-0.5 * ay) / sqrt(pow((2 * M_PI), n) * det_var);

        return ay;
    }

    void MultivariateNormalDistribution::mvnormRnd(RngPtr r, int n,
                                                   const EiVector &mean, const EiMatrix &var, EiVector *result) {
        /* multivariate normal distribution random number generator */
        /*
         *    n       dimension of the random vector
         *    mean    vector of means of size n
         *    var     variance matrix of dimension n x n
         *    result  output variable with a single random vector normal distribution generation
         */

        EiMatrix work_decomp_scal(n, n);
        decomposeVar(var, &work_decomp_scal);
        mvnormRndWithDecomposedVar(r, n, mean, work_decomp_scal, result);
    }

    void MultivariateNormalDistribution::mvnormRndWithDecomposedVar(RngPtr r, int n,
                                                                    EiConstVectorRef mean, EiConstMatrixRef var,
                                                                    EiVector *result) {
        /* multivariate normal distribution random number generator */
        /*
         *    n       dimension of the random vector
         *    mean    vector of means of size n
         *    var     variance matrix of dimension n x n already decomposed using gsl_linalg_cholesky_decomp
         *    result  output variable with a single random vector normal distribution generation
         */
        NormalDistribution dist_normal(0, 1);
        for (int k = 0; k < n; k++) {
            double a = dist_normal(*r);
            (*result)(k) = a;
        }

        (*result) = var * (*result) + mean;
    }

    void MultivariateNormalDistribution::mvnormRnd(RngPtr r, int n,
                                                   const EiVector &mean, const EiMatrix &var,
                                                   std::vector<double> *result) {
        /* multivariate normal distribution random number generator */
        /*
         *    n       dimension of the random vector
         *    mean    vector of means of size n
         *    var     variance matrix of dimension n x n
         *    result  output variable with a single random vector normal distribution generation
         */

        EiMatrix work_decomp_scal(n, n);
        decomposeVar(var, &work_decomp_scal);
        mvnormRndWithDecomposedVar(r, n, mean, work_decomp_scal, result);
    }

    void MultivariateNormalDistribution::mvnormRndWithDecomposedVar(RngPtr r, int n,
                                                                    EiConstVectorRef mean, EiConstMatrixRef var,
                                                                    std::vector<double> *result) {
        /* multivariate normal distribution random number generator */
        /*
         *    n       dimension of the random vector
         *    mean    vector of means of size n
         *    var     variance matrix of dimension n x n already decomposed using gsl_linalg_cholesky_decomp
         *    result  output variable with a single random vector normal distribution generation
         */
        NormalDistribution dist_normal(0, 1);
        double a[n];
        for (int k = 0; k < n; k++) {
            a[k] = dist_normal(*r);
        }

        for (int j = 0; j < n; j++) {
            double var_x_result = 0;
            for (int l = 0; l <= j; l++) {
                var_x_result += var(j, l) * a[l];
            }
            (*result)[j] = var_x_result + mean[j];
        }
    }

    void MultivariateNormalDistribution::decomposeVar(const EiMatrix &var,
                                                      EiMatrix *work_decomp_scal) {
        double max = fabs(var.maxCoeff());
        double min = fabs(var.minCoeff());
        double scale = fmax(max, min);

        EiMatrix work = (1.0 / scale) * var;

        EiCholesky cholesky_work(work);
        *work_decomp_scal = cholesky_work.matrixL();
        (*work_decomp_scal) *= sqrt(scale);
    }
} /* namespace DP_GMM2*/
