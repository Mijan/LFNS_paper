//
// Created by jan on 11/10/18.
//

#ifndef LFNS_SMCPARTICLE_H
#define LFNS_SMCPARTICLE_H


#include <vector>
#include <memory>

namespace particle_filter {


    class SmcParticle {
    public:
        SmcParticle(std::size_t state_size) : state(state_size, 0.0), time(0.0), _weight(0.0) {}

        SmcParticle(const std::vector<double> &state_, double t) : state(state_), time(t), _weight(0.0) {}

        virtual ~SmcParticle() {};

        std::vector<double> state;
        double time;

        double weight() { return _weight; }

        void setWeight(double weight) { _weight = weight; }

        virtual void replaceWith(const SmcParticle &rhs) {
            state = rhs.state;
            time = rhs.time;
            _weight = rhs._weight;
        }

    protected:
        double _weight;

    };

    typedef std::shared_ptr<SmcParticle> SmcParticle_ptr;

}
#endif //LFNS_SMCPARTICLE_H
