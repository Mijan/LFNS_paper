/*
 * BoostDistributions.h
 *
 *  Created on: Feb 24, 2014
 *      Author: jan
 */

#ifndef DP_GMM2_BOOSTDISTRIBUTIONS_H_
#define DP_GMM2_BOOSTDISTRIBUTIONS_H_

#include <random>
#include <memory>

namespace DP_GMM {

    typedef std::mt19937 RandomNumberGenerator;
    typedef std::shared_ptr<std::mt19937> RngPtr;
    typedef std::uniform_int_distribution<int> UniformIntDistribution;
    typedef std::poisson_distribution<> PoissonDistribution;
    typedef std::normal_distribution<> NormalDistribution;
    typedef std::uniform_real_distribution<> UniformRealDistribution;
    typedef std::gamma_distribution<double> GammaDistribution;

    class DistChiSqrt {
    public:
        DistChiSqrt() : _dist_normal() { }

        virtual ~DistChiSqrt() { }

        double operator()(RandomNumberGenerator &rng, int k) {
            double x = 0.0;
            for (int i = 0; i < k; i++) {
                x += pow(_dist_normal(rng), 2);
            }
            return x;
        }

    private:
        NormalDistribution _dist_normal;
    };

} /* namespace DP_GMM2 */

#endif /* DP_GMM2_BOOSTDISTRIBUTIONS_H_ */
