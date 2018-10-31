//
// Created by jan on 31/10/18.
//

#ifndef LFNS_MODELSETTINGS_H
#define LFNS_MODELSETTINGS_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "InputPulse.h"

namespace models {
    class ModelSettings {
    public:
        std::map<std::string, std::vector<models::InputData>> input_datas;

        std::map<std::string, double> fixed_parameters;
        std::string model_file = "";
        std::string initial_value_file = "";
        std::string measurement_file = "";
        std::vector<std::string> param_names;

        std::vector<std::string> getUnfixedParameters() const {
            std::vector<std::string> unfixed_parameters;
            for (std::string param : param_names) {
                if (fixed_parameters.count(param) == 0) { unfixed_parameters.push_back(param); }
            }
            return unfixed_parameters;
        }

        void print(std::ostream &stream) {
            stream << "Model file:\t" << model_file << std::endl;
            stream << "Initial value file:\t" << initial_value_file << std::endl;
            stream << "Measurement file:\t" << measurement_file << std::endl << std::endl;

            std::size_t max_name_length = 0;
            for (std::string &param_name : param_names) {
                if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
            }
            std::size_t max_value_length = 16 + 8;

            stream << std::setw(max_name_length) << "Model parameters ";
            stream << std::endl;

            int index = 0;
            for (std::string &param_name : param_names) {
                stream << std::setw(max_name_length) << param_name;
                if (fixed_parameters.count(param_name) > 0) {
                    std::stringstream fixed_str;
                    fixed_str << "fixed to " << fixed_parameters[param_name];
                    stream << std::setw(max_value_length) << fixed_str.str();
                }
                stream << std::endl;
            }

            if (!input_datas.empty()) {
                stream << "\n\nThe following experiments will be performed: " << std::endl;
                std::map<std::string, std::vector<models::InputData> >::iterator
                        it;
                for (it = input_datas.begin(); it != input_datas.end(); it++) {
                    stream << "Experiment " << it->first << ": " << std::endl;
                    for (models::InputData &input_data : it->second) {
                        stream << "pulses of strength " << input_data.pulse_strenght << " of parameter "
                               << input_data.pulse_inpt_name
                               << " for " << input_data.pulse_duration << " time units, every "
                               << input_data.pulse_period
                               << ", starting from " << input_data.starting_time << std::endl;
                    }
                }
                stream << std::endl;
            }
        }
    };
}


#endif //LFNS_MODELSETTINGS_H
