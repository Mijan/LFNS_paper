//
// Created by jan on 23/10/18.
//

#include "LFNSSeq.h"

namespace lfns {
    namespace seq {
        LFNSSeq::LFNSSeq(LFNSSettings &settings, LogLikelihodEvalFct_ptr log_likelihood_evaluation)
                : LFNS(settings), _log_likelihood_evaluation(log_likelihood_evaluation) {}

        void LFNSSeq::runLFNS() {
            std::stringstream os;
            if (!checkIfInitialized(os)) {
                std::stringstream ss;
                ss << "Tried to run LFNS without initializing it:\n\t" << os.str() << std::endl;
                throw std::runtime_error(ss.str());
            }

            bool lfns_terminate = false;

            int m = 0;

            if (!_resume_run) {
                _logger.lfnsStarted(m, *_epsilon_ptr);


                for (int i = 0; i < _settings.N; i++) {
                    time_t tic = clock();
                    std::vector<double> theta = _sampler->samplePrior();
                    time_t toc = clock();

                    _logger.thetaSampled(theta, toc - tic);
                    double l = (*_log_likelihood_evaluation)(theta);
                    _logger.likelihoodComputed(l);
                    _live_points.push_back(theta, l);
                    _logger.particleAccepted(theta, l);
                }
                _live_points.writeToFile(_settings.output_file, "live_points_0");
                _logger.logIterationStats();
            } else {
                m = _logger.iterationNumber();
                *_epsilon_ptr = _logger.lastEpsilon();
                _logger.lfnsResume(m, *_epsilon_ptr);
            }
            while (!lfns_terminate) {
                _sampler->updateAcceptanceRate(_logger.lastAcceptanceRate());
                m++;
                _logger.iterationStarted(m);

                for (int j = 0; j < _settings.r; j++) {
                    const LFNSParticle &particle = _live_points.removeLowestPartcile();
                    _dead_points.push_back(particle);
                    _logger.deadPointAdded(particle);
                }

                *_epsilon_ptr = _live_points.getLowestLikelihood();
                _logger.epsilonUpdated(*_epsilon_ptr);

                time_t tic = clock();
                _sampler->updateLiveSamples(_live_points);
                time_t toc = clock();


                _logger.samplerUpdated(*_sampler, toc - tic);

                while (_live_points.numberParticles() < _settings.N) {
                    tic = clock();
                    std::vector<double> theta = _sampler->sampleConstrPrior();
                    toc = clock();
                    _logger.thetaSampled(theta, toc - tic);
                    double l = (*_log_likelihood_evaluation)(theta);
                    _logger.likelihoodComputed(l);
                    if (l >= *_epsilon_ptr) {
                        _live_points.push_back(theta, l);
                        _logger.particleAccepted(theta, l);
                    }
                }

                lfns_terminate = _postIteration();
            }

            _logger.lfnsTerminated();
        }
    }
}