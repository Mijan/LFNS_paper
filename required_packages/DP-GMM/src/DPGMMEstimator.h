/*
 * DPGMMEstimatorPartiallyWeighted.h
 *
 *  Created on: Apr 22, 2014
 *      Author: jan
 */

#ifndef DP_GMM2_DPGMMESTIMATOR_H_
#define DP_GMM2_DPGMMESTIMATOR_H_

#include <Eigen/Dense>

#include "EstimationMixtureComponent.h"
#include "BoostDistributions.h"
#include "HyperParameters.h"
#include "DPGMMLogger.h"

namespace DP_GMM {

    class DPGMMEstimator {
    public:
        DPGMMEstimator(RngPtr r, int num_dp_iterations);

        virtual ~DPGMMEstimator();

        void estimate(const Data &data, HyperParameters &hyper_parameters,
                      EstimationMixtureComponentSet *mixture_components, bool verbose = true,
                      DPGMMLogger *logger = NULL);

        HyperParameters estimate(const EiMatrix &data,
                                 EstimationMixtureComponentSet *mixture_components, bool verbose = true,
                                 DPGMMLogger *logger = NULL);

    private:
        void _estimate(const Data &data, HyperParameters &hyper_parameters,
                       EstimationMixtureComponentSet *mixture_components, bool verbose = true,
                       DPGMMLogger *logger = NULL);

    private:
        RngPtr _r;
        int _num_dp_iterations;
    };

} /* namespace DP_GMM */

#endif /* DP_GMM_DPGMMESTIMATOR_H_ */
