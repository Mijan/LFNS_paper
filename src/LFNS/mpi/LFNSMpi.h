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
//            LFNSMpi(LFNSSettings &lfns_settings, sampler::SamplerSettings &sampler_settings, base::RngPtr rng,
//                    int num_tasks);

            LFNSMpi(LFNSSettings &lfns_settings, int num_tasks);

            virtual ~LFNSMpi();

            void runLFNS() override;

        private:

            void _updateSampler(sampler::DensityEstimation_ptr density_estimation);

            void _requestLikelihood(std::vector<double> &theta);


            bmpi::communicator world;
            int _num_tasks;

            time_t time_1;
            time_t time_2;
            time_t time_3;
            time_t time_4;
            time_t time_5;

            void _samplePrior(lfns::mpi::RequestQueue &queue);

            void _sampleConstPrior(RequestQueue &queue);

            void _updateEpsilon(double epsilon);

            void _updateSampler();

            void _initializeQueue(RequestQueue &queue);
        };
    }
}


#endif //LFNS_LFNS_H
