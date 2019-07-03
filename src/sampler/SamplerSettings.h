//
// Created by jan on 31/10/18.
//

#ifndef LFNS_SAMPLERSETTINGS_H
#define LFNS_SAMPLERSETTINGS_H

#include <string>
#include <map>
#include <iomanip>
#include <iostream>
#include <vector>
#include <sstream>

namespace sampler {
    enum class SAMPLER_TYPE {
        UNIFORM, NORMAL
    };
    static std::map<SAMPLER_TYPE, std::string> SAMPLER_TYPE_NAME = {{SAMPLER_TYPE::UNIFORM, "Uniform"},
                                                                    {SAMPLER_TYPE::NORMAL,  "Normal"}};

    class SamplerSettings {
    public:
        std::map<std::string, std::pair<double, double>> parameter_bounds;
        std::map<std::string, bool> parameters_log_scale;
        std::vector<std::string> param_names;

        void print(std::ostream &stream) {

            std::map<std::string, std::pair<double, double> >::iterator it;
            std::size_t max_name_length = 0;
            for (it = parameter_bounds.begin(); it != parameter_bounds.end(); it++) {
                const std::string &name = it->first;
                if (max_name_length < name.size()) { max_name_length = name.size(); }
            }

            std::size_t max_bound_length = 24;
            std::size_t max_scale_length = 7;

            stream << "Parameters to be sampled: " << std::endl;
            stream << std::setw(max_name_length) << "Name";
            stream << std::setw(max_bound_length) << "Bounds";
            stream << std::setw(max_scale_length) << "Scale";
            stream << std::endl;

            for (std::string &param_name : param_names) {
                std::pair<double, double> bounds = parameter_bounds[param_name];
                stream << std::setw(max_name_length) << param_name;

                std::stringstream bound_str;
                bound_str << " [" << bounds.first << ", " << bounds.second << "]";
                stream << std::setw(max_bound_length) << bound_str.str();

                if (parameters_log_scale[param_name]) {
                    stream << std::setw(max_scale_length) << "log";
                } else {
                    stream << std::setw(max_scale_length) << "linear";
                }
                stream << std::endl;
            }

            stream << std::endl;
        }


        std::vector<std::pair<double, double> >

        getBounds(const std::vector<std::string> &param_names) {
            std::vector<std::pair<double, double> > bounds;
            for (const std::string &param: param_names) {
                bool param_found = parameter_bounds.count(param) > 0;
                if (param_found) {
                    std::pair<double, double> d = parameter_bounds[param];
                    bounds.push_back(d);
                }
                if (!param_found) {
                    std::stringstream ss;
                    ss << "Tried to obtain bound for parameter " << param
                       << " but no bounds for this parameter provided!"
                       << std::endl;
                    throw std::runtime_error(ss.str());
                }
            }
            return bounds;
        };

        std::vector<int> getLogParams(const std::vector<std::string> &param_names) {
            std::vector<int> log_params;
            for (int i = 0; i < param_names.size(); i++) {
                const std::string &param = param_names[i];
                bool param_found = parameters_log_scale.count(param) > 0;
                if (param_found && parameters_log_scale[param]) { log_params.push_back(i); }
                if (!param_found) {
                    std::stringstream ss;
                    ss << "Tried to obtain sclae for parameter " << param
                       << " but no scale for this parameter provided!"
                       << std::endl;
                    throw std::runtime_error(ss.str());
                }
            }
            return log_params;
        }

        std::vector<int> getLogParams() { return getLogParams(param_names); }
    };
}

#endif //LFNS_SAMPLERSETTINGS_H
