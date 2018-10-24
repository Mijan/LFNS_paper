//
// Created by jan on 05/10/18.
//

#include <iostream>
#include "LFNSMpi.h"
#include "RequestQueue.h"
#include "MpiRequest.h"

namespace lfns {
    namespace mpi {
        LFNSMpi::LFNSMpi(LFNSSettings settings, base::RngPtr rng, int num_tasks) : LFNS(settings, rng),
                                                                                   _num_tasks(num_tasks) {}

        LFNSMpi::~LFNSMpi() {}

        void LFNSMpi::runLFNS() {

            bool lfns_terminate = false;

            int m = 0;

            RequestQueue queue;
            if (!_resume_run) {
                _logger.lfnsStarted(m, _epsilon);
                _samplePrior(queue);
            } else {
                m = _logger.iterationNumber();
                _epsilon = _logger.lastEpsilon();
                _logger.lfnsResume(m, _epsilon);
            }
            while (!lfns_terminate) {
                m++;
                _logger.iterationStarted(m);

                sampleConstPrior(queue);

                lfns_terminate = _postIteration();
            }
            _logger.lfnsTerminated();
            for (int i = 0; i < _num_tasks; i++) { world.send(i, INSTRUCTION, DIETAG); }
        }

        void LFNSMpi::_samplePrior(RequestQueue &queue) {
            for (int rank = 1; rank < _num_tasks; rank++) {
                std::vector<double> theta = _sampler.samplePrior();
                queue.addRequest(rank, theta);
            }
            while (_live_points.numberParticles() < _settings.N) {
                int finished_task = queue.getFinishedProcess();
                if (finished_task) {
                    std::vector<double> theta = _sampler.samplePrior();
                    queue.addRequest(finished_task, theta);
                }

                if (queue.firstParticleFinished()) {
                    double l = queue.getFirstLikelihood();
                    const std::vector<double> &theta = queue.getFirstTheta();
                    _logger.thetaSampled(theta);
                    _logger.likelihoodComputed(l);
                    _live_points.push_back(theta, l);
                    _logger.particleAccepted(theta, l, queue.getFirstParticleClocks());
                    queue.clearFirstParticle();
                }
            }
            queue.stopPendingRequests();
        }

        void LFNSMpi::sampleConstPrior(RequestQueue &queue) {
            for (int j = 0; j < _settings.r; j++) {
                const LFNSParticle &particle = _live_points.removeLowestPartcile();
                _dead_points.push_back(particle);
                _logger.deadPointAdded(particle);
            }

            double epsilon = _live_points.getLowestLikelihood();
            _logger.epsilonUpdated(epsilon);
            _sampler.updateLiveSamples(_live_points);
            _logger.samplerUpdated(_sampler);

            while (_live_points.numberParticles() < _settings.N) {
                int finished_task = queue.getFinishedProcess();
                if (finished_task) {
                    std::vector<double> theta = _sampler.sampleConstrPrior();
                    queue.addRequest(finished_task, theta);
                }

                if (queue.firstParticleFinished()) {
                    double l = queue.getFirstLikelihood();
                    const std::vector<double> &theta = queue.getFirstTheta();
                    _logger.thetaSampled(theta);
                    _logger.likelihoodComputed(l);
                    if (l >= epsilon) {
                        _live_points.push_back(theta, l);
                        _logger.particleAccepted(theta, l, queue.getFirstParticleClocks());
                    }
                    queue.clearFirstParticle();
                }
            }
            queue.stopPendingRequests();
        }
    }
}