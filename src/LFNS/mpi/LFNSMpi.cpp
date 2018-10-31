//
// Created by jan on 05/10/18.
//

#include <iostream>
#include "LFNSMpi.h"
#include "RequestQueue.h"
#include "MpiRequest.h"

namespace lfns {
    namespace mpi {
        LFNSMpi::LFNSMpi(LFNSSettings &lfns_settings, sampler::SamplerSettings &sampler_settings, base::RngPtr rng,
                         int num_tasks) : LFNS(lfns_settings, sampler_settings, rng), _num_tasks(num_tasks) {}

        LFNSMpi::~LFNSMpi() {}

        void LFNSMpi::runLFNS() {

            bool lfns_terminate = false;

            int m = 0;

            RequestQueue queue;
            _initializeQueue(queue);
            if (!_resume_run) {
                _logger.lfnsStarted(m, _epsilon);
                _samplePrior(queue);
                _live_points.writeToFile(_settings.output_file, "live_points_0");
            } else {
                m = _logger.iterationNumber();
                _epsilon = _logger.lastEpsilon();
                _logger.lfnsResume(m, _epsilon);
            }
            while (!lfns_terminate) {
                m++;
                _logger.iterationStarted(m);

                _sampleConstPrior(queue);

                lfns_terminate = _postIteration();
            }
            _logger.lfnsTerminated();
            for (int i = 0; i < _num_tasks; i++) { world.send(i, INSTRUCTION, DIETAG); }
        }

        void LFNSMpi::_samplePrior(RequestQueue &queue) {
            while (_live_points.numberParticles() < _settings.N) {
                std::queue<std::size_t> finished_tasks = queue.getFinishedProcessess();
                while (!finished_tasks.empty()) {
                    std::vector<double> theta = _sampler.samplePrior();
//                    std::cout << "Master sends request to " << finished_task << std::endl;
                    queue.addRequest(finished_tasks.front(), theta);
                    finished_tasks.pop();
                }


                if (queue.firstParticleFinished()) {
                    double l = queue.getFirstLikelihood();
                    const std::vector<double> &theta = queue.getFirstTheta();
                    _logger.thetaSampled(theta);
                    _logger.likelihoodComputed(l);
                    _live_points.push_back(theta, l);
                    _logger.particleAccepted(theta, l, queue.getFirstParticleClocks(), queue.getFirstUsedProcess());
                    queue.clearFirstParticle();
                }
            }
            queue.stopPendingRequests();
        }

        void LFNSMpi::_initializeQueue(RequestQueue &queue) {
            for (std::size_t rank = 1; rank < _num_tasks; rank++) {
                std::vector<double> theta = _sampler.samplePrior();
//                std::cout << "Master sends initializer to rank " << rank << std::endl;
                queue.addRequest(rank, theta);
            }
        }

        void LFNSMpi::_sampleConstPrior(RequestQueue &queue) {
            for (int j = 0; j < _settings.r; j++) {
                const LFNSParticle &particle = _live_points.removeLowestPartcile();
                _dead_points.push_back(particle);
                _logger.deadPointAdded(particle);
            }

            _epsilon = _live_points.getLowestLikelihood();
            _updateEpsilon(_epsilon);
            _logger.epsilonUpdated(_epsilon);
            _sampler.updateLiveSamples(_live_points);
            _logger.samplerUpdated(_sampler);

            while (_live_points.numberParticles() < _settings.N) {
                std::queue<std::size_t> finished_tasks = queue.getFinishedProcessess();
                while (!finished_tasks.empty()) {
                    std::vector<double> theta = _sampler.sampleConstrPrior();
//                    std::cout << "Master sends request to " << finished_task << std::endl;
                    queue.addRequest(finished_tasks.front(), theta);
                    finished_tasks.pop();
                }

                while (queue.firstParticleFinished() && _live_points.numberParticles() < _settings.N) {
                    double l = queue.getFirstLikelihood();
                    const std::vector<double> &theta = queue.getFirstTheta();
                    _logger.thetaSampled(theta);
                    _logger.likelihoodComputed(l);
                    if (l >= _epsilon) {
                        _live_points.push_back(theta, l);
                        _logger.particleAccepted(theta, l, queue.getFirstParticleClocks(), queue.getFirstUsedProcess());
                    }
                    queue.clearFirstParticle();
                }
            }
            queue.stopPendingRequests();
        }


        void LFNSMpi::_updateEpsilon(double epsilon) {
            for (int rank = 1; rank < _num_tasks; rank++) {
                world.send(rank, INSTRUCTION, UPDATE_EPSILON);
                world.send(rank, EPSILON, epsilon);
            }
        }
    }
}