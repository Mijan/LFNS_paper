/*
 * DPGMMLogger.cpp
 *
 *  Created on: Mar 6, 2015
 *      Author: jan
 */

#include "DPGMMLogger.h"

#include <fstream>
#include <iostream>
#include <iomanip>

#include "IOUtils.h"

namespace DP_GMM {

    DPGMMLogger::DPGMMLogger(std::string output_file_name, int print_frequency) :
            _output_file_name(output_file_name), _print_frequncy(print_frequency) {}

    DPGMMLogger::~DPGMMLogger() {}

    void DPGMMLogger::writeDataToFile(HyperParameters *hyper_parameters,
                                      EstimationMixtureComponentSet *mixture_components, int iteration_nbr) {
        if (iteration_nbr % _print_frequncy != 0 || iteration_nbr == 0) { return; }
        writeDataToFile(hyper_parameters, mixture_components);
    }

    void DPGMMLogger::writeDataToFile(HyperParameters *hyper_parameters,
                                      EstimationMixtureComponentSet *mixture_components) {
        writeDataToFile(mixture_components, _output_file_name);
        writeHyperParams(hyper_parameters, _output_file_name);
    }

    void DPGMMLogger::writeDataToFile(EstimationMixtureComponentSet *mixture_components) {
        writeDataToFile(mixture_components, _output_file_name);
    }

    void DPGMMLogger::writeDataToFile(EstimationMixtureComponentSet *mixture_components,
                                      std::string output_file_name) {
        std::string means_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "means");
        std::string cov_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "covariances");
        std::string weights_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "weights");
        std::string indices_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "indices");

        std::ofstream means_file(means_file_name.c_str());
        if (!means_file.is_open()) {
            std::cerr << "error opening file " << means_file_name.c_str()
                      << "for writing means. Means could not be saved!!" << std::endl;
            return;
        }
        std::ofstream covariance_file(cov_file_name.c_str());
        if (!covariance_file.is_open()) {
            std::cerr << "error opening file " << cov_file_name.c_str()
                      << "for writing covariance. Covariances could not be saved!!"
                      << std::endl;
            return;
        }
        std::ofstream weights_file(weights_file_name.c_str());
        if (!weights_file.is_open()) {
            std::cerr << "error opening file " << weights_file_name.c_str()
                      << "for writing weights. weights could not be saved!!"
                      << std::endl;
            return;
        }
        std::ofstream indices_file(indices_file_name.c_str());
        if (!indices_file.is_open()) {
            std::cerr << "error opening file " << indices_file_name.c_str()
                      << "for writing indices. weights could not be saved!!"
                      << std::endl;
            return;
        }

        EstimationMixtureComponentSet::iterator it;

        for (it = mixture_components->begin(); it != mixture_components->end();
             it++) {

            double weights_entry = (*it)->getComponentWeight();
            weights_file << weights_entry << std::endl;

            EiVector mean = (*it)->getMean();
            EiMatrix covariance = (*it)->getCovariance();
            for (int j = 0; j < mean.size(); j++) {
                double mean_entry = mean(j);
                means_file << mean_entry << std::endl;
                for (int i = 0; i < mean.size(); i++) {
                    double covariance_entry = covariance(j, i);
                    covariance_file << covariance_entry << std::endl;
                }
            }

            const std::unordered_set<int> &data_points = (*it)->getDataPoints();
            for (std::unordered_set<int>::const_iterator it = data_points.begin();
                 it != data_points.end(); it++) {
                indices_file << *it;
                indices_file << " ";
            }
            indices_file << std::endl;
        }
        means_file.close();
        covariance_file.close();
        weights_file.close();
        indices_file.close();
    }


    void DPGMMLogger::writeDataToStream(EstimationMixtureComponentSet *mixture_components, std::ostream &stream) {
        EstimationMixtureComponentSet::iterator it;

        int num_dimensions = (*mixture_components->begin())->getMean().size();

        stream << std::setw(20) << "Weights" << "\t" << std::setw(6 * num_dimensions + 5) << "Covariances" << "\t"
               << "Mean" << std::endl;
        std::string weights_empty_space(7, ' ');
        std::string cov_empty_space(6 * num_dimensions + 5, ' ');
        for (it = mixture_components->begin(); it != mixture_components->end();
             it++) {

            double weights_entry = (*it)->getComponentWeight();
            stream << weights_entry << "\t" << std::endl;

            EiVector mean = (*it)->getMean();
            EiMatrix covariance = (*it)->getCovariance();
            for (int j = 0; j < mean.size(); j++) {
                double covariance_entry = covariance(j, 0);
                stream << weights_empty_space << "\t" << std::setprecision(5) << covariance_entry << " ";
                for (int i = 1; i < mean.size(); i++) {
                    covariance_entry = covariance(j, i);
                    stream << std::setprecision(5) << covariance_entry << " ";
                }
                stream << std::endl;
            }

            for (int j = 0; j < mean.size(); j++) {
                double mean_entry = mean(j);
                stream << weights_empty_space << "\t" << cov_empty_space << "\t" << mean_entry << std::endl;
            }
        }

    }

// TODO find better solution than double implementation come up with mixture component interface
    void DPGMMLogger::writeDataToFile(GaussMixtureComponentSet &mixture_components,
                                      std::string output_file_name) {
        std::string means_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "means");
        std::string cov_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "covariances");
        std::string weights_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "weights");

        std::ofstream means_file(means_file_name.c_str());
        if (!means_file.is_open()) {
            std::cerr << "error opening file " << means_file_name.c_str()
                      << "for writing means. Means could not be saved!!" << std::endl;
            return;
        }
        std::ofstream covariance_file(cov_file_name.c_str());
        if (!covariance_file.is_open()) {
            std::cerr << "error opening file " << cov_file_name.c_str()
                      << "for writing covariance. Covariances could not be saved!!"
                      << std::endl;
            return;
        }
        std::ofstream weights_file(weights_file_name.c_str());
        if (!weights_file.is_open()) {
            std::cerr << "error opening file " << weights_file_name.c_str()
                      << "for writing weights. weights could not be saved!!"
                      << std::endl;
            return;
        }

        GaussMixtureComponentSet::iterator it;

        for (it = mixture_components.begin(); it != mixture_components.end();
             it++) {

            double weights_entry = (*it)->comp_weight;
            weights_file << weights_entry << std::endl;

            const EiVector mean = (*it)->mean;
            const EiMatrix covariance = (*it)->cov;
            for (int j = 0; j < mean.size(); j++) {
                double mean_entry = mean(j);
                means_file << mean_entry << std::endl;
                for (int i = 0; i < mean.size(); i++) {
                    double covariance_entry = covariance(j, i);
                    covariance_file << covariance_entry << std::endl;
                }

            }
        }
        means_file.close();
        covariance_file.close();
        weights_file.close();
    }

    void DPGMMLogger::writeDataToStream(GaussMixtureComponentSet &mixture_components, std::ostream &stream) {
        GaussMixtureComponentSet::iterator it;

        int num_dimensions = (*mixture_components.begin())->mean.size();

        stream << std::setw(9)  << "Weights" << "\t" << std::setw(6 * num_dimensions + 5) << "Covariances" << "\t"
               << "Mean" << std::endl;
        std::string weights_empty_space(7, ' ');
        std::string cov_empty_space(6 * num_dimensions + 5, ' ');
        for (it = mixture_components.begin(); it != mixture_components.end();
             it++) {

            double weights_entry = (*it)->comp_weight;
            stream << std::setw(9) << std::setprecision(5) <<weights_entry << "\t";

            EiVector mean = (*it)->mean;
            EiMatrix covariance = (*it)->cov;
            for (int j = 0; j < mean.size(); j++) {
                double covariance_entry = covariance(j, 0);
                for (int i = 0; i < mean.size(); i++) {
                    covariance_entry = covariance(j, i);
                    stream << std::setprecision(5) << covariance_entry << " ";
                }
                stream << "\t" << mean(j) << std::endl;
            }
        }
    }

    void DPGMMLogger::writeHyperParams(HyperParameters *hyper_parameters,
                                       std::string output_file_name) {
        std::string hyeper_parameters_file_name = IOUtils::appendToFileName(
                output_file_name.c_str(), "hyper_params");
        std::ofstream hyper_params_file(hyeper_parameters_file_name.c_str());
        if (!hyper_params_file.is_open()) {
            std::cerr << "error opening file "
                      << hyeper_parameters_file_name.c_str()
                      << "for writing hyperparameters. hyperparameters could not be saved!!"
                      << std::endl;
            return;
        }

        double alpha = hyper_parameters->getAlpha();
        double beta = hyper_parameters->getBeta();
        const EiVector &xi = hyper_parameters->getXi();
        const EiMatrix &W = hyper_parameters->getW();
        double rho = hyper_parameters->getRho();

        hyper_params_file << alpha << std::endl;
        hyper_params_file << beta << std::endl;

        hyper_params_file << xi << std::endl;
        for (int row_nbr = 0; row_nbr < W.rows(); row_nbr++) {
            EiVector row = W.row(row_nbr);
            hyper_params_file << row << std::endl;
        }
        hyper_params_file << rho << std::endl;

        hyper_params_file.close();
    }

    void DPGMMLogger::writeData(std::vector<EiVector> means,
                                std::vector<EiMatrix> covs) {
        std::string means_file_name = IOUtils::appendToFileName(
                _output_file_name.c_str(), "means");
        std::string cov_file_name = IOUtils::appendToFileName(
                _output_file_name.c_str(), "covariances");

        std::ofstream means_file(means_file_name.c_str());
        if (!means_file.is_open()) {
            std::cerr << "error opening file " << means_file_name.c_str()
                      << "for writing means. Means could not be saved!!" << std::endl;
            return;
        }
        std::ofstream covariance_file(cov_file_name.c_str());
        if (!covariance_file.is_open()) {
            std::cerr << "error opening file " << cov_file_name.c_str()
                      << "for writing covariance. Covariances could not be saved!!"
                      << std::endl;
            return;
        }
        for (std::size_t comp_nbr = 0; comp_nbr < means.size(); comp_nbr++) {
            EiVector mean = means[comp_nbr];
            EiMatrix covariance = covs[comp_nbr];
            for (int j = 0; j < mean.size(); j++) {
                double mean_entry = mean(j);
                means_file << mean_entry << std::endl;
                for (int i = 0; i < mean.size(); i++) {
                    double covariance_entry = covariance(j, i);
                    covariance_file << covariance_entry << std::endl;
                }

            }
        }
    }

} /* namespace DP_GMM */
