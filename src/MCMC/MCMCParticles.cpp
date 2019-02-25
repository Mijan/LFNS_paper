//
// Created by jan on 25/02/19.
//

#include <iomanip>
#include <cfloat>
#include "MCMCParticles.h"
#include "../base/IoUtils.h"

namespace mcmc {

    MCMCParticles::MCMCParticles(int num_particles) : _particles() { _particles.reserve(num_particles); }

    MCMCParticles::~MCMCParticles() {}

    void MCMCParticles::reserve(std::size_t num_particles) { _particles.reserve(num_particles); }

    void MCMCParticles::push_back(MCMCParticle particle) { _particles.push_back(particle); }

    std::size_t MCMCParticles::size() { return _particles.size(); }

    MCMCParticle &MCMCParticles::back() { return _particles.back(); }

    void MCMCParticles::writeToFile(std::string output_file_name, std::string suffix) {
        std::string output_file_with_suffix = output_file_name;
        if (!suffix.empty()) {
            std::string output_file_with_suffix = base::IoUtils::appendToFileName(output_file_name, suffix);
        }

        std::ofstream dead_points_file(output_file_with_suffix.c_str());
        if (!dead_points_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << output_file_with_suffix.c_str()
               << " for writing dead points. Dead points could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::vector<MCMCParticle>::iterator it = _particles.begin(); it != _particles.end(); it++) {
            const std::vector<double> &theta = it->particle;
            for (int j = 0; j < theta.size(); j++) {
                double part = theta[j];
                dead_points_file << part << " ";
            }
            dead_points_file << std::endl;
        }
        dead_points_file.close();


        std::string likelihood_file_name = base::IoUtils::appendToFileName(output_file_with_suffix, "log_likelihoods");


        std::ofstream likelihood_file(likelihood_file_name.c_str());
        if (!likelihood_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << likelihood_file_name.c_str()
               << " for writing dead points log likelihoods. Dead points log likelihoods could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::vector<MCMCParticle>::iterator it = _particles.begin(); it != _particles.end(); it++) {
            double log_like = it->log_likelihood;
            likelihood_file << std::setprecision(10) << log_like << std::endl;
        }
        likelihood_file.close();
    }

    void MCMCParticles::readFromFile(std::string input_file_name) {
        std::string input_file_with_suffix = input_file_name;
        std::string likelihood_file_name = base::IoUtils::appendToFileName(
                input_file_with_suffix, "log_likelihoods");

        std::ifstream data_file(input_file_with_suffix.c_str());
        if (!data_file.is_open()) {
            std::stringstream ss;
            ss << "error opening file " << input_file_with_suffix.c_str()
               << " for reading dead particles. Particles could not be read!"
               << std::endl << std::endl;
            throw std::runtime_error(ss.str());
        }

        std::ifstream likelihood_file(likelihood_file_name.c_str());
        if (!likelihood_file.is_open()) {
            std::stringstream ss;
            ss << "error opening file " << likelihood_file_name.c_str()
               << " for reading likelihoods of dead particles. Particles could not be read!!"
               << std::endl << std::endl;
            throw std::runtime_error(ss.str());
        }

        while (!data_file.eof()) {

            std::string line;
            while (std::getline(data_file, line)) {
                std::stringstream iss(line);
                double n;
                std::vector<double> theta;
                while (iss >> n) {
                    theta.push_back(n);
                }
                double log_likelihood;

                std::string log_like_str;
                std::getline(likelihood_file, log_like_str);
                try {
                    log_likelihood = std::stod(log_like_str.c_str());
                } catch (const std::exception &e) {
                    log_likelihood = -DBL_MAX;
                    std::cerr << "Failed to parse log-likelihood entry " << log_like_str << " in file "
                              << likelihood_file_name << " as a double, assume value " << log_like_str << std::endl;
                }

                MCMCParticle particle(theta, log_likelihood);
                _particles.push_back(particle);
            }

        }
        data_file.close();
        likelihood_file.close();
    }
}