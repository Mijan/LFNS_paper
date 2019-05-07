/*
 * postLogAlpha.h
 *
 *  Created on: Jul 31, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_POSTLOGALPHA_H_
#define DP_GMM2_POSTLOGALPHA_H_

#include "LogConcaveFunction.h"

namespace DP_GMM {

    class PostLogAlpha : public LogConcaveFunction {
    public:
        PostLogAlpha(int n, int K, double feasible_point = _FEASIBLE_POINT);

        virtual ~PostLogAlpha();

        double operator()(double alpha);

    private:
        int _n;
        int _K;

        double _logEvaluate(double log_alpha) const;

        static constexpr double _FEASIBLE_POINT = 0.0;
    };

} /* namespace DP_GMM2 */

#endif /* DP_GMM_POSTLOGALPHA_H_ */
