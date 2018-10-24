//
// Created by jan on 05/10/18.
//

#include <cmath>
#include <iomanip>
#include "LiveParticleSet.h"
#include "../base/IoUtils.h"

namespace lfns {
    LiveParticleSet::LiveParticleSet() : _live_points() {}

    int LiveParticleSet::numberParticles() { return _live_points.size(); }

    void LiveParticleSet::push_back(std::vector<double> vector, double log_likelihood) {
        _live_points.insert(LFNSParticle(vector, log_likelihood));
    }

    const LFNSParticle LiveParticleSet::removeLowestPartcile() {
        LFNSParticle particle = *_live_points.begin();
        _live_points.erase(_live_points.begin());
        return particle;
    }

    double LiveParticleSet::getLowestLikelihood() { return _live_points.begin()->getLogLikelihood(); }

    double LiveParticleSet::getHighestLogLikelihood() { return _live_points.rbegin()->getLogLikelihood(); }

    double LiveParticleSet::getLogAverageL() {
        double average = 0;
        std::set<LFNSParticle, lfns_particle_comp>::iterator it = _live_points.begin();
        double max_log_like = getHighestLogLikelihood();

        for (it; it != _live_points.end(); it++) {
            average += std::exp(it->getLogLikelihood() - max_log_like);
        }
        return std::log(average) - std::log(numberParticles()) + max_log_like;
    }


    double LiveParticleSet::getLogVariance() {
        double log_average = getLogAverageL();
        return getLogVariance(log_average);
    }

    double LiveParticleSet::getLogVariance(double log_average) {
        double var = 0;
        int N = numberParticles();
        double max_log_like = getHighestLogLikelihood();

        std::set<LFNSParticle, lfns_particle_comp>::iterator it = _live_points.begin();
        for (it; it != _live_points.end(); it++) {
            var += std::pow(std::exp(it->getLogLikelihood() - max_log_like) - std::exp(log_average - max_log_like), 2);
        }
        var = log(var) + 2 * max_log_like - std::log(N - 1);
        return var;
    }


    std::multiset<LFNSParticle>::iterator LiveParticleSet::begin() { return _live_points.begin(); }

    std::multiset<LFNSParticle>::iterator LiveParticleSet::end() { return _live_points.end(); }

    void LiveParticleSet::writeToFile(std::string output_file_name, std::string suffix) {
        std::string output_file_with_suffix = base::IoUtils::appendToFileName(output_file_name, suffix);

        std::ofstream live_points_file(output_file_with_suffix.c_str());
        if (!live_points_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << output_file_with_suffix.c_str()
               << " for writing live points. Live points could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::set<LFNSParticle>::iterator it = _live_points.begin(); it != _live_points.end(); it++) {
            const std::vector<double> &theta = it->getTheta();
            for (int j = 0; j < theta.size(); j++) {
                double part = theta[j];
                live_points_file << part << " ";
            }
            live_points_file << std::endl;
        }
        live_points_file.close();


        std::string likelihood_file_name = base::IoUtils::appendToFileName(output_file_with_suffix, "log_likelihoods");

        std::ofstream likelihood_file(likelihood_file_name.c_str());
        if (!likelihood_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << likelihood_file_name.c_str()
               << " for writing live points log likelihoods. Live points log likelihoods could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::set<LFNSParticle>::iterator it = _live_points.begin(); it != _live_points.end(); it++) {
            double log_like = it->getLogLikelihood();
            likelihood_file << std::setprecision(10) << log_like << std::endl;
        }
        likelihood_file.close();
    }

    void LiveParticleSet::readFromFile(std::string input_file_name) {
        std::string input_file_with_suffix = input_file_name;
        std::string likelihood_file_name = base::IoUtils::appendToFileName(
                input_file_with_suffix, "log_likelihoods");

        std::ifstream data_file(input_file_with_suffix.c_str());
        if (!data_file.is_open()) {
            std::cerr << "error opening file "
                      << input_file_with_suffix.c_str()
                      <<
                      " for reading live particles. Particles could not be read! Algorithm will start without previous particles!"
                      << std::endl << std::endl;
            return;
        }

        std::ifstream likelihood_file(likelihood_file_name.c_str());
        if (!likelihood_file.is_open()) {
            std::cerr << "error opening file " << likelihood_file_name.c_str()
                      << " for reading likelihoods of live particles. Particles could not be read!!"
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
                log_likelihood = atof(log_like_str.c_str());

                LFNSParticle particle(theta, log_likelihood);
                _live_points.insert(particle);
            }

        }
        data_file.close();
        likelihood_file.close();
    }

    base::EiMatrix LiveParticleSet::toMatrix() {
        if (_live_points.empty()) {
            throw std::runtime_error("Tried to convert live points to Eigen Matrix, but live points set is empty!");
        }

        int num_dimensions = _live_points.begin()->getTheta().size();
        base::EiMatrix matrix(_live_points.size(), num_dimensions);

        int i = 0;
        for (std::set<LFNSParticle>::iterator it = _live_points.begin(); it != _live_points.end(); it++) {
            for (int j = 0; j < num_dimensions; j++) {
                matrix(i, j) = (it->getTheta())[j];
            }
            i++;
        }
        return matrix;
    }

};
