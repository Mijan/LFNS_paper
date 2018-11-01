//
// Created by jan on 28/10/18.
//

#include "LFNSSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/LFNS/LFNS.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/simulator/SimulatorSsa.h"


LFNSSetup::LFNSSetup(options::LFNSOptions options) : GeneralSetup(options), _lfns_options(options) {}

LFNSSetup::~LFNSSetup() {}

void LFNSSetup::setUp() {
    _readSettingsfromFile();

    for (std::string experiment : experiments) {

        models::FullModel_ptr full_model = std::make_shared<models::FullModel>(model_settings, rng);
        full_models.push_back(full_model);


        times_vec.push_back(_createDataTimes(experiment, particle_filter_settings));
        TrajectorySet data = _createData(full_model->measurement_model->getNumMeasurements(), experiment,
                                         particle_filter_settings);
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
}

void LFNSSetup::printSettings(std::ostream &os) {
    io_settings.print(os);
    lfns_settings.print(os);
    os << std::endl;
    particle_filter_settings.print(os);
    sampler_settings.print(os);
    full_models.back()->printInfo(os);
}


std::vector<std::string> LFNSSetup::_readExperiments() { return interpreter.getExperimentsForLFNS(); }

void LFNSSetup::_readSettingsfromFile() {
    GeneralSetup::_readSettingsfromFile();
    particle_filter_settings = _readParticleFilterSettings();
    sampler_settings = _readSamplerSettings();

    lfns_settings = _readLFNSSettings();
    for (std::string experiment : experiments) {
        std::vector<double> times = _createDataTimes(experiment, particle_filter_settings);
        model_settings.input_datas[experiment] = _getInputDatasForExperiment(experiment, times.back());
    }
}


particle_filter::ParticleFilterSettings LFNSSetup::_readParticleFilterSettings() {
    particle_filter::ParticleFilterSettings filter_settings;
    filter_settings.data_files = interpreter.getDataFiles();
    filter_settings.time_files = interpreter.getTimesFiles();
    if (_lfns_options.vm.count("smcparticles") > 0) { filter_settings.H = _lfns_options.H; }
    else {
        try {
            filter_settings.H = interpreter.getHForEvaluateLikelihood();
        } catch (const std::exception &e) {
            std::cout
                    << "No number of particles for particle H for particle filter provided (either with -H through the command line or 'ComputeLikelihood.H' in the config file). Assume H = "
                    << filter_settings.H << std::endl;
        }
    }
    if (_lfns_options.vm.count("prematurecancelling") >
        0) { filter_settings.use_premature_cancelation = _lfns_options.use_premature_cancelation; }

    if (_lfns_options.vm.count("numuseddata") >
        0) { particle_filter_settings.num_used_trajectories = _lfns_options.num_used_data; }
    filter_settings.param_names = model_settings.getUnfixedParameters();
    return filter_settings;
}


sampler::SamplerSettings LFNSSetup::_readSamplerSettings() {
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

        if (bounds.count(param) > 0) { sampler_setting.parameter_bounds[param] = bounds[param]; }
        else {
            sampler_setting.parameter_bounds[param] = {1e-5, 100};
            std::cerr << "No bounds for parameter " << param << " provided. Assume defauld bounds of ["
                      << sampler_setting.parameter_bounds[param].first << ", "
                      << sampler_setting.parameter_bounds[param].second << "]" << std::endl;
        }
    }
    return sampler_setting;
}

TrajectorySet
LFNSSetup::_createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.data_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No data file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    TrajectorySet data = base::IoUtils::readMultiline(settings.data_files[experiment], num_outputs,
                                                      settings.num_used_trajectories);
    return data;
}

Times LFNSSetup::_createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.time_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No time file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    std::string input_times_file_name = settings.time_files[experiment];
    Times times = base::IoUtils::readVector(input_times_file_name);
    return times;
}

lfns::LFNSSettings LFNSSetup::_readLFNSSettings() {
    lfns::LFNSSettings lfns_setting;
    lfns_setting.output_file = io_settings.output_file;

    if (_lfns_options.vm.count("LFNSparticles") > 0) { lfns_setting.N = _lfns_options.N; }
    else {
        try { lfns_setting.N = interpreter.getNForLFNS(); }
        catch (const std::runtime_error &e) {
            std::cout
                    << "\tNo number of LFNS particles N provided (either with -N through the command line or 'LFNS.N' in the config file). Assume N = "
                    << lfns_setting.N << std::endl;
        }
    }
    if (_lfns_options.vm.count("numberprallelsamples") > 0) { lfns_setting.r = _lfns_options.r; }
    else {
        try { lfns_setting.r = interpreter.getRForLFNS(); }
        catch (const std::runtime_error &e) {
            std::cout
                    << "\tNo number of paralle samples r for LFNS provided (either with -r through the command line or 'LFNS.r' in the config file). Assume r = "
                    << lfns_setting.r << std::endl;
        }
    }
    if (_lfns_options.vm.count("tolerance") > 0) {
        lfns_setting.log_termination = std::log(_lfns_options.LFNS_tolerance);
    } else {
        try { lfns_setting.log_termination = std::log(interpreter.getEpsilonForLFNS()); }
        catch (const std::runtime_error &e) {
            std::cout
                    << "\tNo number termination threshold epsilon for LFNS provided (either with -t through the command line or 'LFNS.epsilon' in the config file). Assume epsilon = "
                    << std::exp(lfns_setting.log_termination) << std::endl;
        }
    }
    if (_lfns_options.vm.count("previous_pop") >
        0) { lfns_setting.previous_log_file = _lfns_options.previous_population_file; }
    if (_lfns_options.vm.count("printinterval") > 0) { lfns_setting.print_interval = _lfns_options.print_interval; }
    if (_lfns_options.vm.count("rej_quan") >
        0) { lfns_setting.rejection_quantile_for_density_estimation = _lfns_options.rejection_quantile; }
    return lfns_setting;
}
