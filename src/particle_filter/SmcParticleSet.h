//
// Created by jan on 11/10/18.
//

#ifndef LFNS_SMCPARTICLESET_H
#define LFNS_SMCPARTICLESET_H

#include "../base/RandomDistributions.h"
#include "SmcParticle.h"

namespace particle_filter {

    class SmcParticleSet {

    public:
        SmcParticleSet(base::RngPtr rng);

        virtual ~SmcParticleSet();

        void resampleOldParticles();

        void setWeight(std::size_t particle_nbr, double weight);

        std::vector<double> getSmoothingWeights();

        std::size_t getTotalNumParticles() const;

        virtual void setTotalNumParticles(std::size_t num_particles, std::size_t particle_size);

        SmcParticle_ptr getParticle(int particle_nbr);

        void resetFullWeight();

        double getSumFullWeights();

        void clear();

        void clearAncestorIndices();

        void resampleWithWeights(std::vector<double> weights);

        std::vector<int>& getAncestorIndices();

    protected:
        std::vector<SmcParticle_ptr> *_particles;
        std::vector<SmcParticle_ptr> *_particles_previous;

        std::vector<double> _cum_weights;
        double _sum_weights;
        base::RngPtr _rng;
        base::UniformRealDistribution _dist_uniform;

        std::vector<int> _ancestor_indices;
        std::vector<int> _original_particle_indices;
        std::vector<int> _original_particle_indices_tmp;
    };

    typedef std::shared_ptr<SmcParticleSet> SmcParticleSet_ptr;
}
#endif //LFNS_SMCPARTICLESET_H
