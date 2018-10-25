//
// Created by jan on 12/10/18.
//

#include <algorithm>
#include "ParserReader.h"

namespace io {
    ParserReader::ParserReader(std::string parser_file_name) : TxtFileReader(parser_file_name) {}

    ParserReader::~ParserReader() {}

    std::vector<std::string> ParserReader::readParameterNames() {
        std::vector<std::string> parameter_names;
        try { parameter_names = _readLineWithCaption(PARAMETER_CAPTION, ","); }
        catch (const std::exception &e) {};
        parameter_names.erase(std::unique(parameter_names.begin(), parameter_names.end()), parameter_names.end());
        return parameter_names;
    }

    std::vector<std::string> ParserReader::readSpeciesNames() {
        std::vector<std::string> species_names;
        try { species_names = _readLineWithCaption(SPECIES_CAPTION, ","); }
        catch (const std::exception &e) {};
        species_names.erase(std::unique(species_names.begin(), species_names.end()), species_names.end());
        return species_names;
    }


    std::vector<std::string> ParserReader::readNormalRdn() {
        std::vector<std::string> normal_number;
        try {
            std::vector<std::string> all_random_numbers = _readCaptionLines(RANDOM_NUMBER_CAPTION);
            for (std::string &random_line : all_random_numbers) {
                std::vector<std::string> random_line_split = _splitInTwo(random_line, "=");
                if (random_line_split[1].find(NORMAL_CAPTION) != std::string::npos) {
                    normal_number.push_back(random_line_split[0]);
                }
            }
        } catch (const std::exception &e) {};
        normal_number.erase(std::unique(normal_number.begin(), normal_number.end()), normal_number.end());
        return normal_number;
    }

    std::vector<std::string> ParserReader::readUniformRdn() {
        std::vector<std::string> uniform_number;
        try {
            std::vector<std::string> all_random_numbers = _readCaptionLines(RANDOM_NUMBER_CAPTION);
            for (std::string &random_line : all_random_numbers) {
                std::vector<std::string> random_line_split = _splitInTwo(random_line, "=");
                if (random_line_split[1].find(UNIFORM_INT_CAPTION) == std::string::npos &&
                    random_line_split[1].find(UNIFORM_CAPTION) != std::string::npos) {
                    uniform_number.push_back(random_line_split[0]);
                }
            }
        } catch (const std::exception &e) {};
        return uniform_number;
    }

    std::vector<std::string> ParserReader::readUniformIntRdn() {
        std::vector<std::string> uniform_number;
        try {
            std::vector<std::string> all_random_numbers = _readCaptionLines(RANDOM_NUMBER_CAPTION);
            for (std::string &random_line : all_random_numbers) {
                std::vector<std::string> random_line_split = _splitInTwo(random_line, "=");
                if (random_line_split[1].find(UNIFORM_INT_CAPTION) != std::string::npos) {
                    uniform_number.push_back(random_line_split[0]);
                }
            }
        } catch (const std::exception &e) {};
        return uniform_number;
    }

    std::vector<std::pair<double, double> > ParserReader::readNormalParams() {
        std::vector<std::pair<double, double> > normal_params;
        try {
            std::vector<std::string> random_number_lines = _readCaptionLines(RANDOM_NUMBER_CAPTION);

            for (std::size_t i = 0; i < random_number_lines.size(); i++) {
                std::vector<std::string> random_line_split = _splitInTwo(random_number_lines[i], "=");
                if (random_line_split[1].find(NORMAL_CAPTION) != std::string::npos) {
                    std::size_t begin_index = random_line_split[1].find("(") + 1;
                    std::size_t end_index = random_line_split[1].find(")");

                    std::string params_str = random_line_split[1].substr(begin_index, end_index - begin_index);
                    std::vector<std::string> params = _splitInTwo(params_str, ",");
                    normal_params.push_back(
                            std::make_pair(std::stod(params[0].c_str()), std::stod(params[1].c_str())));
                }
            }
        } catch (const std::exception &e) {};
        return normal_params;
    }

    std::vector<std::pair<double, double> > ParserReader::readUniformParams() {
        std::vector<std::pair<double, double> > uniform_params;

        try {
            std::vector<std::string> random_number_lines = _readCaptionLines(RANDOM_NUMBER_CAPTION);

            for (std::size_t i = 0; i < random_number_lines.size(); i++) {
                std::vector<std::string> random_line_split = _splitInTwo(random_number_lines[i], "=");

                if (random_line_split[1].find(UNIFORM_INT_CAPTION) == std::string::npos &&
                    random_line_split[1].find(UNIFORM_CAPTION) != std::string::npos) {
                    std::size_t begin_index = random_line_split[1].find("(") + 1;
                    std::size_t end_index = random_line_split[1].find(")");

                    std::string params_str = random_line_split[1].substr(begin_index, end_index - begin_index);
                    std::vector<std::string> params = _splitInTwo(params_str, ",");
                    uniform_params.push_back(
                            std::make_pair(std::stod(params[0].c_str()), std::stod(params[1].c_str())));
                }
            }
        } catch (const std::exception &e) {};
        return uniform_params;
    }

    std::vector<std::pair<int, int> > ParserReader::readUniformIntParams() {
        std::vector<std::pair<int, int> > uniform_params;

        try {
            std::vector<std::string> random_number_lines = _readCaptionLines(RANDOM_NUMBER_CAPTION);

            for (std::size_t i = 0; i < random_number_lines.size(); i++) {
                std::vector<std::string> random_line_split = _splitInTwo(random_number_lines[i], "=");
                if (random_line_split[1].find(UNIFORM_INT_CAPTION) != std::string::npos) {
                    std::size_t begin_index = random_line_split[1].find("(") + 1;
                    std::size_t end_index = random_line_split[1].find(")");

                    std::string params_str = random_line_split[1].substr(begin_index, end_index - begin_index);
                    std::vector<std::string> params = _splitInTwo(params_str, ",");
                    uniform_params.push_back(
                            std::make_pair(std::stoi(params[0].c_str()), std::stoi(params[1].c_str())));
                }
            }
        } catch (const std::exception &e) {};
        return uniform_params;
    }

}