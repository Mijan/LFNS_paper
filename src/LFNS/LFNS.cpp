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
    LFNS::LFNS(LFNSSettings &settings, base::RngPtr rng)
            : _settings(settings), _live_points(), _dead_points(), _post_estimator(settings.N, settings.r),
              _sampler(settings, rng), _logger(settings), _resume_run(false), _epsilon(-DBL_MAX) {}

    LFNS::~LFNS() {}

    void LFNS::resumeRum(std::string previous_log_file) {
        _logger.readFromFile(previous_log_file);
        int it_nbr = _logger.iterationNumber();

        size_t pos = previous_log_file.find("_log_file.txt");
        std::string file_start;
        file_start.assign(previous_log_file.begin(), previous_log_file.begin() + pos);

        std::string dead_points_file = file_start + "_dead_points.txt";
        _dead_points.readFromFile(dead_points_file);

        std::stringstream ss;
        ss << file_start << "_live_points_" << it_nbr << ".txt";
        std::string live_points_file = ss.str();
        _live_points.readFromFile(live_points_file);
        _resume_run = true;

    }

    double *LFNS::getPointerToThreshold() { return &_epsilon; }

    bool LFNS::_postIteration() {
        PosteriorQuantitites post_quant = _post_estimator.estimatePosteriorQuantities(_live_points, _dead_points);
        _logger.logIterationResults(post_quant);
        _post_estimator.writeToFile(_settings.output_file, _live_points, _dead_points, post_quant);
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