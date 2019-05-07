/*
 * postLogAlpha.cpp
 *
 *  Created on: Jul 31, 2013
 *      Author: jan
 */

#include "PostLogAlpha.h"

#include "BoostDistributions.h"

namespace DP_GMM {

    PostLogAlpha::PostLogAlpha(int n, int K, double feasible_point) :
            LogConcaveFunction(), _n(n), _K(K) { _setRange(feasible_point, log(0.0000001), log(10000)); }

    PostLogAlpha::~PostLogAlpha() { }

    double PostLogAlpha::operator()(double log_alpha) {
        double trans_log_alpha = _inverseTransformInput(log_alpha);
        return exp(_logEvaluate(trans_log_alpha));
    }

    double PostLogAlpha::_logEvaluate(double log_alpha) const {
        double alpha = exp(log_alpha);
        double tmp = std::lgamma(alpha) - std::lgamma(_n + alpha);
        tmp = tmp + (_K - 3 / 2.0) * log_alpha + (-1.0 / (2.0 * alpha));
        tmp = tmp - _scale;
        return tmp + log_alpha;

    }

} /* namespace DP_GMM */
