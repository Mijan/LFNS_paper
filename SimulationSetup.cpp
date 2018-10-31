//
// Created by jan on 29/10/18.
//

#include "SimulationSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/simulator/SimulatorOde.h"

void SimulationSetup::setUp(options::SimulationOptions &options) {
    readSettingsfromFile(options);

    rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
    for (std::string experiment : experiments) {

        models::FullModel_ptr full_model = std::make_shared<models::FullModel>(model_settings, rng);
        full_models.push_back(full_model);

        simulator::Simulator_ptr simulator = createSimulator(rng, full_model->dynamics, simulation_settings);
        simulator->setDiscontTimes(full_model->getDiscontTimes());
        simulators.push_back(simulator);
    }

    createParameterVector();
    createOutputTimes();
}

void SimulationSetup::readSettingsfromFile(options::SimulationOptions &options) {

    io_settings.config_file = options.config_file_name;
    io_settings.output_file = options.output_file_name;
    io::ConfigFileInterpreter interpreter(io_settings.config_file);


    experiments = interpreter.getExperimentsForSimulations();
    model_settings = _readModelSettings(interpreter, experiments);
    simulation_settings = _setSimulationSettings(interpreter, options);
    _setInputData(model_settings, interpreter);

}

simulator::Simulator_ptr
SimulationSetup::createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
                                 simulator::SimulationSettings &settings) {
    simulator::Simulator_ptr sim_ptr;
    if (settings.model_type == simulator::MODEL_TYPE::STOCH) {
        simulator::SimulatorSsa simulator_ssa(rng, dynamics->getPropensityFct(), dynamics->getReactionFct(),
                                              dynamics->getNumReactions());
        sim_ptr = std::make_shared<simulator::SimulatorSsa>(simulator_ssa);
    } else {
        simulator::OdeSettings ode_settings;
        simulator::SimulatorOde simulator_ode(ode_settings, dynamics->getRhsFct(), dynamics->getNumSpecies());
        simulator::SimulatorOde_ptr simulator_ode_ptr = std::make_shared<simulator::SimulatorOde>(simulator_ode);
        sim_ptr = simulator_ode_ptr;
    }
    return sim_ptr;
}

void SimulationSetup::createOutputTimes() {
    double t = initial_time;
    while (t <= final_time) {
        times.push_back(t);
        t += interval;
    }

}

void SimulationSetup::createParameterVector() {
    if (!parameter.empty()) {
        parameters.push_back(parameter);
    } else {
        parameters = base::IoUtils::readVectorOfVectors(parameter_file);
    }
}


void SimulationSetup::printSettings(std::ostream &os) {
    io_settings.print(os);
    simulation_settings.print(os);

    os << "\n---------- Model Settings ----------" << std::endl;
    model_settings.print(os);

    if (!parameter_file.empty()) {
        os << "Parameters will be read from " << parameter_file << std::endl;
    } else {
        os << "\nParameters for simulation:" << std::endl;
    }


    std::size_t max_name_length = 0;
    for (std::string &param_name : simulation_settings.param_names) {
        if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
    }
    std::size_t max_value_length = 16 + 8;

    os << std::setw(max_name_length) << "Name ";
    if (parameter_file.empty()) { os << std::setw(max_value_length) << "simulation value"; }
    os << std::endl;

    int index = 0;
    for (std::string &param_name :  simulation_settings.param_names) {
        if (parameter_file.empty()) {
            os << std::setw(max_name_length) << param_name;
            std::stringstream value_str;
            if (parameter_file.empty()) {
                value_str << parameter[index++];
                os << std::setw(max_value_length) << value_str.str();
            }
        }
        os << std::endl;
    }
    os << std::endl;


    full_models.back()->printInfo(os);
}


models::ModelSettings
SimulationSetup::_readModelSettings(io::ConfigFileInterpreter &interpreter, std::vector<std::string> experiments) {
    models::ModelSettings model_settings;
    model_settings.model_file = interpreter.getModelFileName();
    model_settings.initial_value_file = interpreter.getInitialConditionsFile();
    model_settings.measurement_file = interpreter.getMeasurementModelFile();

    models::ModelReactionData model_data(model_settings.model_file);
    models::InitialValueData init_data(model_settings.initial_value_file);
    models::MeasurementModelData measure_data(model_settings.measurement_file);

    std::vector<std::string> param_names = model_data.getParameterNames();
    base::Utils::addOnlyNew<std::string>(param_names, init_data.getParameterNames());
    base::Utils::addOnlyNew<std::string>(param_names, measure_data.getParameterNames());
    model_settings.param_names = param_names;

    model_settings.fixed_parameters = interpreter.getFixedParameters();
    return model_settings;
}

void SimulationSetup::_setInputData(models::ModelSettings &model_settings, io::ConfigFileInterpreter &interpreter) {
    if (experiments.empty()) {
        std::cout
                << "No experiment names for Simulation algorithm provided, only basic model without inputs will be simulated!"
                << std::endl;
    } else {
        std::map<std::string, models::InputData> input_datas;
        for (std::string &experiment : experiments) {
            std::vector<double> periods = interpreter.getPulsePeriods(experiment);
            if (!periods.empty()) {
                std::vector<double> strength = interpreter.getPulseStrengths(experiment);
                std::vector<double> duration = interpreter.getPulseDurations(experiment);
                std::vector<int> num_pulses = interpreter.getNumPulse(experiment);
                std::vector<std::string> names = interpreter.getPulseInputNames(experiment);
                std::vector<double> starting_times = interpreter.getStartingTimes(experiment);

                std::vector<models::InputData> datas;
                for (int i = 0; i < periods.size(); i++) {
                    if (starting_times[i] > final_time) {
                        std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                                  << starting_times[i] << " is after the final simulation time "
                                  << final_time
                                  << ", thus perturbation for experiment " << experiment << " on the parameter "
                                  << names[i] << " starting at " << starting_times[i] << " will be ignored!"
                                  << std::endl;
                    } else {
                        int max_num_pulses = std::min(num_pulses[i],
                                                      (int) ((final_time - starting_times[i]) /
                                                             (periods[i] + 1)));
                        datas.push_back(
                                models::InputData(periods[i], strength[i], duration[i], max_num_pulses, names[i],
                                                  starting_times[i]));
                    }
                }
                model_settings.input_datas[experiment] = datas;
            }
        }
    }
}

simulator::SimulationSettings
SimulationSetup::_setSimulationSettings(io::ConfigFileInterpreter &interpreter, options::SimulationOptions &options) {

    simulator::SimulationSettings simu_settings;
    std::string model_type = interpreter.getModelType();
    if (simulator::MODEL_TYPE_NAME.count(model_type) == 0) {
        std::stringstream os;
        os << "Modeltype " << model_type << " not known. Possible options are: ";
        std::map<std::string, simulator::MODEL_TYPE>::iterator it = simulator::MODEL_TYPE_NAME.begin();
        for (it; it != simulator::MODEL_TYPE_NAME.end(); it++) { os << it->first << ", "; }
        throw std::runtime_error(os.str());
    }
    simu_settings.model_type = simulator::MODEL_TYPE_NAME[model_type];
    simu_settings.param_names = model_settings.getUnfixedParameters();


    if (options.vm.count("parameter") > 0) { parameter = options.params; }
    else if (options.vm.count("parameter-file") > 0) { parameter_file = options.param_file; }
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
        if (parameter.size() != simu_settings.param_names.size()) {
            std::stringstream ss;
            ss << "Number of simulation parameters are not consistent. Number of parameters for simulation:"
               << simu_settings.param_names.size();
            ss << ", but " << parameter.size() << " parameters for simulation provided!"
               << std::endl;
            ss << "Parameters for simulation: " << std::endl;
            for (std::string param_name: simu_settings.param_names) { ss << param_name << std::endl; }
            ss << "Additional fixed model parameters: " << std::endl;
            std::map<std::string, double>::iterator it;
            for (it = model_settings.fixed_parameters.begin(); it != model_settings.fixed_parameters.end(); it++) {
                ss << it->first << ":\t" << it->second << std::endl;
            }
            throw std::runtime_error(ss.str());
        }
    }

    if (options.num_simulations > 0) { number_simulations = options.num_simulations; }
    else {
        int n = interpreter.getNForSimulation();
        if (n > 0) { number_simulations = n; }
        else {
            std::cerr
                    << "No number of simulations (either with -n through command line or 'Simulation.num_simulations') provided, assume default value of "
                    << number_simulations;
        }
    }

    if (options.initialTimeProvided()) {
        initial_time = options.initial_time;
        final_time = options.final_time;
        interval = options.interval;
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
    return simu_settings;
}

