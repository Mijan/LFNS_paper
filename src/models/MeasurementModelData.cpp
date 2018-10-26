//
// Created by jan on 05.01.17.
//

#include <algorithm>
#include "MeasurementModelData.h"
#include "../io/MeasurementModelReader.h"

namespace models {

    MeasurementModelData::MeasurementModelData(std::string measurement_model_file_name) : ParserData(
            measurement_model_file_name), measurement_names(), measurement_equations_by_name(),
                                                                                          log_likelihoods_by_name() {
        io::MeasurementModelReader reader(measurement_model_file_name);

        std::vector<std::string> measurement_equations = reader.readMeasurements();
        measurement_names = reader.readOutputNames();

        for (std::size_t i = 0; i < measurement_equations.size(); i++) {
            std::string measurement_name = measurement_names[i];
            measurement_equations_by_name[measurement_name] = measurement_equations[i];
            log_likelihoods_by_name[measurement_name] = reader.readLogLikelihood(measurement_name);
        }
    }

    MeasurementModelData::MeasurementModelData(const MeasurementModelData &rhs) : ParserData(rhs), measurement_names(
            rhs.measurement_names), measurement_equations_by_name(rhs.measurement_equations_by_name),
                                                                                  log_likelihoods_by_name(
                                                                                          rhs.log_likelihoods_by_name) {}

    MeasurementModelData::~MeasurementModelData() {}

    std::size_t MeasurementModelData::getNumMeasurements() const { return measurement_names.size(); }

    void MeasurementModelData::setMeasurementEquation(std::string measurement, std::string equation) {
        if (std::find(measurement_names.begin(), measurement_names.end(), measurement) ==
            measurement_names.end()) {
            std::stringstream os;
            os << "Tried to set measurement equation for " << measurement << ", but no such measurement defined!"
               << std::endl;
            throw std::runtime_error(os.str());
        }
        measurement_equations_by_name[measurement] = equation;
    }

    void MeasurementModelData::setLogLikelihoodEquation(std::string measurement, std::string equation) {
        if (std::find(measurement_names.begin(), measurement_names.end(), measurement) ==
            measurement_names.end()) {
            std::stringstream os;
            os << "Tried to set log likelihood equation for " << measurement << ", but no such measurement defined!"
               << std::endl;
            throw std::runtime_error(os.str());
        }
        log_likelihoods_by_name[measurement] = equation;
    }

    std::string MeasurementModelData::getMeasurementEquationForMeasurment(std::string measurement_name) const {
        if (measurement_equations_by_name.count(measurement_name) == 0) {
            std::stringstream os;
            os << "Tried to get measurement equation for " << measurement_name
               << ", but no measurement equation provided!" << std::endl;
            throw std::runtime_error(os.str());
        }
        return measurement_equations_by_name.at(measurement_name);
    }

    std::string MeasurementModelData::getLogLikelihoodEquationForMeasurment(std::string measurement_name) const {
        if (log_likelihoods_by_name.count(measurement_name) == 0) {
            std::stringstream os;
            os << "Tried to get log likelihood equation for " << measurement_name
               << ", but no log likelihood equation provided!" << std::endl;
            throw std::runtime_error(os.str());
        }
        return log_likelihoods_by_name.at(measurement_name);
    }


    bool MeasurementModelData::isMeasurementName(std::string name) const {
        return std::find(measurement_names.begin(), measurement_names.end(), name) != measurement_names.end();
    }

    std::size_t MeasurementModelData::getMeasurementIndex(std::string measurement_name) const {
        std::size_t measurement_index =
                std::find(measurement_names.begin(), measurement_names.end(), measurement_name) -
                measurement_names.begin();
        if (measurement_index < measurement_names.size()) {
            return measurement_index;
        } else {
            std::ostringstream os;
            os << "Measurement name '" << measurement_name << "' not found!" << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    const std::vector<std::string> &MeasurementModelData::getMeasurementNames() const {
        return measurement_names;
    }
}