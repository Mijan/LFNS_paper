//
// Created by jan on 31/10/18.
//

#include "LikelihoodSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/simulator/SimulatorSsa.h"


LikelihoodSetup::LikelihoodSetup(options::ComputeLikelihoodOptions options) : GeneralSetup(options),
                                                                              _likelihood_options(options) {}

LikelihoodSetup::~LikelihoodSetup() {}

void LikelihoodSetup::setUp() {
    _readSettingsfromFile();

    int max_num_traj = 0;
    for (std::string experiment : experiments) {

        models::FullModel_ptr full_model = std::make_shared<models::FullModel>(model_settings, rng, experiment);
        full_models.push_back(full_model);


        times_vec.push_back(_createDataTimes(experiment, particle_filter_settings));
        TrajectorySet data = createData(full_model->measurement_model->getNumMeasurements(), experiment,
                                        particle_filter_settings);
        max_num_traj = max_num_traj > data.size() ? max_num_traj : data.size();
        data_vec.push_back(data);

        simulator::Simulator_ptr simulator = _createSimulator(full_model->dynamics);
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
    particle_filter_settings.num_used_trajectories = max_num_traj;
    _createParameterVector();
}

void LikelihoodSetup::_readSettingsfromFile() {
    GeneralSetup::_readSettingsfromFile();
    particle_filter_settings = _readParticleFilterSettings();
    for (std::string experiment : experiments) {
        std::vector<double> times = _createDataTimes(experiment, particle_filter_settings);
        model_settings.input_datas[experiment] = _getInputDatasForExperiment(experiment, times.back());
    }
}

std::vector<std::string>
LikelihoodSetup::_readExperiments() {
    try {
        return interpreter.getExperimentsForEvaluateLikelihood();
    } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "Failed to read experiments for likelihood evaluation:\n\t" << e.what() << std::endl;
        ss << "At least one experiment with corresponding data needs to be provided." << std::endl;
        throw std::runtime_error(ss.str());
    }

}


void LikelihoodSetup::printSettings(std::ostream &os) {
    GeneralSetup::printSettings(os);


    os << "\n---------- Likelihood Computation Settings ----------" << std::endl;
    os << "Number of likelihood computations: " << num_computations << std::endl;
    if (!parameter_file.empty()) {
        os << "Parameters will be read from " << parameter_file << std::endl;
    } else {
        os << "\nParameters for likelihood computation:" << std::endl;
    }
    std::size_t max_name_length = 0;
    for (std::string &param_name : particle_filter_settings.param_names) {
        if (max_name_length < param_name.size()) { max_name_length = param_name.size(); }
    }
    std::size_t max_value_length = 16 + 8;

    os << std::setw(max_name_length) << "Name ";
    if (parameter_file.empty()) { os << std::setw(max_value_length) << "simulation value"; }
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

    os << "Experiments for likelihood computation: ";
    for (std::string &experiment: experiments) { os << experiment << ", "; }
    os << std::endl;


    particle_filter_settings.print(os);


    os << "\n---------- Model Settings ----------" << std::endl;
    model_settings.print(os);
    os << std::endl;
    full_models.back()->printInfo(os);
}

TrajectorySet LikelihoodSetup::createData(int num_outputs, std::string experiment,
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

Times LikelihoodSetup::_createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.time_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No time file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    std::string input_times_file_name = settings.time_files[experiment];
    Times times = base::IoUtils::readVector(input_times_file_name);
    return times;
}

void LikelihoodSetup::_createParameterVector() {
    if (!parameter.empty()) {
        parameters.push_back(parameter);
    } else if (!parameter_file.empty()) {
        parameters = base::IoUtils::readVectorOfVectors(parameter_file);
    } else {
        throw std::runtime_error(
                "Failed to create parameter vector for likelihood computation, neither parameter nor parameter file was provided!");
    }
}

particle_filter::ParticleFilterSettings LikelihoodSetup::_readParticleFilterSettings() {
    particle_filter::ParticleFilterSettings filter_settings;
    filter_settings.data_files = interpreter.getDataFiles(experiments);
    filter_settings.time_files = interpreter.getTimesFiles(experiments);
    filter_settings.param_names = model_settings.getUnfixedParameters();


    if (_likelihood_options.vm.count("parameter") > 0) { parameter = _likelihood_options.params; }
    else if (_likelihood_options.vm.count("parameter-file") > 0) { parameter_file = _likelihood_options.param_file; }
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
        if (parameter.size() != filter_settings.param_names.size()) {
            std::stringstream ss;
            ss
                    << "Number of parameters for likelihood computation are not consistent. Number of parameters for likelihood computation: "
                    << filter_settings.param_names.size();
            ss << ", but " << parameter.size() << " parameters for likelihood computation provided!" << std::endl;
            ss << "Parameters for likelihood computation: " << std::endl;

            for (std::string param_name: filter_settings.param_names) { ss << param_name << " " << std::endl; }
            ss << "Additional fixed model parameters: " << std::endl;
            std::map<std::string, double>::iterator it;
            for (it = model_settings.fixed_parameters.begin(); it != model_settings.fixed_parameters.end(); it++) {
                ss << it->first << ":\t" << it->second << std::endl;
            }
            throw std::runtime_error(ss.str());
        }
    }

    if (_likelihood_options.vm.count("numsmcparticles") > 0) { filter_settings.H = _likelihood_options.H; }
    else {
        try {
            filter_settings.H = interpreter.getHForEvaluateLikelihood();
        } catch (const std::exception &e) {
            std::cerr
                    << "\tNo number of particles for particle H for particle filter provided (either with -H through the command line or 'ComputeLikelihood.H' in the config file). Assume H = "
                    << filter_settings.H << std::endl;
        }
    }
    if (_likelihood_options.vm.count("numuseddata") > 0) {
        filter_settings.num_used_trajectories = _likelihood_options.num_used_data;
    }
    if (_likelihood_options.vm.count("numcomputations") > 0) {
        num_computations = _likelihood_options.num_computations;
    }
    return filter_settings;
}
