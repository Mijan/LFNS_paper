/*
 * IndicatorSampler.h
 *
 *  Created on: Jul 17, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_INDICATORSAMPLER_H_
#define DP_GMM2_INDICATORSAMPLER_H_

#include <unordered_map>

#include "BoostDistributions.h"
#include "EstimationMixtureComponent.h"
#include "HyperParameters.h"
#include "MatrixTypes.h"

namespace DP_GMM {

    class IndicatorParameters {
    public:
        IndicatorParameters(const EiMatrix &data,
                            EstimationMixtureComponentSet *mixture_components, RngPtr r);

        virtual ~IndicatorParameters();

        int getNumberComponents() const { return _num_components; }

        virtual void sampleIndicators(const HyperParameters &hyper_parameters);

    protected:
        const EiMatrix _data;
        int _num_components;

        EstimationMixtureComponentSet *_mixture_components;
        EstimationMixtureComponentPtr *_component_by_data_point;
        std::unordered_map<EstimationMixtureComponentPtr, double> _probs_by_components;
        size_t _D;

        double _alpha;
        double _beta;
        EiMatrix _W;
        double _rho;
        EiVector _xi;

        const EiVector _zero_vector;
        EiMatrix _inv_beta_W;
        double _log_det_beta_W;
        UniformRealDistribution dist_uniform;
        RngPtr _r;

        double _logLikeIndicators(int data_point_index, double component_weight,
                                  const EiVector &emp_component_mean, const EiMatrix &inv_W_star,
                                  double log_det_W_star, double log_gamma_val);

        void _sampleComponent(double sum_probs, double prob_new_comp,
                              int data_point_index);

        void _setHyperParams(const HyperParameters &hyper_parameters);

        void _updateMixtures();

        void _removeDataPointFromComponent(int i,
                                           EstimationMixtureComponentPtr current_mixture);

        void _computeProbs(double *sum_probs, double *prob_new_comp, int i);

        double _multivariateGammaLog(double x, double y, size_t D);

    };

} /* namespace DP_GMM2 */

#endif /* DP_GMM_INDICATORSAMPLER_H_ */
