#include <iostream>
#include "src/LFNS/LFNSSettings.h"
#include "src/models/ChemicalReactionNetwork.h"
#include "src/particle_filter/ParticleFilter.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/models/InitialValueData.h"
#include "src/models/InitialValueProvider.h"
#include "src/models/MeasurementModel.h"
#include "src/base/IoUtils.h"
#include "src/LFNS/seq/LFNSSeq.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/base/Utils.h"
#include "src/options/LFNSOptions.h"

static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;

typedef std::vector<double> Times;
typedef std::vector <std::vector<double>> Trajectory;
typedef std::vector <Trajectory> TrajectorySet;

std::vector <simulator::SimulatorSsa_ptr> simulators_ssa;
std::vector <particle_filter::ParticleFilter_ptr> particle_filters;
std::vector <Times> times_vec;
std::vector <TrajectorySet> data_vec;

options::LFNSOptions lfns_options;

lfns::LFNSSettings readSettingsfromFile(std::string xml_file);

int runLFNS(lfns::LFNSSettings settings);

int main(int argc, char **argv) {
    try {
        lfns_options.handleCommandLineOptions(argc, argv);

        std::cout << "Config file: " << lfns_options.config_file_name << std::endl;
        lfns::LFNSSettings settings_read = readSettingsfromFile(lfns_options.config_file_name);

        return runLFNS(settings_read);
    } catch (const std::exception &e) {
        std::cerr << "Failed to run LFNS, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
    return 1;

}

int runLFNS(lfns::LFNSSettings settings) {


    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

    models::ModelReactionData model_data(settings.model_file);
    models::ChemicalReactionNetwork dynamics(model_data);
    dynamics.setInputParameterOrder(settings.getUnfixedParameters());

    models::InitialValueData init_data(settings.initial_value_file);
    models::InitialValueProvider init_value(rng, init_data);
    init_value.setInputParameterOrder(settings.getUnfixedParameters());


    models::MeasurementModelData measure_data(settings.measurement_file);
    models::MeasurementModel measurement(rng, measure_data);
    measurement.setInputParameterOrder(settings.getUnfixedParameters());
    measurement.setInputStateOrder(dynamics.getSpeciesNames());

    for (int i = 0; i < settings.parameters.size(); i++) {
        lfns::ParameterSetting setting = settings.parameters[i];
        if (setting.fixed) {
            if (dynamics.isParameter(setting.name)) { dynamics.fixParameter(setting.name, setting.fixed_value); }
            if (init_value.isParameter(setting.name)) {
                init_value.fixParameter(setting.name, setting.fixed_value);
            }
            if (measurement.isParameter(setting.name)) {
                measurement.fixParameter(setting.name, setting.fixed_value);
            }
        }
    }


    int max_num_traj = 0;
    lfns::MultLikelihoodEval mult_like_eval;
    for (std::string experiment : settings.experiments_for_LFNS) {
        if (settings.data_files.count(experiment) == 0) {
            std::stringstream ss;
            ss << "No data file for experiment " << experiment << " provided!" << std::endl;
            throw std::runtime_error(ss.str());
        }
        if (settings.time_files.count(experiment) == 0) {
            std::stringstream ss;
            ss << "No time file for experiment " << experiment << " provided!" << std::endl;
            throw std::runtime_error(ss.str());
        }
        std::string input_times_file_name = settings.time_files[experiment];
        Times times = base::IoUtils::readVector(input_times_file_name);
        times_vec.push_back(times);

        TrajectorySet data = base::IoUtils::readMultiline(settings.data_files[experiment],
                                                          measurement.getMeasurementNames().size(),
                                                          settings.num_used_trajectories);
        max_num_traj = max_num_traj > data.size() ? max_num_traj : data.size();
        data_vec.push_back(data);

        simulator::SimulatorSsa simulator_ssa(rng, dynamics.getPropensityFct(), dynamics.getReactionFct(),
                                              dynamics.getNumReactions());
        simulators_ssa.push_back(std::make_shared<simulator::SimulatorSsa>(simulator_ssa));

        particle_filter::ParticleFilter part_filter(rng, simulators_ssa.back()->getSimulationFct(),
                                                    measurement.getLikelihoodFct(), init_value.getInitialStateFct(),
                                                    dynamics.getNumSpecies(), settings.H);
        particle_filters.push_back(std::make_shared<particle_filter::ParticleFilter>(part_filter));

        for (int traj_nbr = 0; traj_nbr < data.size(); traj_nbr++) {
            mult_like_eval.addLogLikeFun(
                    particle_filters.back()->getLikelihoodEvaluationForData(&data_vec.back()[traj_nbr],
                                                                            &times_vec.back()));
        }
    }

    settings.num_used_trajectories = std::min(max_num_traj, settings.num_used_trajectories);
    settings.print(std::cout);
    dynamics.printInfo(std::cout);
    init_value.printInfo(std::cout);
    measurement.printInfo(std::cout);

    settings.print(model_summary_stream);
    dynamics.printInfo(model_summary_stream);
    init_value.printInfo(model_summary_stream);
    measurement.printInfo(model_summary_stream);
    std::string model_summary_file_name = base::IoUtils::appendToFileName(settings.output_file, model_summary_suffix);
    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
    model_summary_file_stream << model_summary_stream.str();
    model_summary_file_stream.close();


    lfns::seq::LFNSSeq lfns(settings, rng, mult_like_eval.getLogLikeFun());
    if (!settings.previous_log_file.empty()) { lfns.resumeRum(settings.previous_log_file); }
    if (settings.use_premature_cancelation) {
        for (particle_filter::ParticleFilter_ptr filter : particle_filters) {
            filter->setThresholdPtr(lfns.getPointerToThreshold());
        }
    }
    lfns.runLFNS();
    return 1;
}

lfns::LFNSSettings readSettingsfromFile(std::string xml_file) {
    io::ConfigFileInterpreter interpreter(xml_file);
    lfns::LFNSSettings settings;

    settings.model_file = interpreter.getModelFileName();
    settings.initial_value_file = interpreter.getInitialConditionsFile();
    settings.measurement_file = interpreter.getMeasurementModelFile();

    models::ModelReactionData model_data(settings.model_file);
    models::InitialValueData init_data(settings.initial_value_file);
    models::MeasurementModelData measure_data(settings.measurement_file);

    std::vector <std::string> param_names = model_data.getParameterNames();
    base::Utils::addOnlyNew<std::string>(param_names, init_data.getParameterNames());
    base::Utils::addOnlyNew<std::string>(param_names, measure_data.getParameterNames());

    std::map <std::string, std::pair<double, double>> param_bounds = interpreter.getParameterBounds();
    std::map<std::string, double> fixed_values = interpreter.getFixedParameters();
    std::map <std::string, std::string> scales = interpreter.getParameterScales();

    for (std::string &param : param_names) {
        lfns::ParameterSetting param_setting(param);
        if (param_bounds.count(param) > 0) {
            param_setting.bounds = param_bounds[param];
        }
        if (scales.count(param) > 0) {
            std::string scale = scales[param];
            if (scale.compare("lin") != 0 && scale.compare("log") != 0) {
                std::stringstream ss;
                ss << "Failed to set scale for parameter " << param
                   << ". Scale needs to be 'lin' or 'log', but provided scale is :" << scale << std::endl;
                throw std::runtime_error(ss.str());
            }
            param_setting.log_scale = scale.compare("log") == 0;
        }
        if (fixed_values.count(param) > 0) {
            param_setting.fixed = true;
            param_setting.fixed_value = fixed_values[param];
        }
        settings.parameters.push_back(param_setting);
    }

    settings.data_files = interpreter.getDataFiles();
    settings.time_files = interpreter.getTimesFiles();
    settings.experiments_for_LFNS = interpreter.getExperimentsForLFNS();
    settings.N = interpreter.getNForLFNS();
    settings.r = interpreter.getRForLFNS();
    settings.H = interpreter.getHForLFNS();
    settings.log_termination = std::log(interpreter.getEpsilonForLFNS());


    settings.output_file = lfns_options.output_file_name;
    if (lfns_options.vm.count("LFNSparticles") > 0) { settings.N = lfns_options.N; }
    if (lfns_options.vm.count("numberprallelsamples") > 0) { settings.r = lfns_options.r; }
    if (lfns_options.vm.count("smcparticles") > 0) { settings.H = lfns_options.H; }
    if (lfns_options.vm.count("tolerance") > 0) {
        settings.log_termination = std::log(lfns_options.LFNS_tolerance);
    }
    if (lfns_options.vm.count("prematurecancelling") >
        0) { settings.use_premature_cancelation = lfns_options.use_premature_cancelation; }
    if (lfns_options.vm.count("previous_pop") >
        0) { settings.previous_log_file = lfns_options.previous_population_file; }
    if (lfns_options.vm.count("numuseddata") > 0) { settings.num_used_trajectories = lfns_options.num_used_data; }
    if (lfns_options.vm.count("printinterval") > 0) { settings._print_interval = lfns_options.print_interval; }
    return settings;

}