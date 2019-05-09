/*
 * PostLogBeta.cpp
 *
 *  Created on: Jul 31, 2013
 *      Author: jan
 */

#include "PostLogBeta.h"

namespace DP_GMM {

    PostLogBeta::PostLogBeta(const EstimationMixtureComponentSet &components,
                             const EiMatrix &W, int D) :
            LogConcaveFunction(), _K(components.size()), _components(components), _W(
            W), _lu_W(W), _D(D) {
        _setRange(log(D), log(D - 1 + 0.001), log(100 * D));
    }

    PostLogBeta::PostLogBeta(const EstimationMixtureComponentSet &components,
                             const EiMatrix &W, int D, double feasible_point) :
            LogConcaveFunction(), _K(components.size()), _components(components), _W(
            W), _lu_W(W), _D(D) {
        _setRange(feasible_point, log(D - 1 + 0.001), log(100 * D));
    }

    PostLogBeta::~PostLogBeta() { }

    double PostLogBeta::operator()(double log_beta) {
        double trans_log_beta = _inverseTransformInput(log_beta);
        return exp(_logEvaluate(trans_log_beta));
    }

    double PostLogBeta::_logEvaluate(double log_beta) const {
        double beta = exp(log_beta);
        double gamma_val = 0;
        for (int i = 1; i <= _D; i++) {
            gamma_val = gamma_val + std::lgamma((beta + 1 - i) / 2.0);
        }

        double p = 0.0;

        for (EstimationMixtureComponentSet::const_iterator it = _components.begin();
             it != _components.end(); it++) {

            EstimationMixtureComponentPtr comp = *it;
            double tr = (_W * comp->getPrecision()).trace();

            p += (beta / 2.0)
                 * (log(comp->getPrecisionDet()) + log(_lu_W.determinant()))
                 - (beta / 2.0) * tr;
        }

        double tmp = (-_D / (double) (2.0 * (beta - _D + 1)))
                     + log(pow(beta - _D + 1, -3 / 2.0))
                     + (_K * beta * _D / 2.0) * log((beta / 2.0)) - _K * gamma_val + p;
        tmp = tmp - _scale;
        return tmp + log_beta;
    }

} /* namespace DP_GMM */

