//
// Created by jan on 05/10/18.
//

#include <iostream>
#include "LFNSMpi.h"
#include "RequestQueue.h"
#include "MpiRequest.h"
#include "../../sampler/DpGmmSampler.h"
#include "../../sampler/EllipsoidSampler.h"
#include "../../sampler/GaussianSampler.h"
#include "../../sampler/KernelDensityEstimation.h"
#include "../../sampler/KernelSupportEstimation.h"
#include "../../sampler/RejectionSupportSampler.h"
#include "../../sampler/UniformSampler.h"
#include "../../base/IoUtils.h"


#include <boost/serialization/export.hpp>

BOOST_SERIALIZATION_ASSUME_ABSTRACT(sampler::Sampler);
BOOST_SERIALIZATION_ASSUME_ABSTRACT(sampler::KernelSampler);

BOOST_CLASS_EXPORT_GUID(sampler::Sampler, "Sampler");
BOOST_CLASS_EXPORT_GUID(sampler::KernelSampler, "KernelSampler");
BOOST_CLASS_EXPORT_GUID(sampler::DensityEstimation, "DensityEstimation");
BOOST_CLASS_EXPORT_GUID(sampler::DpGmmSampler, "DpGmmSampler");
BOOST_CLASS_EXPORT_GUID(sampler::EllipsoidSampler, "EllipsoidSampler");
BOOST_CLASS_EXPORT_GUID(sampler::KernelDensityEstimation, "KernelDensityEstimation");
BOOST_CLASS_EXPORT_GUID(sampler::KernelSupportEstimation, "KernelSupportEstimation");
BOOST_CLASS_EXPORT_GUID(sampler::RejectionSupportSampler, "RejectionSupportSampler");
BOOST_CLASS_EXPORT_GUID(sampler::UniformSampler, "UniformSampler");
BOOST_CLASS_EXPORT_GUID(sampler::GaussianSampler, "GaussianSampler");

namespace lfns {
    namespace mpi {

        LFNSMpi::LFNSMpi(LFNSSettings &lfns_settings, int num_tasks)
                : LFNS(lfns_settings), _num_tasks(num_tasks),
                  time_1(0), time_2(0), time_3(0), time_4(0), time_5(0) {}

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
                std::queue<std::size_t> &finished_tasks = queue.getFinishedProcessess();
                while (!finished_tasks.empty()) {
                    queue.addRequest(finished_tasks.front(), _num_parameters, true);
                    finished_tasks.pop();
                }

                if (queue.firstParticleFinished()) {
                    double l = queue.getFirstLikelihood();
                    const std::vector<double> &theta = queue.getFirstTheta();
                    _logger.thetaSampled(theta, queue.getFirstSamplingClocks());
                    _logger.likelihoodComputed(l);
                    _live_points.push_back(theta, l);
                    _logger.particleAccepted(theta, l, queue.getFirstParticleClocks(), queue.getFirstUsedProcess());
                    queue.clearFirstParticle();
                }
            }
            _logger.logIterationStats();
            queue.stopPendingRequests();
        }

        void LFNSMpi::_initializeQueue(RequestQueue &queue) {
            for (std::size_t rank = 1; rank < _num_tasks; rank++) { queue.addRequest(rank, _num_parameters, true); }
            queue.stopPendingRequests();
        }

        void LFNSMpi::_sampleConstPrior(RequestQueue &queue) {
            _sampler->updateAcceptanceRate(_logger.lastAcceptanceRate());
            for (int j = 0; j < _settings.r; j++) {
                const LFNSParticle &particle = _live_points.removeLowestPartcile();
                _dead_points.push_back(particle);
                _logger.deadPointAdded(particle);
            }

            _epsilon = _live_points.getLowestLikelihood();
            _updateEpsilon(_epsilon);
            _logger.epsilonUpdated(_epsilon);

            time_t tic = clock();
            std::cout << "number of live samples: " << _live_points.numberParticles() << std::endl;
            _sampler->updateLiveSamples(_live_points);
            time_t toc = clock();
            _logger.samplerUpdated(*_sampler, toc - tic);
            _updateSampler();

            while (_live_points.numberParticles() < _settings.N) {
                std::queue<std::size_t> &finished_tasks = queue.getFinishedProcessess();

                while (!finished_tasks.empty()) {
                    queue.addRequest(finished_tasks.front(), _num_parameters);
                    finished_tasks.pop();
                }
                while (queue.firstParticleFinished() && _live_points.numberParticles() < _settings.N) {
                    double l = queue.getFirstLikelihood();
                    const std::vector<double> &theta = queue.getFirstTheta();
                    _logger.thetaSampled(theta, queue.getFirstSamplingClocks());
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


        void LFNSMpi::_updateSampler() {
            for (int rank = 1; rank < _num_tasks; rank++) {
                world.send(rank, INSTRUCTION, UPDATE_SAMPLER);

                std::stringstream stream;
                _sampler->getSerializedSampler(stream);

                std::size_t sampler_size = stream.str().size();
                world.send(rank, SAMPLER_SIZE, sampler_size);

                std::string sampler_string = stream.str();
                world.send(rank, SAMPLER, sampler_string.c_str(), sampler_size);
            }
        }
    }
}