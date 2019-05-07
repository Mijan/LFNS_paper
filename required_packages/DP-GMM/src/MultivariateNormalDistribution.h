/*
 * dmvnorm.h
 *
 *  Created on: Dec 4, 2012
 *      Author: jan
 */

#ifndef DP_GMM2_MULTIVARIATENORMALDISTRIBUTION_H_
#define DP_GMM2_MULTIVARIATENORMALDISTRIBUTION_H_

#include <vector>

#include "BoostDistributions.h"
#include "MatrixTypes.h"

namespace DP_GMM {

class MultivariateNormalDistribution {
public:
	static double mvnorm(const int n, const EiVector& x, const EiVector& mean,
			const EiMatrix& var);

	static double mvnormPrepared(const int n, const EiVector& x,
			const EiVector& mean, const EiMatrix& inv, double detVar);
	static double mvnormPrepared(const int n,
			const std::vector<double>& x, const EiVector& mean, const EiMatrix& inv,
			double det_var);
	static void mvnormRnd(RngPtr r, int n, const EiVector& mean,
			const EiMatrix& var, EiVector* result);
	static void mvnormRndWithDecomposedVar(RngPtr r, int n,
			EiConstVectorRef mean, EiConstMatrixRef var, EiVector* result);
	static void mvnormRnd(RngPtr r, int n, const EiVector& mean,
			const EiMatrix& var, std::vector<double>* result);
	static void mvnormRndWithDecomposedVar(RngPtr r, int n,
			EiConstVectorRef mean, EiConstMatrixRef var,
			std::vector<double>* result);
	static void decomposeVar(const EiMatrix& var, EiMatrix* work_decomp_scal);

private:
	static double _mvnormPreparedTransformedVar(const int n,
			const EiVector& transformed_x, const EiMatrix& inv,
			double det_var);
};

} /* namespace DP_GMM2*/

#endif /* DP_GMM_MULTIVARIATENORMALDISTRIBUTION_H_ */
