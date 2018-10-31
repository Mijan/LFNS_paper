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
        int n = 1;
        std::vector<std::string> experiments_for_simulation;
        std::vector<double> parameter = {};
        std::string parameter_file = "";
        double initial_time = 0;
        double final_time = 100;
        double interval = 1;
        simulator::MODEL_TYPE model_type = simulator::MODEL_TYPE::STOCH;
        std::vector<std::string> param_names;


        void print(std::ostream &stream) {
            if (model_type == simulator::MODEL_TYPE::STOCH) {
                stream << "A SSA simulator will be used" << std::endl;
            } else {
                stream << "An ODE simulator will be used" << std::endl;
            }


            if (!parameter_file.empty()) {
                stream << "Parameters will be read from " << parameter_file << std::endl;
            } else {
                stream << "\nParameters for simulation:" << std::endl;
            }


            std::size_t max_name_length = 0;
            for (std::string &param_name : param_names) {
                if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
            }
            std::size_t max_value_length = 16 + 8;

            stream << std::setw(max_name_length) << "Name ";
            if (parameter_file.empty()) { stream << std::setw(max_value_length) << "simulation value"; }
            stream << std::endl;

            int index = 0;
            for (std::string &param_name : param_names) {
                if (parameter_file.empty()) {
                    stream << std::setw(max_name_length) << param_name;
                    std::stringstream value_str;
                    if (parameter_file.empty()) {
                        value_str << parameter[index++];
                        stream << std::setw(max_value_length) << value_str.str();
                    }
                }
                stream << std::endl;
            }
            stream << std::endl;
        }

    };
}


#endif //LFNS_SIMULATIONSETTINGS_H
