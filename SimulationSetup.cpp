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
    for (std::string experiment : settings.experiments_for_simulation) {

        models::FullModel_ptr full_model = createModel(rng, settings);
        full_models.push_back(full_model);


        simulator::Simulator_ptr simulator = createSimulator(rng, full_model->dynamics, settings);
        simulators.push_back(simulator);

        setUpPerturbations(experiment, simulator, full_model, settings);
    }

    createParameterVector();
    createOutputTimes();
}

void SimulationSetup::readSettingsfromFile(options::SimulationOptions &options) {

    std::string xml_file = options.config_file_name;
    io::ConfigFileInterpreter interpreter(xml_file);

    settings.model_file = interpreter.getModelFileName();
    settings.initial_value_file = interpreter.getInitialConditionsFile();
    settings.measurement_file = interpreter.getMeasurementModelFile();

    std::string model_type = interpreter.getModelType();
    if (simulator::MODEL_TYPE_NAME.count(model_type) == 0) {
        std::stringstream os;
        os << "Modeltype " << model_type << " not known. Possible options are: ";
        std::map<std::string, simulator::MODEL_TYPE>::iterator it = simulator::MODEL_TYPE_NAME.begin();
        for (it; it != simulator::MODEL_TYPE_NAME.end(); it++) { os << it->first << ", "; }
        throw std::runtime_error(os.str());
    }
    settings.model_type = simulator::MODEL_TYPE_NAME[model_type];

    models::ModelReactionData model_data(settings.model_file);
    models::InitialValueData init_data(settings.initial_value_file);
    models::MeasurementModelData measure_data(settings.measurement_file);

    std::vector<std::string> param_names = model_data.getParameterNames();
    base::Utils::addOnlyNew<std::string>(param_names, init_data.getParameterNames());
    base::Utils::addOnlyNew<std::string>(param_names, measure_data.getParameterNames());
    settings.param_names = param_names;

    std::map<std::string, std::pair<double, double>> param_bounds = interpreter.getParameterBounds();
    settings.fixed_parameters = interpreter.getFixedParameters();
    std::map<std::string, std::string> scales = interpreter.getParameterScales();
    settings.experiments_for_simulation = interpreter.getExperimentsForLFNS();
    if (settings.experiments_for_simulation.empty()) {
        throw std::runtime_error(
                "No experiment names for LFNS algorithm provided, please provide at least one experiment name to be used for the inference!");
    }

    std::map<std::string, models::InputData> input_datas;

    for (std::string &experiment : settings.experiments_for_simulation) {
        std::vector<double> periods = interpreter.getPulsePeriods(experiment);
        if (!periods.empty()) {
            std::vector<double> strength = interpreter.getPulseStrengths(experiment);
            std::vector<double> duration = interpreter.getPulseDurations(experiment);
            std::vector<int> num_pulses = interpreter.getNumPulse(experiment);
            std::vector<std::string> names = interpreter.getPulseInputNames(experiment);
            std::vector<double> starting_times = interpreter.getStartingTimes(experiment);

            std::vector<models::InputData> datas;
            for (int i = 0; i < periods.size(); i++) {
                datas.push_back(models::InputData(periods[i], strength[i], duration[i], num_pulses[i], names[i],
                                                  starting_times[i]));
            }
            settings.input_datas[experiment] = datas;
        }
    }


    if (options.vm.count("parameter") > 0) { settings.parameter = options.params; }
    else if (options.vm.count("parameter-file") > 0) { settings.parameter_file = options.param_file; }
    else {
        settings.parameter_file = interpreter.getParameterFileforSimulation();
        settings.parameter = interpreter.getParamForSimulation();
        if (settings.parameter.size() > 0 && settings.parameter_file.size() > 0) {
            std::stringstream ss;
            ss << "In config file only a parameter vector or a parameter file can be provided, but not both!"
               << std::endl;
            throw std::runtime_error(ss.str());
        }
        if (settings.parameter.size() == 0 && settings.parameter_file.size() == 0) {
            std::stringstream ss;
            ss
                    << "For simulation either a parameter vector (either with -p through the command line or 'Simulation.parameter' through config file) or a file with parameters either with -P through the command line or 'Simulation.parameter_file' through config file) needs to be provided"
                    << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    if (!settings.parameter.empty()) {
        if (settings.parameter.size() != settings.param_names.size() - settings.fixed_parameters.size()) {
            std::stringstream ss;
            ss << "Number of simulation parameters are not consistent. Total number of parameters "
               << settings.parameter.size();
            if (!settings.fixed_parameters.empty()) {
                ss << " and " << settings.fixed_parameters.size() << " parameters fixed";
            }
            ss << ", but " << settings.parameter.size() << " parameters for simulation provided!" << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    if (options.num_simulations > 0) { settings.n = options.num_simulations; }
    else {
        int n = interpreter.getNForSimulation();
        if (n > 0) { settings.n = n; }
        else {
            std::cerr
                    << "No number of simulations (either with -n through command line or 'Simulation.num_simulations') provided, assume default value of "
                    << settings.n;
        }
    }

    if (options.initialTimeProvided()) {
        settings.initial_time = options.initial_time;
        settings.final_time = options.final_time;
        settings.interval = options.interval;
    } else {
        try { settings.initial_time = interpreter.getInitialTimeForSimulation(); } catch (const std::exception &e) {
            std::cerr
                    << "Failed to read initial time for simulation (either provided in command line with -I or in the config file with 'Simulation.initialtime'. Assume default value of "
                    << settings.initial_time << std::endl;
        }
        try { settings.final_time = interpreter.getFinalTimeForSimulation(); } catch (const std::exception &e) {
            std::cerr
                    << "Failed to read final time for simulation (either provided in command line with -F or in the config file with 'Simulation.finaltime'. Assume default value of "
                    << settings.final_time << std::endl;
        }
        try { settings.interval = interpreter.getIntervalForSimulation(); } catch (const std::exception &e) {
            std::cerr
                    << "Failed to read interval for for simulation readout (either provided in command line with -i or in the config file with 'Simulation.interval'. Assume default value of "
                    << settings.interval << std::endl;
        }
    }
    settings.output_file = options.output_file_name;

}


models::FullModel_ptr SimulationSetup::createModel(base::RngPtr rng, simulator::SimulationSettings &settings) {

    models::ChemicalReactionNetwork_ptr dynamics = std::make_shared<models::ChemicalReactionNetwork>(
            settings.model_file);
    models::InitialValueData init_data(settings.initial_value_file);
    models::InitialValueProvider_ptr init_value = std::make_shared<models::InitialValueProvider>(rng, init_data);
    models::MeasurementModelData measure_data(settings.measurement_file);
    models::MeasurementModel_ptr measurement = std::make_shared<models::MeasurementModel>(rng, measure_data);
    measurement->setStateOrder(dynamics->getSpeciesNames());

    models::FullModel_ptr full_mode = std::make_shared<models::FullModel>(dynamics, init_value, measurement);

    std::map<std::string, double>::iterator it = settings.fixed_parameters.begin();
    for (it; it != settings.fixed_parameters.end(); it++) {
        full_mode->fixParameter(it->first, it->second);
    }
    return full_mode;
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

void
SimulationSetup::setUpPerturbations(std::string experiment, simulator::Simulator_ptr simulator,
                                    models::FullModel_ptr full_model, simulator::SimulationSettings &settings) {
    if (settings.input_datas.count(experiment) > 0) {
        double max_used_time = settings.final_time;
        for (models::InputData input_data : settings.input_datas[experiment]) {
            if (input_data.starting_time > max_used_time) {
                std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                          << input_data.starting_time << " is after the last data point time " << max_used_time
                          << ", thus perturbation for experiment " << experiment << " on the parameter "
                          << input_data.pulse_inpt_name << " starting at " << input_data.starting_time
                          << " will be ignored!" << std::endl;
            } else {
                models::InputPulse pulse(input_data, max_used_time);
                full_model->addInputPulse(pulse);
            }
        }
        simulator->setDiscontTimes(full_model->getDiscontTimes());
    }
}

void SimulationSetup::createOutputTimes() {
    double t = settings.initial_time;
    while (t <= settings.final_time) {
        times.push_back(t);
        t += settings.interval;
    }

}

void SimulationSetup::createParameterVector() {
    if (!settings.parameter.empty()) {
        parameters.push_back(settings.parameter);
    } else {
        parameters = base::IoUtils::readVectorOfVectors(settings.parameter_file);
    }
}

