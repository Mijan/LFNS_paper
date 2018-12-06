//
// Created by jan on 18.01.17.
//

#include <iostream>
#include "LFNSOptions.h"
#include "../base/IoUtils.h"

namespace options {
    LFNSOptions::LFNSOptions()
            : DataOptions(), num_dpgmm_iterations(-1), N(-1), r(-1),
              rejection_quantile(-1), rejection_quantile_low_accept(-1), thresh_accept_rate(-1),
              LFNS_tolerance(-1),
              H(-1), _sampler_index(0), use_premature_cancelation(false),
              previous_population_file(""), print_interval(-1), verbose(true) {


        desc.add_options()("dpgmmiterations,d", po::value<int>(&num_dpgmm_iterations),
                           "The number of iterations used for the DPGMM density estimation")(
                "printinterval,p", po::value<int>(&print_interval),
                "The period at which the accepted particles will be printed")(
                "LFNSparticles,N", po::value<int>(&N), "The number of LFNS particles")(
                "smcparticles,H", po::value<int>(&H), "The number of particles for the particle filter")
                ("sampler,S", po::value<int>(&_sampler_index),
                 "Indicates whether to use the rejection sampler with DP-GMM estimation (0), Kernel Density Estimation (1) or Ellipsoid sampling (2).")(
                "prematurecancelling,c", po::value<bool>(&use_premature_cancelation),
                "Indicates whether the likelihood computation should be canceled if the current likelihood is already smaller than the current threshold. (This should be done when it is known that the likelhlihood for each timestepo cannot be higher than 1)")(
                "numberprallelsamples,r",
                po::value<int>(&r),
                "Number of LFNS particles to be sampled at once")(
                "rej_quan,q",
                po::value<double>(&rejection_quantile),
                "Quantile used for the rejection sampler")(
                "rej_quan_lo_accept,Q",
                po::value<double>(&rejection_quantile_low_accept),
                "Quantile used for the rejection sampler, if the previous acceptance rate is lower than the acceptance rate threshold")(
                "acc_thresh,A",
                po::value<double>(&thresh_accept_rate),
                "Acceptance threshold. If acceptance rate falls below this threshold the rejection quantile specified in -Q will be used")(
                "tolerance,t",
                po::value<double>(&LFNS_tolerance),
                "Threshold for termination of the algorithm")(
                "previous_pop,P",
                po::value<std::string>(&previous_population_file),
                "The file name of the previous population files. An according previous_pop_log_file.txt files needs to be provided")(
                "verbose,v",
                po::value<bool>(&verbose),
                "Indicates if ODE error messages should be printed out.");
    }

    LFNSOptions::~LFNSOptions() {}

    int LFNSOptions::handleCommandLineOptions(int argc, char **argv) {
        DataOptions::handleCommandLineOptions(argc, argv);

        if (rejection_quantile_low_accept > 0 || thresh_accept_rate > 0) {
            if (rejection_quantile_low_accept < 0 || thresh_accept_rate < 0) {
                std::cerr
                        << "If rejection quantile for low acceptance rate (-Q) is provided, so must be the threshold for a low accetance rate (-A). Both values will be ignored!"
                        << std::endl;
                rejection_quantile_low_accept = -1;
                thresh_accept_rate = -1;
            }
        }

        if (vm.count("previous_pop") > 0) {
            if (!base::IoUtils::isPathAbsolute(previous_population_file)) {
                fs::path full_path(fs::current_path());
                previous_population_file = full_path.string() + "/" + previous_population_file;
            }
            if (previous_population_file.find_last_of(".") == std::string::npos) {
                std::stringstream os;
                os << "The provided previous population file name " << previous_population_file
                   << " is not a file. Please provide a filename in the format '/path/to/previouspopfile.txt'!"
                   << std::endl;
                throw std::runtime_error(os.str());
            }

            bool prev_pop_file_valid = false;
            if (previous_population_file.find("log_file") == std::string::npos) {
                if (base::IoUtils::doesFileExists(previous_population_file)) {
                    std::ifstream fs(previous_population_file);
                    std::string first_string;
                    fs >> first_string;
                    fs.close();
                    if (first_string.compare("i") == 0) {
                        prev_pop_file_valid = true;
                    }
                }

                if (!prev_pop_file_valid) {
                    std::string new_prev_pop_file_name = base::IoUtils::appendToFileName(previous_population_file,
                                                                                         "log_file");
                    std::ifstream fs(new_prev_pop_file_name);
                    std::string first_string;
                    fs >> first_string;
                    fs.close();
                    if (first_string.compare("i") == 0) {
                        std::cerr << "File " << previous_population_file
                                  << " does not seem to be a previous log file. File name is being corrected to "
                                  << new_prev_pop_file_name << "." << std::endl;
                        previous_population_file = new_prev_pop_file_name;
                        prev_pop_file_valid = true;
                    }
                }
            } else {
                std::ifstream fs(previous_population_file);
                std::string first_string;
                fs >> first_string;
                fs.close();
                if (first_string.compare("i") == 0) { prev_pop_file_valid = true; }
            }
            if (!prev_pop_file_valid) {
                std::stringstream ss;
                ss << "File " << previous_population_file
                   << " does not seem to be a previous log file. LF-NS aborted!" << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        return 1;
    }

    bool LFNSOptions::useProperResamplingSet() { return vm.count("resampling") > 0; }

    bool LFNSOptions::samplerSet() { return vm.count("sampler") > 0; }

}