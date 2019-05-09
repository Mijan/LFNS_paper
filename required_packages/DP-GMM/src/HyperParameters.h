/*
 * HyperParameters.h
 *
 *  Created on: Jul 17, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_HYPERPARAMETERS_H_
#define DP_GMM2_HYPERPARAMETERS_H_

#include <MatrixTypes.h>

#include "LogConcaveFunction.h"
#include "EstimationMixtureComponent.h"
#include "BoostDistributions.h"
#include "AdaptiveRejectionSampler.h"

namespace DP_GMM {

    class HyperParameters {
    public:
        HyperParameters(const Data &data, RngPtr r);

        HyperParameters(const EiMatrix &data, RngPtr r, std::string hyper_params_file_name);

        HyperParameters(const Data &data, RngPtr r, double alpha, double beta,
                        EiVector xi, EiMatrix W, double rho);

        virtual ~HyperParameters();

        double getAlpha() const;

        double getBeta() const;

        const EiVector &getXi() const;

        const EiMatrix &getW() const;

        double getRho() const;

        void updateAlpha(int num_samples, int num_components);

        void updateBeta(const EstimationMixtureComponentSet &components);

        void updateXi(const EstimationMixtureComponentSet &components);

        void updateW(const EstimationMixtureComponentSet &components);

        void updateRho(const EstimationMixtureComponentSet &components);

        void printHyperParams();

    private:
        RngPtr _r;
        int _D;
        EiVector _xi;
        EiMatrix _W;

        EiMatrix _sigma_data;
        EiVector _mu_data;

        EiMatrix _inv_sigma_data;
        AdaptiveRejectionSampler _sampler;

        double _alpha;
        double _beta;
        double _rho;

        WishartDistribution _dist_wishart;

        double _sampleLogConcave(LogConcaveFunction &fct);
    };

} /* namespace DP_GMM2 */

#endif /* DP_GMM2_HYPERPARAMETERS_H_ */
