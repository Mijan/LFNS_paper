//
// Created by jan on 23/10/18.
//

#include <algorithm>
#include <cfloat>
#include "RequestQueue.h"

namespace lfns {
    namespace mpi {
        RequestQueue::RequestQueue()
                : computed_particles(), used_process(), clocks_for_particles(), log_likelihoods(), process_finished(),
                  ptr_seconds_for_particles(), ptr_log_likelihoods(), ptr_process_finished(), likelihood_requests() {}

        RequestQueue::~RequestQueue() {}

        void RequestQueue::addRequest(int rank, const std::vector<double> &theta) {
            clocks_for_particles.push(0);
            log_likelihoods.push(-DBL_MAX);
            process_finished.push(false);

            computed_particles.push(theta);
            used_process.push(rank);
            if (rank > likelihood_requests.size()) {
                likelihood_requests.push_back(std::make_shared<MpiLikelihoodRequest>(rank, theta));
                ptr_process_finished.push_back(&process_finished.back());
                ptr_log_likelihoods.push_back(&log_likelihoods.back());
                ptr_seconds_for_particles.push_back(&clocks_for_particles.back());
            } else {
                likelihood_requests[rank - 1]->requestNewLikelihood(theta);
                ptr_process_finished[rank - 1] = &process_finished.back();
                ptr_log_likelihoods[rank - 1] = &log_likelihoods.back();
                ptr_seconds_for_particles[rank - 1] = &clocks_for_particles.back();
            }

        }

        double RequestQueue::getFirstLikelihood() { return log_likelihoods.front(); }

        std::vector<double> RequestQueue::getFirstTheta() { return computed_particles.front(); }

        bool RequestQueue::firstParticleFinished() { return process_finished.front(); }

        double RequestQueue::getFirstParticleClocks() { return clocks_for_particles.front(); }

        int RequestQueue::getFirstUsedProcess() { return used_process.front(); }

        int RequestQueue::getFinishedProcess() {
            for (int i = 0; i < likelihood_requests.size(); i++) {
                MpiLikelihoodRequest_ptr request = likelihood_requests[i];
                if (request->test()) {
                    *ptr_seconds_for_particles[i] = request->getClocksSinceRequest();
                    *ptr_log_likelihoods[i] = request->getLogLikelihood();
                    *ptr_process_finished[i] = true;
                    return request->getRank();
                }
            }
            return 0;
        }


        void RequestQueue::clearFirstParticle() {
            clocks_for_particles.pop();
            log_likelihoods.pop();
            process_finished.pop();
            computed_particles.pop();
            used_process.pop();
        }

        void RequestQueue::stopPendingRequests() {
            for (MpiLikelihoodRequest_ptr request : likelihood_requests) { request->interruptRequest(); }
        }

        std::size_t RequestQueue::size() const{
            return log_likelihoods.size();
        }
    }
}