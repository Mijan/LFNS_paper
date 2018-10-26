//
// Created by jan on 23/10/18.
//

#include "LFNSSeq.h"

namespace lfns {
    namespace seq {

        LFNSSeq::LFNSSeq(LFNSSettings settings, base::RngPtr rng, LogLikelihodEvalFct_ptr log_likelihood_evaluation)
                : LFNS(settings, rng),
                  _log_likelihood_evaluation(
                          std::move(
                                  log_likelihood_evaluation)) {}

        void LFNSSeq::runLFNS() {

            bool lfns_terminate = false;

            int m = 0;

            if (!_resume_run) {
                _logger.lfnsStarted(m, _epsilon);

                for (int i = 0; i < _settings.N; i++) {
                    std::vector<double> theta = _sampler.samplePrior();
                    _logger.thetaSampled(theta);
                    double l = (*_log_likelihood_evaluation)(theta);
                    _logger.likelihoodComputed(l);
                    _live_points.push_back(theta, l);
                    _logger.particleAccepted(theta, l);
                }
                _live_points.writeToFile(_settings.output_file, "live_points_0");
            } else {
                m = _logger.iterationNumber();
                _epsilon = _logger.lastEpsilon();
                _logger.lfnsResume(m, _epsilon);
            }
            while (!lfns_terminate) {
                m++;
                _logger.iterationStarted(m);

                for (int j = 0; j < _settings.r; j++) {
                    const LFNSParticle &particle = _live_points.removeLowestPartcile();
                    _dead_points.push_back(particle);
                    _logger.deadPointAdded(particle);
                }

                _epsilon = _live_points.getLowestLikelihood();
                _logger.epsilonUpdated(_epsilon);

                _sampler.updateLiveSamples(_live_points);
                _logger.samplerUpdated(_sampler);

                while (_live_points.numberParticles() < _settings.N) {
                    std::vector<double> theta = _sampler.sampleConstrPrior();
                    _logger.thetaSampled(theta);
                    double l = (*_log_likelihood_evaluation)(theta);
                    _logger.likelihoodComputed(l);
                    if (l >= _epsilon) {
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