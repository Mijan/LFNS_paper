//
// Created by jan on 12/10/18.
//

#include "MeasurementModelReader.h"

#include <sstream>
#include <stdexcept>

namespace io {
    MeasurementModelReader::MeasurementModelReader(std::string measurement_model_file_name) : ParserReader(
            measurement_model_file_name) {}

    MeasurementModelReader::~MeasurementModelReader() {}

    std::vector<std::string> MeasurementModelReader::readMeasurements() {
        std::vector<std::string> outputs;

        std::vector<std::string> outputs_lines;
        std::vector<std::string> random_number_lines = _readCaptionLines(MEASUREMENT_CAPTION);

        for (std::size_t i = 0; i < random_number_lines.size(); i++) {
            std::vector<std::string> output = _splitInTwo(random_number_lines[i], "=");
            outputs.push_back(output[1]);
        }
        return outputs;
    }

    std::vector<std::string> MeasurementModelReader::readOutputNames() {
        std::vector<std::string> output_names;

        std::vector<std::string> outputs_lines;
        std::vector<std::string> random_number_lines = _readCaptionLines(MEASUREMENT_CAPTION);

        for (std::size_t i = 0; i < random_number_lines.size(); i++) {
            std::vector<std::string> output = _splitInTwo(random_number_lines[i], "=");
            output_names.push_back(output[0]);
        }
        return output_names;
    }

    std::string MeasurementModelReader::readLogLikelihood(std::string measurement) {

        std::vector<std::string> outputs_lines;
        std::vector<std::string> log_likelihood_lines = _readCaptionLines(LOGLIKELIHOOD_CAPTION);

        for (std::string log_likelihood_line : log_likelihood_lines) {
            std::vector<std::string> likelihoods = _splitInTwo(log_likelihood_line, ":");
            if (likelihoods[0].compare(measurement) == 0) {
                return likelihoods[1];
            }
        }
        std::stringstream os;
        os << "No LogLiklehood line for measurement '" << measurement << "' found!" << std::endl;
        throw std::runtime_error(os.str());

    }
}