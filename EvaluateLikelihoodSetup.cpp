//
// Created by jan on 31/10/18.
//

#include "EvaluateLikelihoodSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/simulator/SimulatorSsa.h"

void EvaluateLikelihoodSetup::setUp(options::ComputeLikelihoodOptions &options) {
    readSettingsfromFile(options);

    rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
    for (std::string experiment : experiments) {

        models::FullModel_ptr full_model = std::make_shared<models::FullModel>(model_settings, rng);
        full_models.push_back(full_model);


        times_vec.push_back(createDataTimes(experiment, particle_filter_settings));
        TrajectorySet data = createData(full_model->measurement_model->getNumMeasurements(), experiment,
                                        particle_filter_settings);
        data_vec.push_back(data);

        simulator::Simulator_ptr simulator = createSimulator(rng, full_model->dynamics, simulation_settings);
        simulator->setDiscontTimes(full_model->getDiscontTimes());
        simulators.push_back(simulator);

        particle_filter::ParticleFilter part_filter(rng, full_models.back()->getParameterSettingFct(),
                                                    simulator->getSimulationFct(), simulator->getResetFct(),
                                                    full_model->measurement_model->getLikelihoodFct(),
                                                    full_model->initial_value_provider->getInitialStateFct(),
                                                    full_model->dynamics->getNumSpecies(), particle_filter_settings.H);
        particle_filters.push_back(std::make_shared<particle_filter::ParticleFilter>(part_filter));

        for (int traj_nbr = 0; traj_nbr < data.size(); traj_nbr++) {
            mult_like_eval.addLogLikeFun(
                    particle_filters.back()->getLikelihoodEvaluationForData(&data_vec.back()[traj_nbr],
                                                                            &times_vec.back()));
        }
    }
    createParameterVector();
}

void EvaluateLikelihoodSetup::readSettingsfromFile(options::ComputeLikelihoodOptions &options) {

    io_settings.config_file = options.config_file_name;
    io_settings.output_file = options.output_file_name;
    io::ConfigFileInterpreter interpreter(io_settings.config_file);


    experiments = interpreter.getExperimentsForEvaluateLikelihood();
    _readModelSettings(interpreter, experiments);
    _readParticleFilterSettings(interpreter, options);


    std::string model_type = interpreter.getModelType();
    if (simulator::MODEL_TYPE_NAME.count(model_type) == 0) {
        std::stringstream os;
        os << "Modeltype " << model_type << " not known. Possible options are: ";
        std::map<std::string, simulator::MODEL_TYPE>::iterator it = simulator::MODEL_TYPE_NAME.begin();
        for (it; it != simulator::MODEL_TYPE_NAME.end(); it++) { os << it->first << ", "; }
        throw std::runtime_error(os.str());
    }
    simulation_settings.model_type = simulator::MODEL_TYPE_NAME[model_type];

    _setInputData(model_settings, interpreter);
}

TrajectorySet EvaluateLikelihoodSetup::createData(int num_outputs, std::string experiment,
                                                  particle_filter::ParticleFilterSettings &settings) {
    if (settings.data_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No data file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    TrajectorySet data = base::IoUtils::readMultiline(settings.data_files[experiment], num_outputs,
                                                      settings.num_used_trajectories);
    return data;
}

Times
EvaluateLikelihoodSetup::createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.time_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No time file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    std::string input_times_file_name = settings.time_files[experiment];
    Times times = base::IoUtils::readVector(input_times_file_name);
    return times;
}


simulator::Simulator_ptr
EvaluateLikelihoodSetup::createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
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


void EvaluateLikelihoodSetup::createParameterVector() {
    if (!parameter.empty()) {
        parameters.push_back(parameter);
    } else if (!parameter_file.empty()) {
        parameters = base::IoUtils::readVectorOfVectors(parameter_file);
    } else {
        throw std::runtime_error(
                "Failed to create parameter vector for likelihood computation, neither parameter nor parameter file was provided!");
    }
}

void EvaluateLikelihoodSetup::printSettings(std::ostream &os) {
    io_settings.print(os);
    particle_filter_settings.print(os);
    if (simulation_settings.model_type == simulator::MODEL_TYPE::STOCH) {
        os << "A SSA simulator will be used!\n" << std::endl;
    } else {
        os << "An ODE simulator will be used\n" << std::endl;
    }
    model_settings.print(os);

    if (!parameter_file.empty()) {
        os << "Parameters will be read from " << parameter_file << std::endl;
    } else {
        os << "\nParameters for simulation:" << std::endl;
    }


    std::size_t max_name_length = 0;
    for (std::string &param_name : particle_filter_settings.param_names) {
        if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
    }
    std::size_t max_value_length = 16 + 8;

    os << std::setw(max_name_length) << "Name ";
    if (parameter_file.empty()) { os << std::setw(max_value_length) << "parameter value"; }
    os << std::endl;

    int index = 0;
    for (std::string &param_name :  particle_filter_settings.param_names) {
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


void EvaluateLikelihoodSetup::_readModelSettings(io::ConfigFileInterpreter &interpreter,
                                                 std::vector<std::string> experiments) {
    model_settings.model_file = interpreter.getModelFileName();
    model_settings.initial_value_file = interpreter.getInitialConditionsFile();
    model_settings.measurement_file = interpreter.getMeasurementModelFile();

    models::ModelReactionData model_data(model_settings.model_file);
    models::InitialValueData init_data(model_settings.initial_value_file);
    models::MeasurementModelData measure_data(model_settings.measurement_file);

    std::vector<std::__cxx11::string> param_names = model_data.getParameterNames();
    base::Utils::addOnlyNew<std::string>(param_names, init_data.getParameterNames());
    base::Utils::addOnlyNew<std::string>(param_names, measure_data.getParameterNames());
    model_settings.param_names = param_names;

    model_settings.fixed_parameters = interpreter.getFixedParameters();


}


void EvaluateLikelihoodSetup::_readParticleFilterSettings(io::ConfigFileInterpreter &interpreter,
                                                          options::ComputeLikelihoodOptions &options) {
    try {
        experiments = interpreter.getExperimentsForEvaluateLikelihood();
    } catch (const std::exception &e) {}

    particle_filter_settings.data_files = interpreter.getDataFiles();
    particle_filter_settings.time_files = interpreter.getTimesFiles();
    particle_filter_settings.H = interpreter.getHForEvaluateLikelihood();


    particle_filter_settings.param_names = model_settings.getUnfixedParameters();


    if (options.vm.count("parameter") > 0) { parameter = options.params; }
    else if (options.vm.count("parameter-file") > 0) { parameter_file = options.param_file; }
    else {
        parameter_file = interpreter.getParameterFileforEvaluateLikelihood();
        parameter = interpreter.getParamForEvaluateLikelihood();
        if (parameter.size() > 0 && parameter_file.size() > 0) {
            std::stringstream ss;
            ss << "In config file only a parameter vector or a parameter file can be provided, but not both!"
               << std::endl;
            throw std::runtime_error(ss.str());
        }
        if (parameter.size() == 0 && parameter_file.size() == 0) {
            std::stringstream ss;
            ss
                    << "For Likelihood computation either a parameter vector (either with -p through the command line or 'ComputeLikelihood.parameter' through config file) or a file with parameters either with -P through the command line or 'ComputeLikelihood.parameter_file' through config file) needs to be provided"
                    << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    if (!parameter.empty()) {
        if (parameter.size() != particle_filter_settings.param_names.size()) {
            std::stringstream ss;
            ss
                    << "Number of parameters for likelihood computation are not consistent. Number of parameters for likelihood computation: "
                    << particle_filter_settings.param_names.size();
            ss << ", but " << parameter.size() << " parameters for likelihood computation provided!" << std::endl;
            ss << "Parameters for likelihood computation: " << std::endl;

            for (std::string param_name: particle_filter_settings.param_names) { ss << param_name << " " << std::endl; }
            ss << "Additional fixed model parameters: " << std::endl;
            std::map<std::string, double>::iterator it;
            for (it = model_settings.fixed_parameters.begin(); it != model_settings.fixed_parameters.end(); it++) {
                ss << it->first << ":\t" << it->second << std::endl;
            }
            throw std::runtime_error(ss.str());
        }
    }


    if (options.vm.count("smcparticles") > 0) { particle_filter_settings.H = options.H; }
    if (options.vm.count("numuseddata") > 0) { particle_filter_settings.num_used_trajectories = options.num_used_data; }
    if (options.vm.count("numcomputations") > 0) { num_computations = options.num_computations; }
}

void
EvaluateLikelihoodSetup::_setInputData(models::ModelSettings &model_settings, io::ConfigFileInterpreter &interpreter) {
    if (experiments.empty()) {
        std::stringstream ss;
        ss
                << "No experiment names for likelihood computation algorithm provided, but experiment name is needed to mach model to data"
                << std::endl;
        throw std::runtime_error(ss.str());
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
                std::vector<double> times = base::IoUtils::readVector(particle_filter_settings.time_files[experiment]);
                double final_time = times.back();
                for (int i = 0; i < periods.size(); i++) {
                    if (starting_times[i] > final_time) {
                        std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                                  << starting_times[i] << " is after the last data time point time "
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