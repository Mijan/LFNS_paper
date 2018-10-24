//
// Created by jan on 05/10/18.
//

#ifndef LFNS_LFNSPARTICLE_H
#define LFNS_LFNSPARTICLE_H

#include <vector>

namespace lfns {

    class LFNSParticle {
    public:
        LFNSParticle(std::vector<double> theta, double log_likelihood);

        LFNSParticle(const LFNSParticle &rhs);

        virtual ~LFNSParticle();

        double getLogLikelihood() const;

        const std::vector<double> &getTheta() const;

    private:
        std::vector<double> _theta;
        double _log_likelihood;
    };


    struct lfns_particle_comp {
        bool operator()(const LFNSParticle &lhs, const LFNSParticle &rhs) const {
            return lhs.getLogLikelihood() < rhs.getLogLikelihood();
        }
    };
};


#endif //LFNS_LFNSPARTICLE_H
