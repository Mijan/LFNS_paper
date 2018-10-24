//
// Created by jan on 05/10/18.
//

#include "LFNSParticle.h"

namespace lfns {
    LFNSParticle::LFNSParticle(std::vector<double> theta, double log_likelihood) : _theta(theta),
                                                                                   _log_likelihood(log_likelihood) {};

    LFNSParticle::LFNSParticle(const LFNSParticle &rhs) : _theta(rhs._theta), _log_likelihood(rhs._log_likelihood) {}

    LFNSParticle::~LFNSParticle() {}

    double LFNSParticle::getLogLikelihood() const { return _log_likelihood; }

    const std::vector<double> &LFNSParticle::getTheta() const { return _theta; }
}