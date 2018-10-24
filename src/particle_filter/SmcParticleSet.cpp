//
// Created by jan on 11/10/18.
//

#include <iostream>
#include <sstream>
#include "SmcParticleSet.h"
#include "../base/MathUtils.h"

namespace particle_filter {
    SmcParticleSet::SmcParticleSet(base::RngPtr rng) : _particles(new std::vector<SmcParticle_ptr>()),
                                                       _particles_previous(new std::vector<SmcParticle_ptr>()),
                                                       _cum_weights(), _sum_weights(0.0), _rng(rng),
                                                       _dist_uniform(0, 1),
                                                       _ancestor_indices(), _original_particle_indices() {}

    SmcParticleSet::~SmcParticleSet() {
        delete _particles;
        delete _particles_previous;
    }

    void SmcParticleSet::resampleOldParticles() {
        std::vector<SmcParticle_ptr> *tmp_particles = _particles_previous;
        _particles_previous = _particles;
        _particles = tmp_particles;

        std::vector<SmcParticle_ptr> &curr_particles = *_particles;
        std::vector<SmcParticle_ptr> &prev_particles = *_particles_previous;

        _original_particle_indices_tmp.swap(_original_particle_indices);

        for (std::size_t i = 0; i < getTotalNumParticles(); i++) {
            double r = _dist_uniform(*_rng) * _sum_weights;
            int ancestor_index = base::MathUtils::findIndexLargerThan(_cum_weights, r);
            curr_particles[i]->replaceWith(*prev_particles[ancestor_index]);

            _ancestor_indices[i] = ancestor_index;
            _original_particle_indices[i] = _original_particle_indices_tmp[ancestor_index];

        }
        _sum_weights = 0.0;
        std::fill(_cum_weights.begin(), _cum_weights.end(), 0.0);
    }

    void SmcParticleSet::setWeight(std::size_t particle_nbr, double weight) {
        SmcParticle_ptr particle = (*_particles)[particle_nbr];
        particle->setWeight(weight);
        _sum_weights += weight;
        _cum_weights[particle_nbr] = _sum_weights;
    }

    std::vector<double> SmcParticleSet::getSmoothingWeights() {
        if (getTotalNumParticles() == 1) { return {1}; }

        std::vector<double> smoothing_weights(_particles->size(), 0);
        for (std::size_t i = 0; i < _original_particle_indices.size(); i++) {
            smoothing_weights[_original_particle_indices[i]] += (*_particles)[i]->weight();
        }

        return smoothing_weights;
    }

    std::size_t SmcParticleSet::getTotalNumParticles() const { return _particles->size(); }

    void SmcParticleSet::setTotalNumParticles(std::size_t num_particles, std::size_t particle_size) {
        std::size_t current_number_particles = getTotalNumParticles();
        if (num_particles == current_number_particles) {
            return;
        } else if (num_particles < current_number_particles) {
            _particles->resize(num_particles);
            _particles_previous->resize(num_particles);
            _original_particle_indices.resize(num_particles);
            _original_particle_indices_tmp.resize(num_particles);
        } else {
            _particles->clear();
            _particles_previous->clear();
            _particles->reserve(num_particles);
            _particles_previous->reserve(num_particles);
            for (std::size_t i = 0; i < num_particles; i++) {
                _particles->push_back(std::make_shared<SmcParticle>(particle_size));
                _particles_previous->push_back(std::make_shared<SmcParticle>(particle_size));
            }
            _original_particle_indices.clear();
            _original_particle_indices.reserve(num_particles);
            for (int i = 0; i < num_particles; i++) {
                _original_particle_indices.push_back(i);
                _original_particle_indices_tmp.push_back(i);
            }
        }
        _cum_weights.resize(num_particles);
        std::fill(_cum_weights.begin(), _cum_weights.end(), 0.0);
        _ancestor_indices.resize(num_particles);
        std::fill(_ancestor_indices.begin(), _ancestor_indices.end(), 0);
    }

    SmcParticle_ptr SmcParticleSet::getParticle(int particle_nbr) { return (*_particles)[particle_nbr]; }

    void SmcParticleSet::resetFullWeight() {
        std::fill(_cum_weights.begin(), _cum_weights.end(), 0);
        _sum_weights = 0;
    }

    double SmcParticleSet::getSumFullWeights() { return _sum_weights; }

    void SmcParticleSet::clear() {
        _original_particle_indices.clear();
        _original_particle_indices_tmp.clear();
        for (int i = 0; i < _particles->size(); i++) {
            _original_particle_indices.push_back(i);
            _original_particle_indices_tmp.push_back(i);
        }

        std::fill(_cum_weights.begin(), _cum_weights.end(), 0.0);
        std::fill(_ancestor_indices.begin(), _ancestor_indices.end(), 0);
        _sum_weights = 0;

        for (SmcParticle_ptr particle: *_particles) { particle->setWeight(1.0); }
        for (SmcParticle_ptr particle: *_particles_previous) { particle->setWeight(1); }
    }

    void SmcParticleSet::clearAncestorIndices() {
        std::fill(_ancestor_indices.begin(), _ancestor_indices.end(), 0);
        for (int i = 0; i < _particles->size(); i++) {
            _original_particle_indices[i] = i;
            _original_particle_indices_tmp[i] = i;
        }
    }

    void SmcParticleSet::resampleWithWeights(std::vector<double> weights) {

        if (weights.size() != getTotalNumParticles()) {
            std::ostringstream os;
            os << "Tried to resample " << getTotalNumParticles() << " particles, but provided " <<
               weights.size() << " weights!" << std::endl;

            throw std::runtime_error(os.str());
        }
        _original_particle_indices_tmp.swap(_original_particle_indices);

        double sum_weights = 0;
        for (double &weight : weights) { sum_weights += weight; }

        std::vector<SmcParticle_ptr> *tmp_particles = _particles_previous;
        _particles_previous = _particles;
        _particles = tmp_particles;

        std::vector<SmcParticle_ptr> &curr_particles = *_particles;
        std::vector<SmcParticle_ptr> &prev_particles = *_particles_previous;

        for (std::size_t i = 0; i < getTotalNumParticles(); i++) {
            double r = _dist_uniform(*_rng) * sum_weights;
            int ancestor_index = 0;
            double u = weights[ancestor_index];
            while (u < r) {
                ancestor_index++;
                u += weights[ancestor_index];
            }
            curr_particles[i]->replaceWith(*prev_particles[ancestor_index]);
            _ancestor_indices[i] = ancestor_index;
            _original_particle_indices[i] = _original_particle_indices_tmp[ancestor_index];
        }
    }


    std::vector<int> &SmcParticleSet::getAncestorIndices() { return _ancestor_indices; }
}