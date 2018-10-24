//
// Created by jan on 08/10/18.
//

#ifndef LFNS_MULTIVARIATENORMALDISTRIBUTION_H
#define LFNS_MULTIVARIATENORMALDISTRIBUTION_H

#include "EigenMatrices.h"
#include "RandomDistributions.h"

namespace base {

    class MultivariateNormal {
    public:
        static double mvnorm(const int n, const EiVectorRef &x, const EiVectorRef &mean,
                             const EiMatrix &var);

        static double mvnormPrepared(const int n, const EiVector &x, const EiVector &mean,
                                     const EiMatrix &inv, double detVar);

        static double mvnormPrepared(const int n, const std::vector<double> &x, const EiVector &mean,
                                     const EiMatrix &inv, double det_var);

        static double mvnormPrepared(const int n, const std::vector<double *> x, const EiVector &mean,
                                     const EiMatrix &inv, double det_var);

        static void mvnormRnd(RngPtr r, int n, const EiVector &mean,
                              const EiMatrix &var, EiVector *result);

        static void mvnormRndWithDecomposedVar(RngPtr r, int n, EiConstVectorRef mean,
                                               EiConstMatrixRef var, EiVector *result);

        static void mvnormRndWithDecomposedVar(RngPtr r, int n,
                                               std::vector<double *> mean, EiConstMatrixRef var,
                                               std::vector<double *> result);

        static void mvnormRnd(RngPtr r, int n, const EiVector &mean,
                              const EiMatrix &var, std::vector<double> *result);

        static void mvnormRnd(RngPtr r, int n, const EiVector &mean,
                              const EiMatrix &var, std::vector<double *> result);

        static void mvnormRndWithDecomposedVar(RngPtr r, int n,
                                               EiConstVectorRef mean, EiConstMatrixRef var,
                                               std::vector<double> *result);

        static void mvnormRndWithDecomposedVar(RngPtr r, int n,
                                               EiConstVectorRef mean, EiConstMatrixRef var,
                                               std::vector<double *> result);

        static void decomposeVar(const EiMatrix &var, EiMatrix *work_decomp_scal);

    private:
        static double _mvnormPreparedTransformedVar(const int n,
                                                    const EiVector &transformed_x,
                                                    const EiMatrix &inv,
                                                    double det_var);
    };

} /* namespace base*/



#endif //LFNS_MULTIVARIATENORMALDISTRIBUTION_H
