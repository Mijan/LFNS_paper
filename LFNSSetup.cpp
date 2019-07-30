//
// Created by jan on 28/10/18.
//

#include "LFNSSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/LFNS/LFNS.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/sampler/DpGmmSampler.h"
#include "src/sampler/RejectionSupportSampler.h"
#include "src/sampler/GaussianSampler.h"
#include "src/sampler/KernelSupportEstimation.h"
#include "src/sampler/SliceSampler.h"
#include "src/sampler/UniformSampler.h"
#include "src/sampler/EllipsoidSampler.h"


LFNSSetup::LFNSSetup(options::LFNSOptions options, int process_nbr) : GeneralSetup(options, process_nbr),
                                                                      _lfns_options(options) {}

LFNSSetup::~LFNSSetup() {}

void LFNSSetup::setUp() {
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
    density_estimation = _createDensityEstimation(lfns_settings, sampler_settings);
    prior = _createPrior(lfns_settings, sampler_settings);
}

void LFNSSetup::printSettings(std::ostream &os) {
    GeneralSetup::printSettings(os);
    lfns_settings.print(os);
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


std::vector<std::string> LFNSSetup::_readExperiments() {
    try {
        return interpreter.getExperimentsForLFNS();
    } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "Failed to read experiments for LFNS:\n\t" << e.what() << std::endl;
        ss << "At least one experiment with corresponding data needs to be provided." << std::endl;
        throw std::runtime_error(ss.str());
    }
}

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
    filter_settings.data_files = interpreter.getDataFiles(experiments);
    filter_settings.time_files = interpreter.getTimesFiles(experiments);
    if (_lfns_options.vm.count("smcparticles") > 0) { filter_settings.H = _lfns_options.H; }
    else {
        try {
            filter_settings.H = interpreter.getHForLFNS();
        } catch (const std::exception &e) {
            std::cout
                    << "No number of particles for particle H for particle filter provided (either with -H through the command line or 'ComputeLikelihood.H' in the config file). Assume H = "
                    << filter_settings.H << std::endl;
        }
    }
    if (_lfns_options.vm.count("prematurecancelling") >
        0) { filter_settings.use_premature_cancelation = _lfns_options.use_premature_cancelation; }

    if (_lfns_options.vm.count("numuseddata") >
        0) { filter_settings.num_used_trajectories = _lfns_options.num_used_data; }
    filter_settings.param_names = model_settings.getUnfixedParameters();
    return filter_settings;
}


sampler::DensityEstimation_ptr
LFNSSetup::_createDensityEstimation(lfns::LFNSSettings lfns_settings, sampler::SamplerSettings settings) {

    sampler::DensityEstimation_ptr density_estimation_ptr(nullptr);

    std::vector<std::string> unfixed_params = settings.param_names;
    sampler::SamplerData sampler_data(unfixed_params.size());
    sampler_data.bounds = settings.getBounds(unfixed_params);

    switch (lfns_settings.estimator) {
        case lfns::REJECT_DPGMM  : {
            sampler::DpGmmSamplerData dpgmm_data(sampler_data);
            dpgmm_data.num_dp_iterations = 50;
            sampler::DpGmmSampler_ptr dpgmm_sampler = std::make_shared<sampler::DpGmmSampler>(rng, dpgmm_data);


            sampler::RejectionSamplerData rej_data(sampler_data);
            rej_data.rejection_quantile = lfns_settings.rejection_quantile_for_density_estimation;
            rej_data.thresh_accept_rate = lfns_settings.thresh_accept_rate;
            rej_data.rejection_quantile_low_accept = lfns_settings.rejection_quantile_low_accept;

            density_estimation_ptr = std::make_shared<sampler::RejectionSupportSampler>(rng, dpgmm_sampler, rej_data);
            break;
        }
        case lfns::KDE_GAUSS: {
            sampler::NormalSamplerData normal_data(sampler_data);
            normal_data.cov = base::EiMatrix::Identity(sampler_data.size(), sampler_data.size()) * 0.1;
            sampler::GaussianSampler_ptr gauss_kernel = std::make_shared<sampler::GaussianSampler>(rng, normal_data);

            density_estimation_ptr = std::make_shared<sampler::KernelSupportEstimation>(rng, gauss_kernel,
                                                                                        sampler_data);
            break;
        }
        case lfns::KDE_UNIFORM: {
            sampler::UniformSamplerData unif_data(sampler_data);
            sampler::KernelSampler_ptr unif_kernel = std::make_shared<sampler::UniformSampler>(rng, unif_data);
            density_estimation_ptr = std::make_shared<sampler::KernelSupportEstimation>(rng, unif_kernel,
                                                                                        sampler_data);
            break;
        }
        case lfns::ELLIPS: {
            density_estimation_ptr = std::make_shared<sampler::EllipsoidSampler>(rng, sampler_data);
            break;
        }
        case lfns::SLICE: {
            sampler::SliceSampler_ptr sampler_ptr = std::make_shared<sampler::SliceSampler>(rng, sampler_data,
                                                                                            mult_like_eval.getLogLikeFun(),
                                                                                            &threshold);
            sampler_ptr->setLogScaleIndices(settings.getLogParams());
            density_estimation_ptr = sampler_ptr;

            break;
        }
    }
    return density_estimation_ptr;
}


sampler::Sampler_ptr LFNSSetup::_createPrior(lfns::LFNSSettings lfns_settings, sampler::SamplerSettings settings) {

    sampler::Sampler_ptr prior_ptr(nullptr);
    std::vector<std::string> unfixed_params = settings.param_names;
    sampler::SamplerData sampler_data(unfixed_params.size());
    sampler_data.bounds = settings.getBounds(unfixed_params);

    if (lfns_settings.uniform_prior) {
        sampler::UniformSamplerData uni_data(sampler_data);
        prior_ptr = std::make_shared<sampler::UniformSampler>(rng, sampler_data);
    } else {
        sampler::DpGmmSamplerData dpgmm_data(sampler_data);
        dpgmm_data.num_dp_iterations = 50;
        sampler::DpGmmSampler_ptr dpgmm_sampler = std::make_shared<sampler::DpGmmSampler>(rng, dpgmm_data);

        lfns::LiveParticleSet particles;
        particles.readFromFile(lfns_settings.prior_file);
        base::EiMatrix matrix = particles.toMatrix();
        for (int &index: settings.getLogParams()) {
            matrix.col(index) = matrix.col(index).array().log10();
        }

        dpgmm_sampler->updateDensitySamples(matrix);
        prior_ptr = dpgmm_sampler;
    }
    return prior_ptr;
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
            std::cerr
                    << "\tNo number termination threshold epsilon for LFNS provided (either with -t through the command line or 'LFNS.epsilon' in the config file). Assume epsilon = "
                    << std::exp(lfns_setting.log_termination) << std::endl;
        }
    }
    if (_lfns_options.vm.count("sampler") > 0) {
        switch (_lfns_options._sampler_index) {
            case 0: {
                lfns_setting.estimator = lfns::DENSITY_ESTIMATOR::REJECT_DPGMM;
                break;
            }
            case 1: {
                lfns_setting.estimator = lfns::DENSITY_ESTIMATOR::KDE_GAUSS;
                break;
            }
            case 2: {
                lfns_setting.estimator = lfns::DENSITY_ESTIMATOR::ELLIPS;
                break;
            }
            case 3: {
                lfns_setting.estimator = lfns::DENSITY_ESTIMATOR::SLICE;
                break;
            }
        }
    }
    if (_lfns_options.vm.count("previous_pop") >
        0) { lfns_setting.previous_log_file = _lfns_options.previous_population_file; }
    if (_lfns_options.vm.count("printinterval") > 0) { lfns_setting.print_interval = _lfns_options.print_interval; }
    if (_lfns_options.vm.count("rej_quan") >
        0) { lfns_setting.rejection_quantile_for_density_estimation = _lfns_options.rejection_quantile; }

    if (_lfns_options.vm.count("acc_thresh") > 0 || _lfns_options.vm.count("rej_quan_lo_accept") > 0) {
        if (_lfns_options.vm.count("acc_thresh") != _lfns_options.vm.count("rej_quan_lo_accept")) {
            std::cerr
                    << "If variable rejection constant for the rejection sampler is set, bot, the acceptance rate threshold as well as the corresponding rejection quantile needs to be provided! Ignoring provided values!"
                    << std::endl;
            lfns_setting.rejection_quantile_low_accept = -1;
            lfns_settings.thresh_accept_rate = -1;
        }

        lfns_setting.rejection_quantile_low_accept = _lfns_options.rejection_quantile_low_accept;
        lfns_setting.thresh_accept_rate = _lfns_options.thresh_accept_rate;
    }
    if (_lfns_options.vm.count("priorfile") > 0) {
        lfns_setting.uniform_prior = false;
        lfns_setting.prior_file = _lfns_options.prior_file;
    }
    return lfns_setting;
}
