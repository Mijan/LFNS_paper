//
// Created by jan on 22/10/18.
//

#include "LFNSWorker.h"

namespace lfns {
    namespace mpi {
        LFNSWorker::LFNSWorker(std::size_t my_rank, int num_parameters,
                               LogLikelihodEvalFct_ptr log_likelihood_evaluation) :
                _my_rank(my_rank), _num_parameters(num_parameters), _particle(new double[num_parameters]),
                _epsilon(-DBL_MAX), _stopping_flag_request(new bmpi::request()),
                _mpi_stopping_criterion(std::make_shared<MPIStoppingCriterion>(_stopping_flag_request)),
                _log_likelihood_evaluation(log_likelihood_evaluation), _stop_iteration(false) {}

        LFNSWorker::~LFNSWorker() {
            delete[] _particle;
            delete _stopping_flag_request;
        }


        StoppingFct_ptr LFNSWorker::getStoppingFct() {
            return std::make_shared<StoppingFct>(
                    std::bind(&MPIStoppingCriterion::processStopped, _mpi_stopping_criterion.get()));
        }

        void LFNSWorker::run() {

            MPI_INSTRUCTION instruction(INSTRUCTION);

            bool process_terminated = false;
            while (!process_terminated) {
                world.recv(0, INSTRUCTION, instruction);
                switch (instruction) {

                    case DIETAG:
                        std::cout << "This is process " << _my_rank << " , I am exiting." << std::endl;
                        process_terminated = true;
                        break;

                    case LIKELIHOOD_RECOMPU:
                        _computeLikelihood();
                        break;

                    case UPDATE_EPSILON:
                        _updateEpsilon();
                        break;

                    case PREPARE_STOPPING:
                        _prepareStoppingFlag();
                        break;

                    default:
                        break;
                }
            }

            return;
        }


        double *LFNSWorker::getEpsilonPtr() { return &_epsilon; }

        void LFNSWorker::_computeLikelihood() {
            world.recv(0, PARTICLE, _particle, _num_parameters);
            std::vector<double> parameter(_particle, _particle + _num_parameters);
            double log_likelihood = (*_log_likelihood_evaluation)(parameter);
            world.send(0, LIKELIHOOD_RECOMPU, log_likelihood);
        }

        void LFNSWorker::_prepareStoppingFlag() {
            _stop_iteration = false;
            *_stopping_flag_request = world.irecv(0, STOP_SIMULATION, _stop_iteration);
            _mpi_stopping_criterion->updateRequest(_stopping_flag_request);
        }


        void LFNSWorker::_updateEpsilon() { world.recv(0, EPSILON, _epsilon); }
    }
}
