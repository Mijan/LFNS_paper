//
// Created by jan on 05/10/18.
//

#include <iostream>
#include "LFNS.h"
#include "LiveParticleSet.h"
#include "DeadParticleSet.h"
#include "LFNSParticle.h"
#include "../base/IoUtils.h"
#include "../base/MathUtils.h"

namespace lfns {
    LFNS::LFNS(LFNSSettings &lfns_settings)
            : _settings(lfns_settings), _live_points(), _dead_points(),
              _post_estimator(std::make_shared<PosteriorEstimator>(lfns_settings.N, lfns_settings.r)),
              _sampler(nullptr),
              _logger(lfns_settings), _resume_run(false), _epsilon(-DBL_MAX), _epsilon_ptr(&_epsilon),
              _num_parameters(-1) {}

    LFNS::~LFNS() {}

    void LFNS::resumeRum(std::string previous_log_file) {
        try {
            _logger.readFromFile(previous_log_file);
        } catch (const std::runtime_error &e) {
            std::stringstream ss;
            ss << "Failed to read previous population from file " << previous_log_file << ":\n\t";
            ss << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
        int it_nbr = _logger.iterationNumber();

        size_t pos = previous_log_file.find("_log_file.txt");
        std::string file_start;
        file_start.assign(previous_log_file.begin(), previous_log_file.begin() + pos);

        std::string dead_points_file = file_start + "_dead_points.txt";

        std::stringstream ss;
        ss << file_start << "_live_points_" << it_nbr << ".txt";
        std::string live_points_file = ss.str();

        if (base::IoUtils::doesFileExists(dead_points_file) && base::IoUtils::doesFileExists(live_points_file)) {
            _dead_points.readFromFile(dead_points_file);
            _live_points.readFromFile(live_points_file);
        } else {
            std::string posterior_file_name = file_start + "_posterior.txt";
            std::string posterior_log_like_file_name = file_start + "_posterior_log_likelihoods.txt";

            if (base::IoUtils::doesFileExists(posterior_file_name) &&
                base::IoUtils::doesFileExists(posterior_log_like_file_name)) {
                _live_points.readFromFile(posterior_file_name);
                if (_live_points.numberParticles() == it_nbr * _settings.r + _settings.N) {
                    for (int i = 0; i < it_nbr * _settings.r; i++) {
                        const LFNSParticle &particle = _live_points.removeLowestPartcile();
                        _dead_points.push_back(particle);
                    }
                } else {
                    std::cerr << "Previous posterior files found, but number of particles does not match!";
                    std::cerr << " log file indicates that the last iteration was iteration " << it_nbr << " and N="
                              << _settings.N << ", and r = " << _settings.r;
                    std::cerr << ". Thus posterior is expected to have " << it_nbr * _settings.r << " + " << _settings.N
                              << "=" << it_nbr * _settings.r + _settings.N << " particles, but posterior has "
                              << _live_points.numberParticles() << " particles." << std::endl;
                    _live_points = LiveParticleSet();
                }
            } else {
                std::cerr << "Neither dead points and live points files " << dead_points_file << " and "
                          << live_points_file << " or posterior file " << posterior_file_name << " provided."
                          << std::endl;
            }
        }
        if (_dead_points.size() == 0 && _live_points.numberParticles() == 0) {
            std::cerr << "Faile to read previous population, LFNS algorithm will start from scratch!";
            _resume_run = false;
            _logger = LFNSLogger(_settings);
            return;
        }

        _resume_run = true;

    }

    double *LFNS::getPointerToThreshold() { return _epsilon_ptr; }

    bool LFNS::checkIfInitialized(std::ostream &os) {
        if (!_sampler.get()) {
            os << "LFNS_Sampler has not been set!" << std::endl;
            return false;
        }
        return true;
    }

    void
    LFNS::setSampler(sampler::Sampler_ptr prior, sampler::DensityEstimation_ptr density_estimation, base::RngPtr rng) {
        _sampler = std::make_shared<LFNSSampler>(prior, density_estimation, rng);
        _num_parameters = prior->getSamplerDimension();
    }

    void LFNS::setLogParams(std::vector<int> log_params) { _sampler->setLogParams(log_params); }

    void LFNS::setThresholdPointer(double *epsilon_ptr) { _epsilon_ptr = epsilon_ptr; }

    bool LFNS::_postIteration() {
        _logger.logIterationStats();
        PosteriorQuantitites post_quant = _post_estimator->estimatePosteriorQuantities(_live_points, _dead_points);
        _logger.logIterationResults(post_quant);
        _post_estimator->writeToFile(_settings.output_file, _live_points, _dead_points, post_quant);
        std::stringstream ss;
        ss << "live_points_" << _logger.iterationNumber();
        _live_points.writeToFile(_settings.output_file, ss.str());
        _dead_points.writeToFile(_settings.output_file, "dead_points");
        _logger.writeToFile();
        bool lfns_terminate = _testTermination(post_quant);
        return lfns_terminate;
    }

    bool LFNS::_testTermination(PosteriorQuantitites &post_quant) {
        return post_quant.log_max_std_improvement < _settings.log_termination;
    }
}