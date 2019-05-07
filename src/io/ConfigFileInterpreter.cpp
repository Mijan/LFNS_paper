//
// Created by jan on 24/10/18.
//

#include <sstream>
#include <cfloat>
#include <cstdlib>
#include "ConfigFileInterpreter.h"
#include "XmlPropertyMap.h"
#include "../base/IoUtils.h"
#include "../base/Utils.h"
#include "../sampler/GaussianSampler.h"
#include "../sampler/UniformSampler.h"

namespace io {

    ConfigFileInterpreter::ConfigFileInterpreter(std::string instance_file_name) : _reader(instance_file_name) {}

    ConfigFileInterpreter::~ConfigFileInterpreter() {}


    std::string ConfigFileInterpreter::getModelFileName() {
        std::string model_file = _reader.getEntry("model.model");
        if (base::IoUtils::isPathAbsolute(model_file)) { return model_file; }
        else {
            return _reader.getXmlFilePath() + model_file;
        }
    }

    std::string ConfigFileInterpreter::getMeasurementModelFile() {
        std::string measurement_file = _reader.getEntry("model.measurement");
        if (base::IoUtils::isPathAbsolute(measurement_file)) { return measurement_file; }
        else {
            return _reader.getXmlFilePath() + measurement_file;
        }
    }

    std::string ConfigFileInterpreter::getInitialConditionsFile() {
        std::string initial_conditions_file = _reader.getEntry("model.initialvalue");
        if (base::IoUtils::isPathAbsolute(initial_conditions_file)) {
            return initial_conditions_file;
        } else {
            return _reader.getXmlFilePath() + initial_conditions_file;
        }
    }

    std::string ConfigFileInterpreter::getModelType() { return _reader.getEntry("model.type"); }

    std::map<std::string, std::string> ConfigFileInterpreter::getParameterScales() {
        std::map<std::string, std::string> parameter_scales;

        std::vector<XmlMap> scale_entries = _reader.getEntryMaps("parameters", "scales");

        for (XmlMap &map: scale_entries) {
            std::string param_name_str = map["parameters"].entry;
            std::vector<std::string> param_names = base::Utils::StringToStringVector(param_name_str);
            std::string scale = map["parameters"].getAttributeValue("scale");
            for (std::string param_name : param_names) {
                try { parameter_scales[param_name] = scale; } catch (const std::exception &e) {
                    std::stringstream ss;
                    ss << "Failed to obtain scale for parameter " << param_name << ":\n\t" << e.what() << std::endl;
                    throw std::runtime_error(ss.str());
                };
            }
        }
        return parameter_scales;
    };


    bool ConfigFileInterpreter::detSpeciesProvided() {
        try {
            std::string det_species_str = _reader.getEntry("model.detspecies");
            return true;
        } catch (const std::exception &e) {
            return false;
        }
    }

    bool ConfigFileInterpreter::stochSpeciesProvided() {
        try {
            std::string stoch_species_str = _reader.getEntry("model.stochspecies");
            return true;
        } catch (const std::exception &e) {
            return false;
        }
    }

    std::vector<std::string> ConfigFileInterpreter::getDetSpecies() {
        std::string det_species_str = _reader.getEntry("model.detspecies");
        std::vector<std::string> det_species = base::Utils::StringToStringVector(det_species_str);
        return det_species;
    }

    std::vector<std::string> ConfigFileInterpreter::getStochSpecies() {
        std::string stoch_species_str = _reader.getEntry("model.stochspecies");
        std::vector<std::string> stoch_species = base::Utils::StringToStringVector(stoch_species_str);
        return stoch_species;
    }


    std::map<std::string, std::pair<double, double> > ConfigFileInterpreter::getParameterBounds() {
        std::map<std::string, std::pair<double, double> > bounds;
        std::vector<XmlMap> bound_entries = _reader.getEntryMaps("parameters.bounds", "bound");

        for (XmlMap bound_entry: bound_entries) {
            std::string param_name_str = bound_entry["parameters"].entry;
            std::vector<std::string> param_names = base::Utils::StringToStringVector(param_name_str);
            int num_found_param = 0;
            for (std::string &param_name : param_names) {
                XmlPropertyMap bound_map(bound_entry);
                double lower_bound;
                double upper_bound;
                try {
                    lower_bound = std::stod(bound_map.getValueForKey("parameters", param_name, "lowerbounds").c_str());
                    upper_bound = std::stod(bound_map.getValueForKey("parameters", param_name, "upperbounds").c_str());
                    bounds[param_name] = std::make_pair(lower_bound, upper_bound);
                } catch (const std::exception &e) {
                    std::stringstream ss;
                    ss << "Failed to obtain bound for parameter " << param_name << ":\n\t" << e.what() << std::endl;
                    throw std::runtime_error(ss.str());
                }
            }
        }
        return bounds;
    };


    std::map<std::string, double> ConfigFileInterpreter::getFixedParameters() {
        std::map<std::string, double> fixed_params;
        XmlMap fixed_entries = _reader.getEntryMap("parameters", "fixedparams");
        std::string param_names_str = fixed_entries["parameters"].entry;
        std::vector<std::string> param_names = base::Utils::StringToStringVector(param_names_str);

        XmlPropertyMap fixed_map(fixed_entries);
        for (std::string &param_name : param_names) {
            try {
                double value = std::stod(fixed_map.getValueForKey("parameters", param_name, "values").c_str());
                fixed_params[param_name] = value;
            } catch (const std::exception &e) {
                std::stringstream ss;
                ss << "Failed to obtain fixed value for parameter " << param_name << ":\n\t" << e.what() << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        return fixed_params;
    };


    std::map<std::string, std::string> ConfigFileInterpreter::getDataFiles(std::vector<std::string> experiments) {
        std::map<std::string, std::string> data_files;
        std::vector<XmlMap> data_entries = _reader.getEntryMaps("data", "dataset");

        for (std::string requested_experiment : experiments) {
            for (XmlMap data_entry: data_entries) {
                try {
                    XmlPropertyMap data_entry_map(data_entry);
                    std::string experiment_names_str = data_entry["experiments"].entry;
                    std::vector<std::string> experiemnt_names = base::Utils::StringToStringVector(experiment_names_str);

                    for (std::string experiment_name : experiemnt_names) {
                        if (experiment_name.compare(requested_experiment) == 0) {
                            std::string data_file = data_entry_map.getValueForKey("experiments", experiment_name,
                                                                                  "datafile");
                            if (base::IoUtils::isPathAbsolute(data_file)) {
                                data_files[experiment_name] = data_file;
                            } else {
                                data_files[experiment_name] = _reader.getXmlFilePath() + data_file;
                            }
                        }
                    }

                } catch (const std::exception &e) {
                    std::stringstream ss;
                    ss << "Failed to obtain data file:\n\t" << e.what() << std::endl;
                    throw std::runtime_error(ss.str());
                }
            }
        }
        return data_files;
    };

    std::map<std::string, std::string> ConfigFileInterpreter::getTimesFiles(std::vector<std::string> experiments) {
        std::map<std::string, std::string> time_files;
        std::vector<XmlMap> data_entries = _reader.getEntryMaps("data", "dataset");

        for (std::string requested_experiment : experiments) {
            for (XmlMap data_entry: data_entries) {
                try {
                    XmlPropertyMap data_entry_map(data_entry);
                    std::string experiment_names_str = data_entry["experiments"].entry;
                    std::vector<std::string> experiemnt_names = base::Utils::StringToStringVector(experiment_names_str);
                    for (std::string experiment_name : experiemnt_names) {
                        if (experiment_name.compare(requested_experiment) == 0) {
                            std::string time_file = data_entry_map.getValueForKey("experiments", experiment_name,
                                                                                  "timefile");
                            if (base::IoUtils::isPathAbsolute(time_file)) {
                                time_files[experiment_name] = time_file;
                            } else {
                                time_files[experiment_name] = _reader.getXmlFilePath() + time_file;
                            }
                        }
                    }
                } catch (const std::exception &e) {
                    std::stringstream ss;
                    ss << "Failed to obtain times file:\n\t" << e.what() << std::endl;
                    throw std::runtime_error(ss.str());
                }
            }
        }
        return time_files;
    }


    std::vector<std::string> ConfigFileInterpreter::getExperimentsForLFNS() {
        try {
            std::string experiment_str = _reader.getEntry("LFNS.experiments");
            std::vector<std::string> experiements = base::Utils::StringToStringVector(experiment_str);
            return experiements;
        } catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain experiments for LFNS:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    std::vector<std::string> ConfigFileInterpreter::getExperimentsForSimulations() {
        try {
            std::string experiment_str = _reader.getEntry("Simulation.experiments");
            std::vector<std::string> experiements = base::Utils::StringToStringVector(experiment_str);
            return experiements;
        } catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain experiments for Simulation:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    std::vector<std::string> ConfigFileInterpreter::getExperimentsForEvaluateLikelihood() {
        try {
            std::string experiment_str = _reader.getEntry("ComputeLikelihood.experiments");
            std::vector<std::string> experiements = base::Utils::StringToStringVector(experiment_str);
            return experiements;
        } catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain experiments for Likelihood Computation:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    int ConfigFileInterpreter::getNForLFNS() {
        try {
            std::string N_str = _reader.getEntry("LFNS.N");
            return stoi(N_str);
        }
        catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain N for LFNS:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    int ConfigFileInterpreter::getRForLFNS() {
        try {
            std::string r_str = _reader.getEntry("LFNS.r");
            return stoi(r_str);
        }
        catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain r for LFNS:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    int ConfigFileInterpreter::getHForLFNS() {
        try {
            std::string r_str = _reader.getEntry("LFNS.H");
            return stoi(r_str);
        }
        catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain H for LFNS:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    int ConfigFileInterpreter::getHForEvaluateLikelihood() {
        try {
            std::string r_str = _reader.getEntry("ComputeLikelihood.H");
            return stoi(r_str);
        }
        catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain H for ComputeLikelihood:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    };


    int ConfigFileInterpreter::getDPGMMItForLFNS() {
        try {
            std::string dpgmm_str = _reader.getEntry("LFNS.dpgmmiterations");
            return stoi(dpgmm_str);
        }
        catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain number of DPGMM iterations for LFNS:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    double ConfigFileInterpreter::getEpsilonForLFNS() {
        try {
            std::string dpgmm_str = _reader.getEntry("LFNS.epsilon");
            return std::stod(dpgmm_str);
        }
        catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to obtain epsilon for LFNS:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }


    std::vector<double> ConfigFileInterpreter::getPulsePeriods(std::string experiment_name) {
        std::vector<double> periods;
        std::vector<std::string> periods_str = _getInputValues(experiment_name, "period");
        for (std::string period: periods_str) { periods.push_back(std::stod(period)); }
        return periods;
    }

    std::vector<double> ConfigFileInterpreter::getPulseStrengths(std::string experiment_name) {
        std::vector<double> strengths;
        std::vector<std::string> strength_str = _getInputValues(experiment_name, "strength");
        for (std::string period: strength_str) { strengths.push_back(std::stod(period)); }
        return strengths;
    }

    std::vector<double> ConfigFileInterpreter::getPulseDurations(std::string experiment_name) {
        std::vector<double> durations;
        std::vector<std::string> duration_str = _getInputValues(experiment_name, "duration");
        for (std::string period: duration_str) { durations.push_back(std::stod(period)); }
        return durations;
    }

    std::vector<int> ConfigFileInterpreter::getNumPulse(std::string experiment_name) {
        std::vector<int> num_pulses;
        std::vector<std::string> num_pulses_str = _getInputValues(experiment_name, "numpulses");
        for (std::string period: num_pulses_str) { num_pulses.push_back(std::stod(period)); }
        return num_pulses;
    }

    std::vector<std::string> ConfigFileInterpreter::getPulseInputNames(std::string experiment_name) {
        std::vector<std::string> input_names = _getInputValues(experiment_name, "inputparam");
        return input_names;
    }

    std::vector<double> ConfigFileInterpreter::getStartingTimes(std::string experiment_name) {
        std::vector<double> starting_times;
        std::vector<std::string> duration_str = _getInputValues(experiment_name, "startingtime");
        for (std::string period: duration_str) { starting_times.push_back(std::stod(period)); }
        return starting_times;
    }

    std::vector<std::string>
    ConfigFileInterpreter::_getInputValues(std::string experiment_name, std::string field_name) {
        std::vector<std::string> entries;
        std::vector<XmlMap> input_entries;
        try {
            input_entries = _reader.getEntryMaps("inputs", "input");
        } catch (const std::exception &e) { return entries; }

        for (XmlMap input_entry: input_entries) {
            std::string param_name_str = input_entry["experiments"].entry;
            std::vector<std::string> experiment_names = base::Utils::StringToStringVector(param_name_str);
            int num_found_experiment = 0;
            for (std::string &experiment : experiment_names) {
                if (experiment_name.compare(experiment) == 0) {
                    if (num_found_experiment++ > 1) {
                        std::stringstream ss;
                        ss << "repeated entries for experiment " << experiment
                           << " found in 'experiments' found for input provider." << std::endl;
                        throw std::runtime_error(ss.str());
                    }
                    XmlPropertyMap input_map(input_entry);
                    try {
                        std::string period = input_map.getValueForKey("experiments", experiment, field_name);
                        entries.push_back(period);
                    } catch (const std::exception &e) {
                        std::stringstream ss;
                        ss << "Failed to obtain input value for experiment " << experiment << ":\n\t" << e.what()
                           << std::endl;
                        throw std::runtime_error(ss.str());
                    }
                }
            }
        }
        return entries;
    }

    std::vector<double> ConfigFileInterpreter::getParamForSimulation() {
        std::vector<double> params = {};
        try {
            std::string param_string = _reader.getEntry("Simulation.parameter");
            params = base::Utils::StringToDoubleVector(param_string);
            return params;
        }
        catch (const std::exception &e) { return params; }
    }

    std::vector<double> ConfigFileInterpreter::getParamForEvaluateLikelihood() {
        std::vector<double> params = {};
        try {
            std::string param_string = _reader.getEntry("ComputeLikelihood.parameter");
            params = base::Utils::StringToDoubleVector(param_string);
            return params;
        }
        catch (const std::exception &e) { return params; }
    }

    int ConfigFileInterpreter::getNForSimulation() {
        int n = -1;
        try {
            std::string n_string = _reader.getEntry("Simulation.num_simulations");
            n = std::stoi(n_string);
            return n;
        }
        catch (const std::exception &e) { return n; }
    }

    std::string ConfigFileInterpreter::getParameterFileforSimulation() {
        std::string param_file = "";
        try {
            param_file = _reader.getEntry("Simulation.parameter_file");
            return param_file;
        }
        catch (const std::exception &e) { return param_file; }
    }

    std::string ConfigFileInterpreter::getParameterFileforEvaluateLikelihood() {
        std::string param_file = "";
        try {
            param_file = _reader.getEntry("ComputeLikelihood.parameter_file");
            return param_file;
        }
        catch (const std::exception &e) { return param_file; }

    }

    double ConfigFileInterpreter::getInitialTimeForSimulation() {
        std::string n_string = _reader.getEntry("Simulation.initialtime");
        int n = std::stoi(n_string);
        return n;
    }

    double ConfigFileInterpreter::getFinalTimeForSimulation() {
        std::string final_time_str = _reader.getEntry("Simulation.finaltime");
        double final_time = std::stod(final_time_str);
        return final_time;
    }

    double ConfigFileInterpreter::getIntervalForSimulation() {
        std::string interval_str = _reader.getEntry("Simulation.interval");
        double interval = std::stod(interval_str);
        return interval;
    }
}