//
// Created by jan on 05/10/18.
//

#include <iomanip>
#include "DeadParticleSet.h"
#include "../base/IoUtils.h"
#include <exception>

namespace lfns {
    DeadParticleSet::DeadParticleSet() : _dead_points() {}

    void DeadParticleSet::push_back(const LFNSParticle &particle) { _dead_points.push_back(particle); }

    std::size_t DeadParticleSet::size() { return _dead_points.size(); }

    const LFNSParticle &DeadParticleSet::operator[](std::size_t i) const { return _dead_points[i]; }

    std::vector<LFNSParticle>::iterator DeadParticleSet::begin() { return _dead_points.begin(); }

    std::vector<LFNSParticle>::iterator DeadParticleSet::end() { return _dead_points.end(); }


    void DeadParticleSet::writeToFile(std::string output_file_name, std::string suffix) {
        std::string output_file_with_suffix = base::IoUtils::appendToFileName(output_file_name, suffix);

        std::ofstream dead_points_file(output_file_with_suffix.c_str());
        if (!dead_points_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << output_file_with_suffix.c_str()
               << " for writing dead points. Dead points could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::vector<LFNSParticle>::iterator it = _dead_points.begin(); it != _dead_points.end(); it++) {
            const std::vector<double> &theta = it->getTheta();
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

        for (std::vector<LFNSParticle>::iterator it = _dead_points.begin(); it != _dead_points.end(); it++) {
            double log_like = it->getLogLikelihood();
            likelihood_file << std::setprecision(10) << log_like << std::endl;
        }
        likelihood_file.close();
    }

    void DeadParticleSet::readFromFile(std::string input_file_name) {
        std::string input_file_with_suffix = input_file_name;
        std::string likelihood_file_name = base::IoUtils::appendToFileName(
                input_file_with_suffix, "log_likelihoods");

        std::ifstream data_file(input_file_with_suffix.c_str());
        if (!data_file.is_open()) {
            std::cerr << "error opening file "
                      << input_file_with_suffix.c_str()
                      <<
                      " for reading dead particles. Particles could not be read! Algorithm will start without previous particles!"
                      << std::endl << std::endl;
            return;
        }

        std::ifstream likelihood_file(likelihood_file_name.c_str());
        if (!likelihood_file.is_open()) {
            std::cerr << "error opening file " << likelihood_file_name.c_str()
                      << " for reading likelihoods of dead particles. Particles could not be read!!"
                      << std::endl << std::endl;
            return;
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
                log_likelihood = std::stod(log_like_str.c_str());

                LFNSParticle particle(theta, log_likelihood);
                _dead_points.push_back(particle);
            }

        }
        data_file.close();
        likelihood_file.close();
    }
}