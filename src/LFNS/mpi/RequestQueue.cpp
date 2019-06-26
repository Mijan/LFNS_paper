//
// Created by jan on 23/10/18.
//

#include <algorithm>
#include <cfloat>
#include "RequestQueue.h"

namespace lfns {
    namespace mpi {
        RequestQueue::RequestQueue()
                : computed_particles(), used_process(), clocks_for_particles(), clocks_for_sampling(),
                  log_likelihoods(), process_finished(),
                  ptr_clocks_for_particles(), ptr_clocks_for_sampling(), ptr_log_likelihoods(), ptr_process_finished(),
                  particle_requests(), _finished_request_queue() {}

        RequestQueue::~RequestQueue() {}

        void RequestQueue::addRequest(std::size_t rank, int num_parameters, bool sample_prior) {
            clocks_for_particles.push(0);
            clocks_for_sampling.push(0);
            log_likelihoods.push(-DBL_MAX);
            process_finished.push(false);

            computed_particles.push(std::vector<double>(num_parameters));
            used_process.push(rank);
            if (rank > particle_requests.size()) {
                particle_requests.push_back(std::make_shared<MpiParticleRequest>(rank, sample_prior));
                ptr_process_finished.push_back(&process_finished.back());
                ptr_log_likelihoods.push_back(&log_likelihoods.back());
                ptr_clocks_for_particles.push_back(&clocks_for_particles.back());
                ptr_clocks_for_sampling.push_back(&clocks_for_sampling.back());
                ptr_particles.push_back(&computed_particles.back());
            } else {
                particle_requests[rank - 1]->requestNewParticle(sample_prior);
                ptr_process_finished[rank - 1] = &process_finished.back();
                ptr_log_likelihoods[rank - 1] = &log_likelihoods.back();
                ptr_clocks_for_particles[rank - 1] = &clocks_for_particles.back();
                ptr_clocks_for_sampling[rank - 1] = &clocks_for_sampling.back();
                ptr_particles[rank - 1] = &computed_particles.back();
            }
        }

        double RequestQueue::getFirstLikelihood() { return log_likelihoods.front(); }

        const std::vector<double> &RequestQueue::getFirstTheta() { return computed_particles.front(); }

        bool RequestQueue::firstParticleFinished() { return process_finished.front(); }

        time_t RequestQueue::getFirstParticleClocks() { return clocks_for_particles.front(); }

        time_t RequestQueue::getFirstSamplingClocks() { return clocks_for_sampling.front(); }

        int RequestQueue::getFirstUsedProcess() { return used_process.front(); }

        std::queue<std::size_t> &RequestQueue::getFinishedProcessess() {
            for (int i = 0; i < particle_requests.size(); i++) {
                MpiParticleRequest_ptr request = particle_requests[i];
                if (request->test()) {
                    request->receiveParticle(ptr_log_likelihoods[i], ptr_particles[i], ptr_clocks_for_sampling[i]);
                    *ptr_clocks_for_particles[i] = request->getClocksSinceRequest();
                    *ptr_process_finished[i] = true;
                    _finished_request_queue.push(request->getRank());
                }
            }
            return _finished_request_queue;
        }


        void RequestQueue::clearFirstParticle() {
            clocks_for_particles.pop();
            log_likelihoods.pop();
            process_finished.pop();
            computed_particles.pop();
            used_process.pop();
            clocks_for_sampling.pop();
        }

        void RequestQueue::stopPendingRequests() {
            for (MpiParticleRequest_ptr request : particle_requests) { request->interruptRequest(); }
        }

        std::size_t RequestQueue::size() const { return log_likelihoods.size(); }
    }
}