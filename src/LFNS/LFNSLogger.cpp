//
// Created by jan on 05/10/18.
//

#include <iostream>
#include <iomanip>
#include "LFNSLogger.h"
#include "../base/Utils.h"
#include "../base/MathUtils.h"
#include "../base/IoUtils.h"
#include "LFNSSampler.h"

namespace lfns {

    LFNSLogger::LFNSLogger(LFNSSettings settings) : _remaining_required_particles_iteration(settings.N),
                                                    _num_samples_iteration(0), _num_samples_particle(0),
                                                    _num_accepted_iteration(0),
                                                    _print_interval(settings.print_interval),
                                                    _acceptance_info_print_interval(
                                                            settings.acceptance_info_print_interval), _particle_tic(0),
                                                    _algorithm_tic(0), _iteration_tic(0), _sampling_seconds(0.0),
                                                    _output_file_name(settings.output_file), _acceptance_rates(),
                                                    _epsilons(), _iteration_nbrs(), _seconds_for_iteration(),
                                                    _sampling_seconds_for_iteration(), _log_zd(), _log_var_zd(),
                                                    _log_zl(), _log_var_zl(), _log_ztot(), _log_var_ztot(),
                                                    _log_v_min(), _var_L_contribution(), _max_L_contribution() {}

    LFNSLogger::~LFNSLogger() {}

    void LFNSLogger::lfnsStarted(int m, double epsilon) {
        std::cout << "\n\nStarting LF-NS algorithm at initial iteration " << m << " and initial epsilon " << epsilon
                  << std::endl << std::endl;
        _algorithm_tic = clock();
    }

    void LFNSLogger::lfnsResume(int m, double epsilon) {
        std::cout << "Resume LF-NS algorithm after initial iteration " << m << " and last epsilon " << epsilon
                  << std::endl << std::endl;
    }

    void LFNSLogger::iterationStarted(int m) {
        std::cout << "\n\nStarting iteration " << m << std::endl;
        _iteration_nbrs.push_back(m);
        _num_accepted_iteration = 0;
        _num_samples_iteration = 0;
        _iteration_tic = clock();
        _sampling_seconds = 0;
    }

    void LFNSLogger::deadPointAdded(const LFNSParticle &particle) {
        _remaining_required_particles_iteration++;
    }

    void LFNSLogger::epsilonUpdated(double epsilon) {
        std::cout << "New epsilon: " << epsilon << std::endl;
        _epsilons.push_back(epsilon);
    }

    void LFNSLogger::samplerUpdated(LFNSSampler &sampler, time_t clocks_for_sampler_update) {
        _sampling_seconds += clocks_for_sampler_update / (double) CLOCKS_PER_SEC;
        std::cout << "Sampler updated: ";
        sampler.writeToStream(std::cout);
        std::cout << std::endl;
    }

    void LFNSLogger::thetaSampled(const std::vector<double> &theta, time_t clocks_sampling) {
        _num_samples_particle++;
        _num_samples_iteration++;
        _particle_tic = clock();
        _sampling_seconds += clocks_sampling / (double) CLOCKS_PER_SEC;
    }

    void LFNSLogger::likelihoodComputed(double likelihood) {}

    void LFNSLogger::particleAccepted(const std::vector<double> &theta, double l) {
        particleAccepted(theta, l, clock() - _particle_tic);
    }


    void LFNSLogger::particleAccepted(const std::vector<double> &theta, double l, time_t clocks_particle,
                                      int origin_process) {

        _num_accepted_iteration++;
        _remaining_required_particles_iteration--;

        if (_num_accepted_iteration % _print_interval == 0) {
            std::cout << "i = " << _num_accepted_iteration << ", log likelihood = " << l
                      << ", acceptance probability = " << 1.0 / _num_samples_particle << ", time for particle = "
                      << clocks_particle / (double) CLOCKS_PER_SEC;
            if (origin_process > 0) { std::cout << ", process: " << origin_process; }
            std::cout << std::endl;
        }

        if (_num_accepted_iteration % _acceptance_info_print_interval == 0 ||
            _remaining_required_particles_iteration == 0) { _printAcceptanceInfo(); }

        _num_samples_particle = 0;
    }

    void LFNSLogger::logIterationResults(PosteriorQuantitites &post_quant) {
        double acceptance_rate = _num_accepted_iteration / (double) _num_samples_iteration;

        clock_t toc2 = clock();

        double log_zd = post_quant.log_zd;
        _log_zd.push_back(log_zd);
        double log_var_zd = post_quant.log_zd_var;
        _log_var_zd.push_back(log_var_zd);

        double log_zl = post_quant.log_zl;
        _log_zl.push_back(log_zl);
        double log_var_zl = post_quant.log_zl_var;
        _log_var_zl.push_back(log_var_zl);

        double log_ztot = post_quant.log_ztot;
        _log_ztot.push_back(log_ztot);

        double log_var_z_tot = post_quant.log_ztot_var;
        _log_var_ztot.push_back(log_var_z_tot);

        double log_min_var = post_quant.log_min_var;
        _log_v_min.push_back(log_min_var);

        double var_L_contribution = post_quant.log_max_std_improvement;
        _var_L_contribution.push_back(var_L_contribution);

        double max_L_contribution = post_quant.log_final_vol + post_quant.log_max_live - log_zd;
        _max_L_contribution.push_back(max_L_contribution);

        double current_contribution;
        if (_log_ztot.size() == 1) {
            current_contribution = log_ztot;
        } else {
            double first = std::max(log_ztot, *(_log_ztot.end() - 2));
            double second = std::min(log_ztot, *(_log_ztot.end() - 2));
            current_contribution = base::MathUtils::diffOfLog(first, second);
        }
        std::cout << std::endl << "\nIteration " << _iteration_nbrs.back()
                  << " needed " << base::Utils::getTimeFromClocks(toc2 - _iteration_tic) << std::endl;
        std::cout << "Total LF-NS time so far: " << base::Utils::getTimeFromClocks(toc2 - _algorithm_tic) << std::endl;
        std::cout << "Acceptance rate: " << acceptance_rate << std::endl;
        std::cout << "The total log Bayesian Evidence after this iteration is "
                  << log_ztot
                  << ". This iteration contributed "
                  << exp(current_contribution - log_ztot) * 100
                  << "% to the total Bayesian Evidence" << std::endl;
    }

    void LFNSLogger::logIterationStats(){
        double acceptance_rate = _num_accepted_iteration / (double) _num_samples_iteration;
        _acceptance_rates.push_back(acceptance_rate);

        clock_t toc2 = clock();

        double sec = ((double) (toc2 - _iteration_tic) / CLOCKS_PER_SEC);
        _seconds_for_iteration.push_back(sec);

        _sampling_seconds_for_iteration.push_back(_sampling_seconds);

    }

    void LFNSLogger::lfnsTerminated() {
        time_t toc = clock();
        std::cout << "\n\nLFNS algorithm successfully terminated!" << std::endl;
        std::cout << "Total LF-NS time: " << base::Utils::getTimeFromClocks(toc - _algorithm_tic) << std::endl
                  << std::endl;
    }

    void LFNSLogger::_printAcceptanceInfo() {
        double acceptance_rate = ((double) _num_accepted_iteration)
                                 / ((double) _num_samples_iteration);
        std::cout << std::endl << std::endl << "Particle acceptance rate: "
                  << acceptance_rate << std::endl;

        clock_t toc2 = clock();
        std::cout << "Time needed so far for this iteration: " << base::Utils::getTimeFromClocks(toc2 - _iteration_tic)
                  <<
                  std::endl;

        double eta = ((toc2 - _iteration_tic) / ((double) _num_accepted_iteration))
                     * _remaining_required_particles_iteration;
        std::cout << "Estimated time remaining: " << base::Utils::getTimeFromClocks(eta) <<
                  std::endl << std::endl;
    }

    int LFNSLogger::iterationNumber() { return _iteration_nbrs.back(); }


    double LFNSLogger::lastEpsilon() { return _epsilons.back(); }

    double LFNSLogger::lastAcceptanceRate() { return _acceptance_rates.back(); }

    void LFNSLogger::writeToFile() {
        std::string log_file_name =
                base::IoUtils::appendToFileName(_output_file_name,
                                                "log_file");
        std::ofstream log_file_file(log_file_name.c_str());
        if (!log_file_file.is_open()) {
            std::cerr << "error opening file "
                      << log_file_name.c_str()
                      << " for writing run log.. run log could not be saved!!"
                      << std::endl << std::endl;
            return;
        }
        log_file_file << std::setw(3) << "i" << "\t" << std::setw(15) << std::setprecision(8) << "epsilon"
                      << "\t" << std::setw(20) << std::setprecision(8) << "Acceptance rate" << "\t"
                      << std::setw(12) << std::setprecision(8) << "seconds" << "\t" << std::setw(17)
                      << std::setprecision(8) << "log(Z_tot)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "log(Var(Z_tot))" << "\t" << std::setw(20)
                      << std::setprecision(8) << "log(Z_D)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "log(Var(Z_D))" << "\t" << std::setw(20)
                      << std::setprecision(8) << "log(Z_L)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "log(Var(Z_L))" << "\t" << std::setw(20)
                      << std::setprecision(8) << "log(Var_min)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Delta_max" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Delta_LFNS" << "\t" << std::setw(20)
                      << std::setprecision(8) << "sampling seconds" << std::endl;

        for (size_t i = 0; i < _iteration_nbrs.size(); i++) {
            int num_simulation = _iteration_nbrs[i];
            double epsilon = _epsilons[i];
            double acceptance_rate = _acceptance_rates[i];
            double seconds = _seconds_for_iteration[i];

            double log_z_D = _log_zd[i];
            double log_z_L = _log_zl[i];
            double log_z_tot = _log_ztot[i];

            double log_var_z_D = _log_var_zd[i];
            double log_var_z_L = _log_var_zl[i];
            double lov_var_z_tot = _log_var_ztot[i];
            double log_var_min = _log_v_min[i];

            double delta_LFNS = _var_L_contribution[i];
            double delta_max = _max_L_contribution[i];

            double seconds_for_sampling = _sampling_seconds_for_iteration[i];
            log_file_file << std::setw(3) << num_simulation << "\t" << std::setw(15) << std::setprecision(6) << epsilon
                          << "\t" << std::setw(20) << std::setprecision(6) << acceptance_rate << "\t"
                          << std::setw(12) << std::setprecision(8) << seconds << "\t" << std::setw(17)
                          << std::setprecision(6) << log_z_tot << "\t" << std::setw(20)
                          << std::setprecision(6) << lov_var_z_tot << "\t" << std::setw(20)
                          << std::setprecision(6) << log_z_D << "\t" << std::setw(20)
                          << std::setprecision(6) << log_var_z_D << "\t" << std::setw(20)
                          << std::setprecision(6) << log_z_L << "\t" << std::setw(20)
                          << std::setprecision(6) << log_var_z_L << "\t" << std::setw(20)
                          << std::setprecision(6) << log_var_min << "\t" << std::setw(20)
                          << std::setprecision(6) << delta_max << "\t" << std::setw(20)
                          << std::setprecision(6) << delta_LFNS << "\t" << std::setw(20)
                          << std::setprecision(6) << seconds_for_sampling << std::endl;
        }
        log_file_file.close();
        std::cout << "Log wrote into " << log_file_name.c_str() << std::endl;
    }

    void LFNSLogger::readFromFile(std::string previous_log_file_name) {
        std::string log_file_name = previous_log_file_name;
        std::ifstream log_file(log_file_name.c_str());
        if (!log_file.is_open()) {
            std::stringstream ss;
            ss << "error opening file " << log_file_name.c_str() << " for reading logs of previous run!"
               << std::endl;
            throw std::runtime_error(ss.str());
        }

        std::string line;
        getline(log_file, line); // read the title
        std::istringstream iss(line);


        int sim_nbr;
        double epsilon;
        double acceptance_rate;
        double seconds;
        double log_z_D;
        double log_z_L;
        double log_z_tot;

        double log_var_z_D;
        double log_var_z_L;
        double log_var_z_tot;
        double log_var_min;

        double delta_LFNS;
        double delta_max;

        double seconds_for_sampling = 0;

        while (!log_file.eof()) {

            std::getline(log_file, line);
            if (!line.empty()) {
                iss.clear();
                iss.str(line);


                std::string num_simulation_str;
                iss >> num_simulation_str;
                sim_nbr = std::stoi(num_simulation_str.c_str());
                _iteration_nbrs.push_back(sim_nbr);

                std::string epsilon_str;
                iss >> epsilon_str;
                epsilon = std::stod(epsilon_str.c_str());
                _epsilons.push_back(epsilon);

                std::string acceptance_rate_string;
                iss >> acceptance_rate_string;
                acceptance_rate = std::stod(acceptance_rate_string.c_str());
                _acceptance_rates.push_back(acceptance_rate);
                iss >> seconds;
                _seconds_for_iteration.push_back(seconds);

                bool all_BE_provided = true;
                if (!iss.eof()) {
                    iss >> log_z_tot;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> log_var_z_tot;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> log_z_D;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> log_var_z_D;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> log_z_L;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> log_var_z_L;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> log_var_min;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> delta_max;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> delta_LFNS;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> seconds_for_sampling;
                } else { seconds_for_sampling = 0; }

                if (all_BE_provided) {
                    _log_zd.push_back(log_z_D);
                    _log_zl.push_back(log_z_L);
                    _log_ztot.push_back(log_z_tot);
                    _log_var_zd.push_back(log_var_z_D);
                    _log_var_zl.push_back(log_var_z_L);
                    _log_var_ztot.push_back(log_var_z_tot);
                    _log_v_min.push_back(log_var_min);

                    _max_L_contribution.push_back(delta_max);
                    _var_L_contribution.push_back(delta_LFNS);
                }
                _sampling_seconds_for_iteration.push_back(seconds_for_sampling);
            }
        }

        log_file.close();
        std::cout << "Log read from " << log_file_name.c_str() << std::endl;
    }
}
