//
// Created by jan on 12/10/18.
//

#include "InitialValueReader.h"

namespace io {
    InitialValueReader::InitialValueReader(std::string initial_value_file_name) : ParserReader(
            initial_value_file_name) {}

    InitialValueReader::~InitialValueReader() {}

    std::vector<std::string> InitialValueReader::readInitialStates() {
        std::vector<std::string> outputs;

        std::vector<std::string> random_number_lines = _readCaptionLines(INITIAL_VALUES);

        for (std::size_t i = 0; i < random_number_lines.size(); i++) {
            std::vector<std::string> output = _splitInTwo(random_number_lines[i], ":");
            outputs.push_back(output[0]);
        }
        return outputs;
    }

    std::vector<std::string> InitialValueReader::readInitialEquations() {
        std::vector<std::string> outputs;

        std::vector<std::string> outputs_lines;
        std::vector<std::string> random_number_lines = _readCaptionLines(INITIAL_VALUES);

        for (std::size_t i = 0; i < random_number_lines.size(); i++) {
            std::vector<std::string> output = _splitInTwo(random_number_lines[i], ":");
            outputs.push_back(output[1]);
        }
        return outputs;
    }
}