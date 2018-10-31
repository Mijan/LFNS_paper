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
    for (std::string experiment : simulation_settings.experiments_for_simulation) {

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

    model_settings.model_file = interpreter.getModelFileName();
    model_settings.initial_value_file = interpreter.getInitialConditionsFile();
    model_settings.measurement_file = interpreter.getMeasurementModelFile();

    std::string model_type = interpreter.getModelType();
    if (simulator::MODEL_TYPE_NAME.count(model_type) == 0) {
        std::stringstream os;
        os << "Modeltype " << model_type << " not known. Possible options are: ";
        std::map<std::string, simulator::MODEL_TYPE>::iterator it = simulator::MODEL_TYPE_NAME.begin();
        for (it; it != simulator::MODEL_TYPE_NAME.end(); it++) { os << it->first << ", "; }
        throw std::runtime_error(os.str());
    }
    simulation_settings.model_type = simulator::MODEL_TYPE_NAME[model_type];

    models::ModelReactionData model_data(model_settings.model_file);
    models::InitialValueData init_data(model_settings.initial_value_file);
    models::MeasurementModelData measure_data(model_settings.measurement_file);

    std::vector<std::string> param_names = model_data.getParameterNames();
    base::Utils::addOnlyNew<std::string>(param_names, init_data.getParameterNames());
    base::Utils::addOnlyNew<std::string>(param_names, measure_data.getParameterNames());
    model_settings.param_names = param_names;

    model_settings.fixed_parameters = interpreter.getFixedParameters();
    simulation_settings.experiments_for_simulation = interpreter.getExperimentsForSimulations();
    if (simulation_settings.experiments_for_simulation.empty()) {
        std::cout
                << "No experiment names for Simulation algorithm provided, only basic model without inputs will be simulated!";
    } else {
        std::map<std::string, models::InputData> input_datas;
        for (std::string &experiment : simulation_settings.experiments_for_simulation) {
            std::vector<double> periods = interpreter.getPulsePeriods(experiment);
            if (!periods.empty()) {
                std::vector<double> strength = interpreter.getPulseStrengths(experiment);
                std::vector<double> duration = interpreter.getPulseDurations(experiment);
                std::vector<int> num_pulses = interpreter.getNumPulse(experiment);
                std::vector<std::string> names = interpreter.getPulseInputNames(experiment);
                std::vector<double> starting_times = interpreter.getStartingTimes(experiment);

                std::vector<models::InputData> datas;
                for (int i = 0; i < periods.size(); i++) {
                    if (starting_times[i] > simulation_settings.final_time) {
                        std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                                  << starting_times[i] << " is after the final simulation time "
                                  << simulation_settings.final_time
                                  << ", thus perturbation for experiment " << experiment << " on the parameter "
                                  << names[i] << " starting at " << starting_times[i] << " will be ignored!"
                                  << std::endl;
                    } else {
                        int max_num_pulses = std::min(num_pulses[i],
                                                      (int) ((simulation_settings.final_time - starting_times[i]) /
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
    simulation_settings.param_names = model_settings.getUnfixedParameters();


    if (options.vm.count("parameter") > 0) { simulation_settings.parameter = options.params; }
    else if (options.vm.count("parameter-file") > 0) { simulation_settings.parameter_file = options.param_file; }
    else {
        simulation_settings.parameter_file = interpreter.getParameterFileforSimulation();
        simulation_settings.parameter = interpreter.getParamForSimulation();
        if (simulation_settings.parameter.size() > 0 && simulation_settings.parameter_file.size() > 0) {
            std::stringstream ss;
            ss << "In config file only a parameter vector or a parameter file can be provided, but not both!"
               << std::endl;
            throw std::runtime_error(ss.str());
        }
        if (simulation_settings.parameter.size() == 0 && simulation_settings.parameter_file.size() == 0) {
            std::stringstream ss;
            ss
                    << "For simulation either a parameter vector (either with -p through the command line or 'Simulation.parameter' through config file) or a file with parameters either with -P through the command line or 'Simulation.parameter_file' through config file) needs to be provided"
                    << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    if (!simulation_settings.parameter.empty()) {
        if (simulation_settings.parameter.size() != simulation_settings.param_names.size()) {
            std::stringstream ss;
            ss << "Number of simulation parameters are not consistent. Number of parameters for simulation:"
               << simulation_settings.param_names.size();
            ss << ", but " << simulation_settings.parameter.size() << " parameters for simulation provided!"
               << std::endl;
            ss << "Parameters for simulation: " << std::endl;
            for (std::string param_name: simulation_settings.param_names) { ss << param_name << std::endl; }
            ss << "Additional fixed model parameters: " << std::endl;
            std::map<std::string, double>::iterator it;
            for (it = model_settings.fixed_parameters.begin(); it != model_settings.fixed_parameters.end(); it++) {
                ss << it->first << ":\t" << it->second << std::endl;
            }
            throw std::runtime_error(ss.str());
        }
    }

    if (options.num_simulations > 0) { simulation_settings.n = options.num_simulations; }
    else {
        int n = interpreter.getNForSimulation();
        if (n > 0) { simulation_settings.n = n; }
        else {
            std::cerr
                    << "No number of simulations (either with -n through command line or 'Simulation.num_simulations') provided, assume default value of "
                    << simulation_settings.n;
        }
    }

    if (options.initialTimeProvided()) {
        simulation_settings.initial_time = options.initial_time;
        simulation_settings.final_time = options.final_time;
        simulation_settings.interval = options.interval;
    } else {
        try { simulation_settings.initial_time = interpreter.getInitialTimeForSimulation(); } catch (
                const std::exception &e) {
            std::cerr
                    << "Failed to read initial time for simulation (either provided in command line with -I or in the config file with 'Simulation.initialtime'. Assume default value of "
                    << simulation_settings.initial_time << std::endl;
        }
        try { simulation_settings.final_time = interpreter.getFinalTimeForSimulation(); } catch (
                const std::exception &e) {
            std::cerr
                    << "Failed to read final time for simulation (either provided in command line with -F or in the config file with 'Simulation.finaltime'. Assume default value of "
                    << simulation_settings.final_time << std::endl;
        }
        try { simulation_settings.interval = interpreter.getIntervalForSimulation(); } catch (const std::exception &e) {
            std::cerr
                    << "Failed to read interval for for simulation readout (either provided in command line with -i or in the config file with 'Simulation.interval'. Assume default value of "
                    << simulation_settings.interval << std::endl;
        }
    }
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
    double t = simulation_settings.initial_time;
    while (t <= simulation_settings.final_time) {
        times.push_back(t);
        t += simulation_settings.interval;
    }

}

void SimulationSetup::createParameterVector() {
    if (!simulation_settings.parameter.empty()) {
        parameters.push_back(simulation_settings.parameter);
    } else {
        parameters = base::IoUtils::readVectorOfVectors(simulation_settings.parameter_file);
    }
}


void SimulationSetup::printSettings(std::ostream &os) {
    io_settings.print(os);
    simulation_settings.print(os);

    os << "---------- Model Settings ----------" << std::endl;
    model_settings.print(os);
    full_models.back()->printInfo(os);
}

