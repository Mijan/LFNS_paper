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

            void addRequest(std::size_t rank, int num_parameters, bool sample_prior = false);

            double getFirstLikelihood();

            const std::vector<double> &getFirstTheta();

            bool firstParticleFinished();

            time_t getFirstParticleClocks();

            time_t getFirstSamplingClocks();

            int getFirstUsedProcess();

            std::queue<std::size_t>& getFinishedProcessess();

            void clearFirstParticle();

            void stopPendingRequests();

            std::size_t size() const;

//            time_t finished_proccesses_time;
//            time_t sending_requests_time;
        private:
            std::queue<std::vector<double> > computed_particles;
            std::queue<int> used_process;
            std::queue<time_t> clocks_for_particles;
            std::queue<time_t> clocks_for_sampling;
            std::queue<double> log_likelihoods;
            std::queue<bool> process_finished;


            std::vector<time_t *> ptr_clocks_for_particles;
            std::vector<time_t *> ptr_clocks_for_sampling;
            std::vector<double *> ptr_log_likelihoods;
            std::vector<bool *> ptr_process_finished;
            std::vector<std::vector<double> *> ptr_particles;

            std::vector<MpiParticleRequest_ptr> particle_requests;

            std::queue<std::size_t> _finished_request_queue;
        };
    }
}


#endif //LFNS_REQUESTQUEUE_H
