//
// Created by jan on 29/10/18.
//

#ifndef LFNS_SIMULATIONSETTINGS_H
#define LFNS_SIMULATIONSETTINGS_H

#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#include "../models/InputPulse.h"

namespace simulator {
    enum class MODEL_TYPE {
        ODE, STOCH
    };
    static std::map<std::string, MODEL_TYPE> MODEL_TYPE_NAME = {{"DET",   MODEL_TYPE::ODE},
                                                                {"STOCH", MODEL_TYPE::STOCH}};

    class SimulationSettings {
    public:

        std::map<std::string, std::vector<models::InputData> > input_datas;

        std::map<std::string, double> fixed_parameters;
        std::string model_file = "";
        std::string output_file = "";
        std::string initial_value_file = "";
        std::string measurement_file = "";
        simulator::MODEL_TYPE model_type = simulator::MODEL_TYPE::STOCH;
        int n = 1;
        std::vector<std::string> experiments_for_simulation;
        std::vector<double> parameter = {};
        std::string parameter_file = "";
        double initial_time = 0;
        double final_time = 100;
        double interval = 1;
        std::vector<std::string> param_names;


        void print(std::ostream &stream) {
            stream << "Output file: " << output_file << std::endl << std::endl;
            stream << "n:\t" << n << std::endl;
            stream << "Model file:\t" << model_file << std::endl;
            stream << "Initial value file:\t" << initial_value_file << std::endl;
            stream << "Measurement file:\t" << measurement_file << std::endl << std::endl;

            if (!parameter_file.empty()) {
                stream << "Parameters will be read from " << parameter_file << std::endl;
            }

            std::size_t max_name_length = 0;
            for (std::string &param_name : param_names) {
                if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
            }
            std::size_t max_value_length = 16 + 8;

            stream << std::setw(max_name_length) << "Parameters ";
            if (parameter_file.empty()) { stream << std::setw(max_value_length) << "simulation value"; }
            stream << std::endl;

            int index = 0;
            for (std::string &param_name : param_names) {
                stream << std::setw(max_name_length) << param_name;
                if (fixed_parameters.count(param_name) > 0) {
                    std::stringstream fixed_str;
                    fixed_str << "fixed to " << fixed_parameters[param_name];
                    stream << std::setw(max_value_length) << fixed_str.str();
                } else if (parameter_file.empty()) {
                    std::stringstream value_str;
                    value_str << parameter[index++];
                    stream << std::setw(max_value_length) << value_str.str();
                }
                stream << std::endl;
            }

            if (!input_datas.empty()) {
                stream << "\n\nThe following experiments will be performed: " << std::endl;
                std::map<std::string, std::vector<models::InputData> >::iterator it;
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


#endif //LFNS_SIMULATIONSETTINGS_H
