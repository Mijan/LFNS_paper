//
// Created by jan on 23/10/18.
//

#ifndef LFNS_MPIREQUEST_H
#define LFNS_MPIREQUEST_H

#include "MpiTags.h"
#include <ctime>

namespace lfns {
    namespace mpi {

        class MpiRequest {
        public:
            MpiRequest(std::size_t rank);

            virtual ~MpiRequest();

            virtual bool test();

            std::size_t getRank();

            bool processStopped();

            void interruptRequest();

            void prepareStopping();

            time_t getClocksSinceRequest();

        protected:
            std::size_t _rank;
            bmpi::request _request;

            clock_t _request_time;

            bmpi::communicator world;
            bool _process_stopped;
        };

        class MpiLikelihoodRequest : public MpiRequest {
        public:
            MpiLikelihoodRequest(std::size_t rank, const std::vector<double> &particle);

            ~MpiLikelihoodRequest();

            void requestNewLikelihood(const std::vector<double> &particle);

            double getLogLikelihood();

        private:
            double _log_likelihood;
        };

        typedef std::shared_ptr<MpiLikelihoodRequest> MpiLikelihoodRequest_ptr;

    }
}

#endif //LFNS_MPIREQUEST_H
