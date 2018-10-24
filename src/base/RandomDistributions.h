//
// Created by jan on 08/10/18.
//

#ifndef LFNS_RANDOMDISTRIBUTIONS_H
#define LFNS_RANDOMDISTRIBUTIONS_H

#include <random>
#include <memory>

namespace base {

typedef std::mt19937 RandomNumberGenerator;
typedef std::shared_ptr<std::mt19937> RngPtr;
typedef std::uniform_int_distribution<int> UniformIntDistribution;
typedef std::poisson_distribution<> PoissonDistribution;
typedef std::normal_distribution<> NormalDistribution;
typedef std::uniform_real_distribution<> UniformRealDistribution;

} /* namespace base */
#endif //LFNS_RANDOMDISTRIBUTIONS_H
