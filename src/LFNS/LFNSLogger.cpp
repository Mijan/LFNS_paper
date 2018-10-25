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
                                                    _print_interval(1), _acceptance_info_print_interval(100),
                                                    _particle_tic(0), _iteration_tic(0),
                                                    _output_file_name(settings.output_file), _acceptance_rates(),
                                                    _epsilons(), _iteration_nbrs(), _seconds_for_iteration(),
                                                    _log_BE_dead(), _log_BE_dead_var(), _log_BE_live(),
                                                    _log_BE_live_var(), _log_BE_tot(), _log_BE_tot_var(),
                                                    _var_L_contribution(), _max_L_contribution() {}

    LFNSLogger::~LFNSLogger() {}

    void LFNSLogger::lfnsStarted(int m, double epsilon) {
        std::cout << "\n\nStarting LF-NS algorithm at initial iteration " << m << " and initial epsilon " << epsilon
                  << std::endl << std::endl;
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
    }

    void LFNSLogger::deadPointAdded(const LFNSParticle &particle) {
        _remaining_required_particles_iteration++;
    }

    void LFNSLogger::epsilonUpdated(double epsilon) {
        std::cout << "New epsilon: " << epsilon << std::endl;
        _epsilons.push_back(epsilon);
    }

    void LFNSLogger::samplerUpdated(LFNSSampler &sampler) {
        std::cout << "Sampler updated: ";
        sampler.writeToStream(std::cout);
        std::cout << std::endl;
    }

    void LFNSLogger::thetaSampled(const std::vector<double> &theta) {
        _num_samples_particle++;
        _num_samples_iteration++;
        _particle_tic = clock();
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

        if (_num_accepted_iteration % _acceptance_info_print_interval == 0) { _printAcceptanceInfo(); }

        _num_samples_particle = 0;
    }

    void LFNSLogger::logIterationResults(PosteriorQuantitites &post_quant) {
        double acceptance_rate = _num_accepted_iteration / (double) _num_samples_iteration;
        _acceptance_rates.push_back(acceptance_rate);

        clock_t toc2 = clock();

        double sec = ((double) (toc2 - _iteration_tic) / CLOCKS_PER_SEC);
        _seconds_for_iteration.push_back(sec);

        double log_BE_dead = post_quant.log_zd;
        _log_BE_dead.push_back(log_BE_dead);
        double log_BE_dead_var = post_quant.log_zd_var;
        _log_BE_dead_var.push_back(log_BE_dead_var);

        double log_BE_live = post_quant.log_zl;
        _log_BE_live.push_back(log_BE_live);
        double log_BE_live_var = post_quant.log_zl_var;
        _log_BE_live_var.push_back(log_BE_live_var);

        double log_BE_tot = post_quant.log_ztot;
        _log_BE_tot.push_back(log_BE_tot);

        double log_BE_tot_var = post_quant.log_ztot_var;
        _log_BE_tot_var.push_back(log_BE_tot_var);

        double var_L_contribution = post_quant.log_max_std_improvement;
        _var_L_contribution.push_back(var_L_contribution);

        double max_L_contribution = post_quant.log_final_vol + post_quant.log_max_live - log_BE_dead;
        _max_L_contribution.push_back(max_L_contribution);

        double current_contribution;
        if (_log_BE_tot.size() == 1) {
            current_contribution = log_BE_tot;
        } else {
            double first = std::max(log_BE_tot, *(_log_BE_tot.end() - 2));
            double second = std::min(log_BE_tot, *(_log_BE_tot.end() - 2));
            current_contribution = base::MathUtils::diffOfLog(first, second);
        }
        std::cout << std::endl << "\nIteration " << _iteration_nbrs.back()
                  << " needed " << base::Utils::getTimeFromClocks(toc2 - _iteration_tic) << std::endl;
        std::cout << "The total log Bayesian Evidence after this iteration is "
                  << log_BE_tot
                  << ". This iteration contributed "
                  << exp(current_contribution - log_BE_tot) * 100
                  << "% to the total Bayesian Evidence" << std::endl;
    }

    void LFNSLogger::lfnsTerminated() {
        std::cout << "\n\nLFNS algorithm successfully terminated!" << std::endl << std::endl;
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
                      << std::setprecision(8) << "Z_tot" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Var(Z_tot)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Z_D" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Var(Z_D)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Z_L" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Var(Z_L)" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Delta_max" << "\t" << std::setw(20)
                      << std::setprecision(8) << "Delta_LFNS" << std::endl;

        for (size_t i = 0; i < _iteration_nbrs.size(); i++) {
            int num_simulation = _iteration_nbrs[i];
            double epsilon = _epsilons[i];
            double acceptance_rate = _acceptance_rates[i];
            double seconds = _seconds_for_iteration[i];

            double log_z_D = _log_BE_dead[i];
            double log_z_L = _log_BE_live[i];
            double log_z_tot = _log_BE_tot[i];

            double log_var_z_D = _log_BE_dead_var[i];
            double log_var_z_L = _log_BE_live_var[i];
            double lov_var_z_tot = _log_BE_tot_var[i];

            double delta_LFNS = _var_L_contribution[i];
            double delta_max = _max_L_contribution[i];
            log_file_file << std::setw(3) << num_simulation << "\t" << std::setw(15) << std::setprecision(6) << epsilon
                          << "\t" << std::setw(20) << std::setprecision(6) << acceptance_rate << "\t"
                          << std::setw(12) << std::setprecision(8) << seconds << "\t" << std::setw(17)
                          << std::setprecision(6) << log_z_tot << "\t" << std::setw(20)
                          << std::setprecision(6) << lov_var_z_tot << "\t" << std::setw(20)
                          << std::setprecision(6) << log_z_D << "\t" << std::setw(20)
                          << std::setprecision(6) << log_var_z_D << "\t" << std::setw(20)
                          << std::setprecision(6) << log_z_L << "\t" << std::setw(20)
                          << std::setprecision(6) << log_var_z_L << "\t" << std::setw(20)
                          << std::setprecision(6) << delta_max << "\t" << std::setw(20)
                          << std::setprecision(6) << delta_LFNS << std::endl;
        }
        log_file_file.close();
        std::cout << "Log wrote into " << log_file_name.c_str() << std::endl;
    }

    void LFNSLogger::readFromFile(std::string previous_log_file_name) {
        std::string log_file_name = previous_log_file_name;
        std::ifstream log_file(log_file_name.c_str());
        if (!log_file.is_open()) {
            std::cerr << "error opening file " << log_file_name.c_str() << " for reading logs of previous run!"
                      << std::endl << std::endl;
            return;
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

        double delta_LFNS;
        double delta_max;

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
                    iss >> delta_max;
                } else { all_BE_provided = false; }
                if (!iss.eof()) {
                    iss >> delta_LFNS;
                } else { all_BE_provided = false; }

                if (all_BE_provided) {
                    _log_BE_dead.push_back(log_z_D);
                    _log_BE_live.push_back(log_z_L);
                    _log_BE_tot.push_back(log_z_tot);
                    _log_BE_dead_var.push_back(log_var_z_D);
                    _log_BE_live_var.push_back(log_var_z_L);
                    _log_BE_tot_var.push_back(log_var_z_tot);

                    _max_L_contribution.push_back(delta_max);
                    _var_L_contribution.push_back(delta_LFNS);
                }
            }
        }

        log_file.close();
        std::cout << "Log read from " << log_file_name.c_str() << std::endl;
    }
}
