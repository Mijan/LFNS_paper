//
// Created by jan on 28/10/18.
//

#include "LFNSSetup.h"
#include "src/io/ConfigFileInterpreter.h"
#include "src/LFNS/LFNS.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/simulator/SimulatorSsa.h"


void LFNSSetup::setUp(options::LFNSOptions &options) {
    readSettingsfromFile(options);

    rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
    for (std::string experiment : lfns_settings.experiments_for_LFNS) {

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
}

void LFNSSetup::readSettingsfromFile(options::LFNSOptions &options) {

    io_settings.config_file = options.config_file_name;
    io_settings.output_file = options.output_file_name;
    lfns_settings.output_file = io_settings.output_file;
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


    sampler_settings.param_names = model_settings.getUnfixedParameters();

    sampler_settings.parameter_bounds = interpreter.getParameterBounds();
    std::map<std::string, std::string> log_scale_str = interpreter.getParameterScales();
    std::map<std::string, std::string>::iterator it;
    for (std::string &param : sampler_settings.param_names) {
        std::string scale = "log";
        if (log_scale_str.count(param) > 0) {
            scale = log_scale_str[param];
            if (scale.compare("lin") != 0 && scale.compare("log") != 0 && scale.compare("linear") != 0) {
                std::stringstream ss;
                ss << "Failed to set scale for parameter " << it->first
                   << ". Scale needs to be 'lin' or 'log', but provided scale is :" << scale << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        sampler_settings.parameters_log_scale[param] = scale.compare("log") == 0;
    }
    lfns_settings.experiments_for_LFNS = interpreter.getExperimentsForLFNS();
    particle_filter_settings.experiments_for_particle_filter = lfns_settings.experiments_for_LFNS;
    simulation_settings.experiments_for_simulation = lfns_settings.experiments_for_LFNS;
    if (lfns_settings.experiments_for_LFNS.empty()) {
        std::cerr << "No experiment names for LFNS algorithm provided, only basic model without inputs will be used!";
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

                std::vector<double> times = createDataTimes(experiment, particle_filter_settings);
                std::vector<models::InputData> datas;
                for (int i = 0; i < periods.size(); i++) {
                    double last_needed_time = times.back();
                    if (starting_times[i] > last_needed_time) {
                        std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                                  << starting_times[i] << " is after the final data point time "
                                  << last_needed_time << ", thus perturbation for experiment " << experiment
                                  << " on the parameter " << names[i] << " starting at " << starting_times[i]
                                  << " will be ignored!" << std::endl;
                    } else {
                        int max_num_pulses = std::min(num_pulses[i], (int) ((last_needed_time - starting_times[i]) /
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

    particle_filter_settings.data_files = interpreter.getDataFiles();
    particle_filter_settings.time_files = interpreter.getTimesFiles();
    particle_filter_settings.H = interpreter.getHForLFNS();

    lfns_settings.N = interpreter.getNForLFNS();
    lfns_settings.r = interpreter.getRForLFNS();
    lfns_settings.log_termination = std::log(interpreter.getEpsilonForLFNS());


    io_settings.output_file = options.output_file_name;
    if (options.vm.count("LFNSparticles") > 0) { lfns_settings.N = options.N; }
    if (options.vm.count("numberprallelsamples") > 0) { lfns_settings.r = options.r; }
    if (options.vm.count("smcparticles") > 0) { particle_filter_settings.H = options.H; }
    if (options.vm.count("tolerance") > 0) { lfns_settings.log_termination = std::log(options.LFNS_tolerance); }
    if (options.vm.count("prematurecancelling") >
        0) { particle_filter_settings.use_premature_cancelation = options.use_premature_cancelation; }
    if (options.vm.count("previous_pop") > 0) { lfns_settings.previous_log_file = options.previous_population_file; }
    if (options.vm.count("numuseddata") > 0) { particle_filter_settings.num_used_trajectories = options.num_used_data; }
    if (options.vm.count("printinterval") > 0) { lfns_settings.print_interval = options.print_interval; }
    if (options.vm.count("rej_quan") >
        0) { lfns_settings.rejection_quantile_for_density_estimation = options.rejection_quantile; }
}


TrajectorySet
LFNSSetup::createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings) {
    if (settings.data_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No data file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    TrajectorySet data = base::IoUtils::readMultiline(settings.data_files[experiment], num_outputs,
                                                      settings.num_used_trajectories);
    return data;
}

Times LFNSSetup::createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings) {
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
LFNSSetup::createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
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


void LFNSSetup::printSettings(std::ostream &os) {
    io_settings.print(os);
    lfns_settings.print(os);
    particle_filter_settings.print(os);
    model_settings.print(os);
    full_models.back()->printInfo(os);
}