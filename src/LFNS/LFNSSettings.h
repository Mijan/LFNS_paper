//
// Created by jan on 04/10/18.
//

#ifndef LFNS_LFNSSETTINGS_H
#define LFNS_LFNSSETTINGS_H

#include <vector>
#include <string>
#include <map>
#include <cmath>
#include "../simulator/SimulationSettings.h"
#include "../models/InputPulse.h"

namespace lfns {

    enum DENSITY_ESTIMATOR {
        REJECT_DPGMM, KDE_GAUSS, KDE_UNIFORM, ELLIPS
    };

    class LFNSSettings {
    public:
        int N = 1000;
        int r = 100;
        bool uniform_prior = true;
        std::vector<std::string> experiments_for_LFNS;
        std::string previous_log_file = "";
        DENSITY_ESTIMATOR estimator = REJECT_DPGMM;
        double log_termination = -4.6;
        int print_interval = 1;
        int acceptance_info_print_interval = 100;
        double rejection_quantile_for_density_estimation = 0.001;
        std::string output_file = "";


        void print(std::ostream &stream) {
            stream << "N:\t" << N << std::endl;
            stream << "r:\t" << r << std::endl;
            stream << "Termination threshold:\t" << std::exp(log_termination) << std::endl;
            stream << std::endl;
            if (previous_log_file.size() > 0) {
                stream << "A previous log file was provided:\t" << previous_log_file
                       << ", and LFNS will continue from the last iteration." << std::endl;
            }
        }
    };
};

#endif //LFNS_LFNSSETTINGS_H
