//
// Created by jan on 23/10/18.
//

#include "MpiRequest.h"
#include <math.h>
#include <cfloat>

namespace lfns {
    namespace mpi {
        MpiRequest::MpiRequest(std::size_t rank) : _rank(rank), _request(), _request_finished(true), _request_time(0),
                                                   _process_stopped(false) {}

        MpiRequest::~MpiRequest() {}

        bool MpiRequest::test() {
            if (!_request_finished) { _request_finished = (bool) _request.test(); }
            return _request_finished;
        }

        std::size_t MpiRequest::getRank() { return _rank; }

        bool MpiRequest::processStopped() { return _process_stopped; }

        void MpiRequest::interruptRequest() {
            if (!_process_stopped) {
                world.send(_rank, STOP_SIMULATION, true);
                _process_stopped = true;
            }
            prepareStopping();
        }

        void MpiRequest::prepareStopping() {
            world.send(_rank, INSTRUCTION, PREPARE_STOPPING);
            _process_stopped = false;
        }

        time_t MpiRequest::getClocksSinceRequest() { return clock() - _request_time; }

        MpiLikelihoodRequest::MpiLikelihoodRequest(std::size_t rank, const std::vector<double> &particle) : MpiRequest(
                rank) {

            world.send(_rank, INSTRUCTION, PREPARE_STOPPING);
            _process_stopped = false;
            world.send(_rank, INSTRUCTION, LIKELIHOOD_RECOMPU);
            world.send(_rank, PARTICLE, &particle[0], particle.size());

            _request = world.irecv(_rank, LIKELIHOOD_RECOMPU, _log_likelihood);
            _request_finished = false;
            _request_time = clock();
        }

        MpiLikelihoodRequest::~MpiLikelihoodRequest() {
            if (!_process_stopped) { world.send(_rank, STOP_SIMULATION, true); }
            if (!_request_finished) { _request.wait(); }
        }

        void MpiLikelihoodRequest::requestNewLikelihood(const std::vector<double> &particle) {
            if (!_request_finished) { _request.wait(); }

            world.send(_rank, INSTRUCTION, LIKELIHOOD_RECOMPU);
            world.send(_rank, PARTICLE, &particle[0], particle.size());
            _request = world.irecv(_rank, LIKELIHOOD_RECOMPU, _log_likelihood);
            _request_finished = false;
            _request_time = clock();
        }

        double MpiLikelihoodRequest::getLogLikelihood() { return _log_likelihood; }


        MpiParticleRequest::MpiParticleRequest(std::size_t rank, bool sample_prior) : MpiRequest(
                rank) {

            world.send(_rank, INSTRUCTION, PREPARE_STOPPING);
            _process_stopped = false;
            if (sample_prior) { world.send(_rank, INSTRUCTION, SAMPLE_PRIOR); }
            else { world.send(_rank, INSTRUCTION, SAMPLE_CONSTR_PRIOR); }
            _request = world.irecv(_rank, INSTRUCTION, instruction);
            _request_finished = false;
            _request_time = clock();
        }

        MpiParticleRequest::~MpiParticleRequest() {
            if (!_process_stopped) { world.send(_rank, STOP_SIMULATION, true); }
            if (!_request_finished) {
                _request.wait();
                double log_like;
                std::vector<double> particle;
                time_t clocks;
                receiveParticle(&log_like, &particle, &clocks);
            }
        }

        void MpiParticleRequest::requestNewParticle(bool sample_prior) {
            if (!_request_finished) { _request.wait(); }
            if (sample_prior) {
                world.send(_rank, INSTRUCTION, SAMPLE_PRIOR);
            } else {
                world.send(_rank, INSTRUCTION, SAMPLE_CONSTR_PRIOR);
            }
            _request = world.irecv(_rank, INSTRUCTION, instruction);
            _request_finished = false;
            _request_time = clock();
        }

        void MpiParticleRequest::receiveParticle(double *log_likelihood, std::vector<double> *particle_ptr,
                                                 time_t *sampling_clocks_ptr) {
            if (instruction == PARTICLE_ACCEPTED) {
                world.recv(_rank, LIKELIHOOD_RECOMPU, log_likelihood, 1);
                world.recv(_rank, PARTICLE, particle_ptr->data(), particle_ptr->size());
                world.recv(_rank, CLOCKS_SAMPLING, sampling_clocks_ptr, 1);
            } else if (instruction == PARTICLE_REJECTED) {
                *log_likelihood = -DBL_MAX;
            } else {
                std::stringstream ss;
                ss << "MPI error: Failed to receive computed particle. Expected instruction " << PARTICLE_ACCEPTED
                   << " or " << PARTICLE_REJECTED << ", but received: " << instruction << std::endl;
                throw std::runtime_error(ss.str());
            }
        }


//        bool MpiParticleRequest::test() { MpiRequest::test(); }
    }
}