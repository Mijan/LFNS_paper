//
// Created by jan on 23/10/18.
//

#ifndef LFNS_REQUESTQUEUE_H
#define LFNS_REQUESTQUEUE_H

#include "../LFNSParticle.h"
#include "MpiRequest.h"
#include <deque>
#include <queue>

namespace lfns {
    namespace mpi {
        class RequestQueue {
        public:
            RequestQueue();

            virtual ~RequestQueue();

            void addRequest(std::size_t rank, const std::vector<double> &theta);

            double getFirstLikelihood();

            std::vector<double> getFirstTheta();

            bool firstParticleFinished();

            double getFirstParticleClocks();

            int getFirstUsedProcess();

            std::queue<std::size_t> getFinishedProcessess();

            void clearFirstParticle();

            void stopPendingRequests();

            std::size_t size() const;
        private:
            std::queue<std::vector<double> > computed_particles;
            std::queue<int> used_process;
            std::queue<double> clocks_for_particles;
            std::queue<double> log_likelihoods;
            std::queue<bool> process_finished;


            std::vector<double *> ptr_clocks_for_particles;
            std::vector<double *> ptr_log_likelihoods;
            std::vector<bool *> ptr_process_finished;

            std::vector<MpiLikelihoodRequest_ptr> likelihood_requests;
        };
    }
}


#endif //LFNS_REQUESTQUEUE_H
