//
// Created by jan on 01/02/19.
//

#include "MCMCSetup.h"
#include "src/base/IoUtils.h"


MCMCSetup::MCMCSetup(options::MCMCOptions options) : GeneralSetup(options), _mcmc_options(options) {}

MCMCSetup::~MCMCSetup() {}

void MCMCSetup::setUp() {
    _readSettingsfromFile();

    int max_num_traj = 0;
    for (std::string experiment : experiments) {

        models::FullModel_ptr full_model = std::make_shared<models::FullModel>(model_settings, rng, experiment);
        full_models.push_back(full_model);


        times_vec.push_back(_createDataTimes(experiment, particle_filter_settings));
        TrajectorySet data = _createData(full_model->measurement_model->getNumMeasurements(), experiment,
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

    mcmc_settings.output_file = _mcmc_options.output_file_name;
    mcmc_settings.N = _mcmc_options.N;
    mcmc_settings.uniform_prior = true;
    mcmc_settings.previous_pop_file = _mcmc_options.previous_population_file;
}

void MCMCSetup::printSettings(std::ostream &os) {
    GeneralSetup::printSettings(os);
    os << std::endl;

    os << "Experiments for LF-NS:";
    for (std::string &experiment: experiments) { os << experiment << ", "; }
    os << std::endl;

    particle_filter_settings.print(os);
    sampler_settings.print(os);

    os << "\n---------- Model Settings ----------" << std::endl;
    model_settings.print(os);
    os << std::endl;
    full_models.back()->printInfo(os);
}


std::vector<std::string> MCMCSetup::_readExperiments() {
    try {
        return interpreter.getExperimentsForLFNS();
    } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "Failed to read experiments for MCMC:\n\t" << e.what() << std::endl;
        ss << "At least one experiment with corresponding data needs to be provided." << std::endl;
        throw std::runtime_error(ss.str());
    }
}

void MCMCSetup::_readSettingsfromFile() {
    GeneralSetup::_readSettingsfromFile();
    particle_filter_settings = _readParticleFilterSettings();
    sampler_settings = _readSamplerSettings();

    for (std::string experiment : experiments) {
        std::vector<double> times = _createDataTimes(experiment, particle_filter_settings);
        model_settings.input_datas[experiment] = _getInputDatasForExperiment(experiment, times.back());
    }
}


particle_filter::ParticleFilterSettings MCMCSetup::_readParticleFilterSettings() {
    particle_filter::ParticleFilterSettings filter_settings;
    filter_settings.data_files = interpreter.getDataFiles(experiments);
    filter_settings.time_files = interpreter.getTimesFiles(experiments);
    if (_mcmc_options.vm.count("smcparticles") > 0) { filter_settings.H = _mcmc_options.H; }
    else {
        try {
            filter_settings.H = interpreter.getHForEvaluateLikelihood();
        } catch (const std::exception &e) {
            std::cout
                    << "No number of particles for particle H for particle filter provided (either with -H through the command line or 'ComputeLikelihood.H' in the config file). Assume H = "
                    << filter_settings.H << std::endl;
        }
    }
    if (_mcmc_options.vm.count("prematurecancelling") > 0) { filter_settings.use_premature_cancelation = false; }
    if (_mcmc_options.vm.count("numuseddata") >
        0) { filter_settings.num_used_trajectories = _mcmc_options.num_used_data; }
    filter_settings.param_names = model_settings.getUnfixedParameters();
    return filter_settings;
}


sampler::SamplerSettings MCMCSetup::_readSamplerSettings() {
    sampler::SamplerSettings sampler_setting;
    sampler_setting.param_names = model_settings.getUnfixedParameters();

    std::map<std::string, std::pair<double, double> > bounds = interpreter.getParameterBounds();
    std::map<std::string, std::string> log_scale_str = interpreter.getParameterScales();
    for (std::string &param : sampler_setting.param_names) {
        std::string scale = "log";
        if (log_scale_str.count(param) > 0) {
            scale = log_scale_str[param];
            if (scale.compare("lin") != 0 && scale.compare("log") != 0 && scale.compare("linear") != 0) {
                std::stringstream ss;
                ss << "Failed to set scale for parameter " << param
                   << ". Scale needs to be 'lin' or 'log', but provided scale is :" << scale << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        sampler_setting.parameters_log_scale[param] = scale.compare("log") == 0;

        if (bounds.count(param) > 0) {
            if (bounds[param].first <= 0 && sampler_setting.parameters_log_scale[param]) {
                std::cerr << "Lower bound for parameter " << param << " is " << bounds[param].first
                          << ", but log scale is assumed. Instead, linear scale will be assumed!" << std::endl;
                sampler_setting.parameters_log_scale[param] = false;
            }
            sampler_setting.parameter_bounds[param] = bounds[param];
        } else {
            sampler_setting.parameter_bounds[param] = {1e-5, 100};
            std::cerr << "No bounds for parameter " << param << " provided. Assume defauld bounds of ["
                      << sampler_setting.parameter_bounds[param].first << ", "
                      << sampler_setting.parameter_bounds[param].second << "]" << std::endl;
        }
    }
    return sampler_setting;
}

TrajectorySet
MCMCSetup::_createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.data_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No data file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    TrajectorySet data = base::IoUtils::readMultiline(settings.data_files[experiment], num_outputs,
                                                      settings.num_used_trajectories);
    return data;
}

Times MCMCSetup::_createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.time_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No time file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    std::string input_times_file_name = settings.time_files[experiment];
    Times times = base::IoUtils::readVector(input_times_file_name);
    return times;
}