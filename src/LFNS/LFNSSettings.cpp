//
// Created by jan on 04/10/18.
//

#include <iostream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include "LFNSSettings.h"

namespace lfns {

    std::vector<std::string> LFNSSettings::getUnfixedParameters() const {
        std::vector<std::string> unfixed_parameters;
        for (const ParameterSetting &setting: parameters) {
            if (!setting.fixed) { unfixed_parameters.push_back(setting.name); }
        }
        return unfixed_parameters;
    }

    std::vector<std::pair<double, double> > LFNSSettings::getBounds(const std::vector<std::string> &param_names) const {
        std::vector<std::pair<double, double> > bounds;
        for (const std::string &param: param_names) {
            bool param_found = false;
            for (const ParameterSetting &setting: parameters) {
                if (setting.name.compare(param) == 0) {
                    bounds.push_back(setting.bounds);
                    param_found = true;
                }
            }
            if (!param_found) {
                std::stringstream ss;
                ss << "Tried to obtain bound for parameter " << param << " but no bounds for this parameter provided!"
                   << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        return bounds;
    }

    std::vector<int> LFNSSettings::getLogParams(const std::vector<std::string> &param_names) const {
        std::vector<int> log_params;
        for (int i = 0; i < param_names.size(); i++) {
            const std::string &param = param_names[i];
            bool param_found = false;
            for (const ParameterSetting &setting: parameters) {
                if (setting.name.compare(param) == 0) {
                    if (setting.log_scale) {
                        log_params.push_back(i);
                    }
                    param_found = true;
                }
            }
            if (!param_found) {
                std::stringstream ss;
                ss << "Tried to obtain sclae for parameter " << param << " but no scale for this parameter provided!"
                   << std::endl;
                throw std::runtime_error(ss.str());
            }
        }

        return log_params;
    }

    void LFNSSettings::print(std::ostream &stream) {
        stream << "Output file: " << output_file << std::endl << std::endl;
        stream << "N:\t" << N << std::endl;
        stream << "H:\t" << H << std::endl;
        stream << "r:\t" << r << std::endl;
        stream << "Termination threshold:\t" << std::exp(log_termination) << std::endl;
        stream << std::endl;
        stream << "Model file:\t" << model_file << std::endl;
        stream << "Initial value file:\t" << initial_value_file << std::endl;
        stream << "Measurement file:\t" << measurement_file << std::endl << std::endl;

        stream << "Data used for LFNS:" << std::endl;
        for (std::string experiment: experiments_for_LFNS) {
            stream << "Experiment:\t" << experiment << "\t\tData file: " << data_files[experiment] << "\ttimes file: "
                   << time_files[experiment] << std::endl;
        }
        stream << "Nnumber of trajectories used for each experiment: " << num_used_trajectories << std::endl;
        stream << "\n\n" << std::endl;
        if (previous_log_file.size() > 0) {
            stream << "A previous log file was provided:\t" << previous_log_file
                   << ", and LFNS will continue from the last iteration." << std::endl;
        }


        std::size_t max_name_length = 0;
        for (ParameterSetting &setting : parameters) {
            const std::string &name = setting.name;
            if (max_name_length < name.size()) { max_name_length = name.size(); }
        }
        std::size_t max_bound_length = 24;
        std::size_t max_scale_length = 7;

        stream << std::setw(max_name_length) << "Parameters";
        stream << std::setw(max_bound_length) << "Bounds";
        stream << std::setw(max_scale_length) << "Scale";
        stream << std::endl;

        for (std::size_t param_nbr = 0; param_nbr < parameters.size(); param_nbr++) {
            ParameterSetting setting = parameters[param_nbr];
            stream << std::setw(max_name_length) << setting.name;
            if (setting.fixed) {
                std::stringstream fixed_str;
                fixed_str << "fixed to " << setting.fixed_value;
                stream << std::setw(max_bound_length) << fixed_str.str();
            } else {
                std::stringstream bound_str;
                bound_str << " [" << setting.bounds.first << ", " << setting.bounds.second << "]";
                stream << std::setw(max_bound_length) << bound_str.str();
            }
            if (setting.log_scale) {
                stream << std::setw(max_scale_length) << "log";
            } else {
                stream << std::setw(max_scale_length) << "linear";
            }
            stream << std::endl;
        }
    }

}