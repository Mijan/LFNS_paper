//
// Created by jan on 29/10/18.
//

#include "SimulationSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/simulator/SimulatorOde.h"


SimulationSetup::SimulationSetup(options::SimulationOptions &options) : GeneralSetup(options), _sim_options(options) {}

SimulationSetup::~SimulationSetup() {}

void SimulationSetup::setUp() {
    _readSettingsfromFile();

    rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
    for (std::string experiment : experiments) {

        models::FullModel_ptr full_model = std::make_shared<models::FullModel>(model_settings, rng, experiment);
        full_models.push_back(full_model);

        simulator::Simulator_ptr simulator = _createSimulator(full_model->dynamics);
        simulator->setDiscontTimes(full_model->getDiscontTimes());
        simulators.push_back(simulator);
    }

    _createParameterVector();
    _createOutputTimes();
}

void SimulationSetup::_readSettingsfromFile() {
    GeneralSetup::_readSettingsfromFile();

    _readSimulationSettings();
    for (std::string experiment : experiments) {
        std::vector<models::InputData> datas = _getInputDatasForExperiment(experiment, final_time);
        if (!datas.empty()) { model_settings.input_datas[experiment] = datas; }
    }
    if (model_settings.input_datas.size() < experiments.size() - 1) {
        std::cerr << "Warning: " << experiments.size() << " different experiments provided, but only "
                  << model_settings.input_datas.size()
                  << " different inputs provided. Experiments may be indistinguishible from one another!" << std::endl;
    }

}

std::vector<std::string> SimulationSetup::_readExperiments() {
    try {
        return interpreter.getExperimentsForSimulations();
    } catch (const std::exception &e) {
        std::cerr << "Failed to read experiments for simulation:\n\t" << e.what() << std::endl;
        std::cerr << "Dummy experiment '0' will be simulated." << std::endl;
        return {"0"};
    }
}

void SimulationSetup::printSettings(std::ostream &os) {
    GeneralSetup::printSettings(os);

    os << "\n---------- Simulation Settings ----------" << std::endl;
    if (!parameter_file.empty()) {
        os << "Parameters will be read from " << parameter_file << std::endl;
    } else {
        os << "\nParameters for simulation:" << std::endl;
    }


    std::size_t max_name_length = 0;
    for (std::string &param_name : sim_param_names) {
        if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
    }
    std::size_t max_value_length = 16 + 8;

    os << std::setw(max_name_length) << "Name ";
    if (parameter_file.empty()) { os << std::setw(max_value_length) << "simulation value"; }
    os << std::endl;

    int index = 0;
    for (std::string &param_name :  sim_param_names) {
        os << std::setw(max_name_length) << param_name;
        std::stringstream value_str;
        if (parameter_file.empty()) {
            value_str << parameter[index++];
            os << std::setw(max_value_length) << value_str.str();
        }
        os << std::endl;
    }
    os << std::endl;


    os << "Experiments for simulation: ";
    for (std::string &experiment: experiments) { os << experiment << ", "; }
    os << std::endl;


    os << "\n---------- Model Settings ----------" << std::endl;
    model_settings.print(os);
    os << std::endl;
    if (full_models.empty()) { std::cerr << "No models created!!" << std::endl; }
    else { full_models.back()->printInfo(os); }
}

void SimulationSetup::_readSimulationSettings() {
    sim_param_names = model_settings.getUnfixedParameters();

    if (_sim_options.vm.count("parameter") > 0) { parameter = _sim_options.params; }
    else if (_sim_options.vm.count("parameter-file") > 0) { parameter_file = _sim_options.param_file; }
    else {
        parameter_file = interpreter.getParameterFileforSimulation();
        parameter = interpreter.getParamForSimulation();
        if (parameter.size() > 0 && parameter_file.size() > 0) {
            std::stringstream ss;
            ss << "In config file only a parameter vector or a parameter file can be provided, but not both!"
               << std::endl;
            throw std::runtime_error(ss.str());
        }
        if (parameter.size() == 0 && parameter_file.size() == 0) {
            std::stringstream ss;
            ss
                    << "For simulation either a parameter vector (either with -p through the command line or 'Simulation.parameter' through config file) or a file with parameters either with -P through the command line or 'Simulation.parameter_file' through config file) needs to be provided"
                    << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    if (!parameter.empty()) {
        if (parameter.size() != sim_param_names.size()) {
            std::stringstream ss;
            ss << "Number of simulation parameters are not consistent. Number of parameters for simulation:"
               << sim_param_names.size();
            ss << ", but " << parameter.size() << " parameters for simulation provided!"
               << std::endl;
            ss << "Parameters for simulation: " << std::endl;
            for (std::string param_name: sim_param_names) { ss << param_name << std::endl; }
            ss << "Additional fixed model parameters: " << std::endl;
            std::map<std::string, double>::iterator it;
            for (it = model_settings.fixed_parameters.begin(); it != model_settings.fixed_parameters.end(); it++) {
                ss << it->first << ":\t" << it->second << std::endl;
            }
            throw std::runtime_error(ss.str());
        }
    }

    if (_sim_options.num_simulations > 0) { number_simulations = _sim_options.num_simulations; }
    else {
        int n = interpreter.getNForSimulation();
        if (n > 0) { number_simulations = n; }
        else {
            std::cerr
                    << "No number of simulations (either with -n through command line or 'Simulation.num_simulations') provided, assume default value of "
                    << number_simulations << std::endl;
        }
    }

    if (_sim_options.initialTimeProvided()) {
        initial_time = _sim_options.initial_time;
        final_time = _sim_options.final_time;
        interval = _sim_options.interval;
    } else if (_sim_options.timepointsProvided()) {
        times = _sim_options.time_points;
    } else {
        try { initial_time = interpreter.getInitialTimeForSimulation(); } catch (
                const std::exception &e) {
            std::cerr
                    << "Failed to read initial time for simulation (either provided in command line with -I or in the config file with 'Simulation.initialtime'. Assume default value of "
                    << initial_time << std::endl;
        }
        try { final_time = interpreter.getFinalTimeForSimulation(); } catch (
                const std::exception &e) {
            std::cerr
                    << "Failed to read final time for simulation (either provided in command line with -F or in the config file with 'Simulation.finaltime'. Assume default value of "
                    << final_time << std::endl;
        }
        try { interval = interpreter.getIntervalForSimulation(); } catch (const std::exception &e) {
            std::cerr
                    << "Failed to read interval for for simulation readout (either provided in command line with -i or in the config file with 'Simulation.interval'. Assume default value of "
                    << interval << std::endl;
        }
    }
}


void SimulationSetup::_createOutputTimes() {
    if (times.empty()) {
        double t = initial_time;
        while (t <= final_time) {
            times.push_back(t);
            t += interval;
        }
    }
}

void SimulationSetup::_createParameterVector() {
    if (!parameter.empty()) {
        parameters.push_back(parameter);
    } else {
        parameters = base::IoUtils::readVectorOfVectors(parameter_file);
    }
}
