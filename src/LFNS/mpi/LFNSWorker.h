//
// Created by jan on 22/10/18.
//

#ifndef LFNS_LFNSWORKER_H
#define LFNS_LFNSWORKER_H

#include "../LFNS.h"
#include "MpiTags.h"

namespace lfns {
    namespace mpi {
        typedef std::function<bool()> StoppingFct;
        typedef std::shared_ptr<StoppingFct> StoppingFct_ptr;

        class MPIStoppingCriterion {
        public:
            MPIStoppingCriterion(boost::mpi::request *request, double checking_interval = 5.0) : _request(request),
                                                                                                 _process_stopped(
                                                                                                         false), _tic(
                            clock()), _checking_interval(checking_interval) {}

            virtual ~MPIStoppingCriterion() {}


            virtual bool processStopped() {
                time_t toc = clock();
                double passed_sec = ((double) (toc - _tic) / CLOCKS_PER_SEC);
                if (passed_sec >= _checking_interval) {
                    _process_stopped = (bool) _request->test();
                    _tic = toc;
                }
                return _process_stopped;
            }

            virtual void updateRequest(boost::mpi::request *request) {
                _request = request;
                _process_stopped = (bool) _request->test();
            }

        private:
            boost::mpi::request *_request;
            bool _process_stopped;
            time_t _tic;
            double _checking_interval;
        };

        typedef std::shared_ptr<MPIStoppingCriterion> MPIStoppingCriterion_ptr;


        class LFNSWorker {

        public:
            LFNSWorker(std::size_t my_rank, int num_parameters, LogLikelihodEvalFct_ptr log_likelihood_evaluation);

            virtual ~LFNSWorker();

            StoppingFct_ptr getStoppingFct();

            virtual void run();

            double *getEpsilonPtr();

            void
            setSampler(sampler::Sampler_ptr prior, sampler::DensityEstimation_ptr density_estimation, base::RngPtr rng);

            void setLogParams(std::vector<int> log_params);

        protected:
            const std::size_t _my_rank;
            const std::size_t _num_parameters;

            double *_particle;
            double _epsilon;
            std::size_t _sampler_size;
            bmpi::request *_stopping_flag_request;
            MPIStoppingCriterion_ptr _mpi_stopping_criterion;

            LogLikelihodEvalFct_ptr _log_likelihood_evaluation;
            bool _stop_iteration;

            bmpi::communicator world;

            LFNSSampler_ptr _sampler;

            void _computeLikelihood();

            void _sampleConstrPrior();

            void _samplePrior();

            void _updateSampler();

            void _prepareStoppingFlag();

            void _updateEpsilon();
        };
    }
}


#endif //LFNS_LFNSWORKER_H
