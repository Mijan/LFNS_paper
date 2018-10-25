//
// Created by jan on 05/10/18.
//

#ifndef LFNS_LFNSMPI_H
#define LFNS_LFNSMPI_H

#include "../LFNS.h"
#include "MpiTags.h"
#include "RequestQueue.h"


namespace lfns {
    namespace mpi {

        class LFNSMpi : public lfns::LFNS {

        public:
            LFNSMpi(LFNSSettings settings, base::RngPtr rng, int num_tasks);

            virtual ~LFNSMpi();

            void runLFNS() override;

        private:

            void _updateSampler(sampler::DensityEstimation_ptr density_estimation);

            void _requestLikelihood(std::vector<double> &theta);


            bmpi::communicator world;
            int _num_tasks;

            void _samplePrior(lfns::mpi::RequestQueue &queue);

            void _sampleConstPrior(RequestQueue &queue);

            void _updateEpsilon(double epsilon);

            void _initializeQueue(RequestQueue &queue);
        };
    }
}


#endif //LFNS_LFNS_H
