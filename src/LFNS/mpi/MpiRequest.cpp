//
// Created by jan on 23/10/18.
//

#include "MpiRequest.h"

namespace lfns {
    namespace mpi {
        MpiRequest::MpiRequest(std::size_t rank) : _rank(rank), _request(), _request_time(0), _process_stopped(false) {}

        MpiRequest::~MpiRequest() {}

        bool MpiRequest::test() { return (bool) _request.test(); }

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
            _request_time = clock();
        }

        MpiLikelihoodRequest::~MpiLikelihoodRequest() {
            if (!_process_stopped) { world.send(_rank, STOP_SIMULATION, true); }
            if (!_request.test()) { _request.wait(); }
        }

        void MpiLikelihoodRequest::requestNewLikelihood(const std::vector<double> &particle) {
            if (!_request.test()) { _request.wait(); }

            world.send(_rank, INSTRUCTION, LIKELIHOOD_RECOMPU);
            world.send(_rank, PARTICLE, &particle[0], particle.size());
            _request = world.irecv(_rank, LIKELIHOOD_RECOMPU, _log_likelihood);
            _request_time = clock();
        }

        double MpiLikelihoodRequest::getLogLikelihood() { return _log_likelihood; }


        MpiParticleRequest::MpiParticleRequest(std::size_t rank, int num_parameters, bool sample_prior) : MpiRequest(
                rank), _particle(num_parameters), _log_likelihood(0.0) {

            world.send(_rank, INSTRUCTION, PREPARE_STOPPING);
            _process_stopped = false;
            if (sample_prior) {
                world.send(_rank, INSTRUCTION, SAMPLE_PRIOR);
            } else {
                world.send(_rank, INSTRUCTION, SAMPLE_CONSTR_PRIOR);
            }

            world.irecv(_rank, PARTICLE, _particle.data(), _particle.size());
            world.irecv(_rank, CLOCKS_SAMPLING, _clocks_for_sampling);
            _request = world.irecv(_rank, LIKELIHOOD_RECOMPU, _log_likelihood);
            _request_time = clock();
        }

        MpiParticleRequest::~MpiParticleRequest() {
            if (!_process_stopped) { world.send(_rank, STOP_SIMULATION, true); }
            if (!_request.test()) { _request.wait(); }
        }

        void MpiParticleRequest::requestNewParticle(bool sample_prior) {
            if (!_request.test()) { _request.wait(); }

            if (sample_prior) {
                world.send(_rank, INSTRUCTION, SAMPLE_PRIOR);
            } else {
                world.send(_rank, INSTRUCTION, SAMPLE_CONSTR_PRIOR);
            }
            world.send(_rank, PARTICLE, _particle.data(), _particle.size());

            world.irecv(_rank, PARTICLE, _particle.data(), _particle.size());
            _request = world.irecv(_rank, LIKELIHOOD_RECOMPU, _log_likelihood);
            _request_time = clock();
        }

        double MpiParticleRequest::getLogLikelihood() { return _log_likelihood; }
        time_t MpiParticleRequest::getClocksForSampling() { return _log_likelihood; }

        std::vector<double> &MpiParticleRequest::getParticle() { return _particle; }
    }
}