//
// Created by jan on 04/10/18.
//

#ifndef LFNS_LFNSSETTINGS_H
#define LFNS_LFNSSETTINGS_H

#include <vector>
#include <string>
#include <map>
#include <cmath>
#include "../models/InputPulse.h"
#include <iostream>

namespace lfns {

    enum DENSITY_ESTIMATOR {
        REJECT_DPGMM, KDE_GAUSS, KDE_UNIFORM, ELLIPS, SLICE
    };

    class LFNSSettings {
    public:
        int N = 1000;
        int r = 100;
        bool uniform_prior = true;
        std::string prior_file = "";
        std::string previous_log_file = "";
        DENSITY_ESTIMATOR estimator = REJECT_DPGMM;
        double log_termination = -4.6;
        int print_interval = 1;
        int acceptance_info_print_interval = 100;
        double rejection_quantile_for_density_estimation = 0.001;
        double thresh_accept_rate = -1;
        double rejection_quantile_low_accept = -1;
        std::string output_file = "";


        void print(std::ostream &stream) {
            stream << "N:\t" << N << std::endl;
            stream << "r:\t" << r << std::endl;
            stream << "Termination threshold:\t" << std::exp(log_termination) << std::endl;
            if (uniform_prior) {
                stream << "A uniform prior will be used." << std::endl;
            } else {
                stream << "A non-uniform prior will be used. It will be approximated from the samples in file "
                       << prior_file << std::endl;
            }
            stream << "To sample from the super-level sets of the likelihood a";
            switch (estimator) {
                case REJECT_DPGMM: {
                    stream
                            << " DP-GMM density estimation with rejection sampling will be used. The rejection constant is the "
                            << rejection_quantile_for_density_estimation * 100
                            << "% quantile" << std::endl;
                    if (thresh_accept_rate > 0) {
                        stream << "If the acceptance rate drops below " << thresh_accept_rate
                               << " the rejection constant will be set to the " << rejection_quantile_low_accept * 100
                               << "% quantile" << std::endl;
                    }
                    break;
                }
                case KDE_GAUSS: {
                    stream << " Kernel Density estimation with Gaussian kernels will be used.";
                    break;
                }
                case KDE_UNIFORM: {
                    stream << " Kernel Density estimation with uniform kernels will be used.";
                    break;
                }
                case ELLIPS: {
                    stream << "An ellipsoid sampler will be used.";
                    break;
                }
                case SLICE: {
                    stream << "A slice sampler will be used.";
                    break;
                }

            }
            if (previous_log_file.size() > 0) {
                stream << "\nA previous log file was provided:\t" << previous_log_file
                       << ", and LFNS will continue from the last iteration." << std::endl;
            }
        }
    };
};

#endif //LFNS_LFNSSETTINGS_H
