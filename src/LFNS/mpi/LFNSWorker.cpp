//
// Created by jan on 22/10/18.
//

#include "LFNSWorker.h"

namespace lfns {
    namespace mpi {
        LFNSWorker::LFNSWorker(std::size_t my_rank, int num_parameters,
                               LogLikelihodEvalFct_ptr log_likelihood_evaluation) :
                _my_rank(my_rank), _num_parameters(num_parameters), _particle(new double[num_parameters]),
                _epsilon(-DBL_MAX), _sampler_size(1), _stopping_flag_request(new bmpi::request()),
                _mpi_stopping_criterion(std::make_shared<MPIStoppingCriterion>(_stopping_flag_request)),
                _log_likelihood_evaluation(log_likelihood_evaluation), _stop_iteration(false),
                _sampler(nullptr) {}

        LFNSWorker::~LFNSWorker() {
            delete[] _particle;
            delete _stopping_flag_request;
        }


        StoppingFct_ptr LFNSWorker::getStoppingFct() {
            return std::make_shared<StoppingFct>(
                    std::bind(&MPIStoppingCriterion::processStopped, _mpi_stopping_criterion.get()));
        }

        void LFNSWorker::run() {


            if (!_sampler.get()) {
                std::stringstream ss;
                ss << "Tried to run LFNS without initializing it:\n\t" << "LFNS_Sampler has not been set!" << std::endl;
                throw std::runtime_error(ss.str());
            }

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

                    case SAMPLE_CONSTR_PRIOR:
                        _sampleConstrPrior();
                        break;

                    case SAMPLE_PRIOR:
                        _samplePrior();
                        break;

                    case UPDATE_EPSILON:
                        _updateEpsilon();
                        break;

                    case UPDATE_SAMPLER:
                        _updateSampler();
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


        void LFNSWorker::_sampleConstrPrior() {
            time_t tic = clock();
            const std::vector<double> &parameter = _sampler->sampleConstrPrior();
            time_t toc = clock();
            time_t sampling_time = toc - tic;
            double log_likelihood = (*_log_likelihood_evaluation)(parameter);
            if (log_likelihood > _epsilon) {
                world.send(0, INSTRUCTION, PARTICLE_ACCEPTED);
                world.send(0, LIKELIHOOD_RECOMPU, log_likelihood);
                world.send(0, PARTICLE, parameter.data(), _num_parameters);
                world.send(0, CLOCKS_SAMPLING, sampling_time);
            } else {
                world.send(0, INSTRUCTION, PARTICLE_REJECTED);
            }
        }

        void LFNSWorker::_samplePrior() {
            time_t tic = clock();
            std::vector<double> parameter = _sampler->samplePrior();
            time_t toc = clock();
            double log_likelihood = (*_log_likelihood_evaluation)(parameter);
            world.send(0, INSTRUCTION, PARTICLE_ACCEPTED);
            world.send(0, LIKELIHOOD_RECOMPU, log_likelihood);
            world.send(0, PARTICLE, parameter.data(), _num_parameters);;
            world.send(0, CLOCKS_SAMPLING, toc - tic);
        }

        void LFNSWorker::_updateSampler() {
            world.recv(0, SAMPLER_SIZE, _sampler_size);
            char sampler_char_ptr[_sampler_size];
            world.recv(0, SAMPLER, sampler_char_ptr, _sampler_size);
            std::string sampler_str(sampler_char_ptr, sampler_char_ptr + _sampler_size);
            std::stringstream stream(sampler_str);
            _sampler->updateSerializedSampler(stream);
            _sampler->getDensityEstimation()->updateLogLikelihoodFct(_log_likelihood_evaluation);
        }

        void LFNSWorker::_prepareStoppingFlag() {
            _stop_iteration = false;
            *_stopping_flag_request = world.irecv(0, STOP_SIMULATION, _stop_iteration);
            _mpi_stopping_criterion->updateRequest(_stopping_flag_request);
        }


        void
        LFNSWorker::setSampler(sampler::Sampler_ptr prior, sampler::DensityEstimation_ptr density_estimation,
                               base::RngPtr rng) {
            _sampler = std::make_shared<LFNSSampler>(prior, density_estimation, rng);
        }

        void LFNSWorker::setLogParams(std::vector<int> log_params) { _sampler->setLogParams(log_params); }


        void LFNSWorker::_updateEpsilon() { world.recv(0, EPSILON, _epsilon); }
    }
}
