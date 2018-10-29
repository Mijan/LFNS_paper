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
    for (std::string experiment : settings.experiments_for_LFNS) {

        models::FullModel_ptr full_model = createModel(rng, settings);
        full_models.push_back(full_model);


        times_vec.push_back(createDataTimes(experiment, settings));
        TrajectorySet data = createData(full_model->measurement_model->getNumMeasurements(), experiment, settings);
        data_vec.push_back(data);

        simulator::Simulator_ptr simulator = createSimulator(rng, full_model->dynamics, settings);
        simulators.push_back(simulator);

        setUpPerturbations(experiment, simulator, full_model, settings);

        particle_filter::ParticleFilter part_filter(rng, full_models.back()->getParameterSettingFct(),
                                                    simulator->getSimulationFct(), simulator->getResetFct(),
                                                    full_model->measurement_model->getLikelihoodFct(),
                                                    full_model->initial_value_provider->getInitialStateFct(),
                                                    full_model->dynamics->getNumSpecies(), settings.H);
        particle_filters.push_back(std::make_shared<particle_filter::ParticleFilter>(part_filter));

        for (int traj_nbr = 0; traj_nbr < data.size(); traj_nbr++) {
            mult_like_eval.addLogLikeFun(
                    particle_filters.back()->getLikelihoodEvaluationForData(&data_vec.back()[traj_nbr],
                                                                            &times_vec.back()));
        }
    }
}

void LFNSSetup::readSettingsfromFile(options::LFNSOptions &options) {

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

    std::map<std::string, std::pair<double, double>> param_bounds = interpreter.getParameterBounds();
    std::map<std::string, double> fixed_values = interpreter.getFixedParameters();
    std::map<std::string, std::string> scales = interpreter.getParameterScales();
    settings.experiments_for_LFNS = interpreter.getExperimentsForLFNS();
    if (settings.experiments_for_LFNS.empty()) {
        throw std::runtime_error(
                "No experiment names for LFNS algorithm provided, please provide at least one experiment name to be used for the inference!");
    }

    std::map<std::string, models::InputData> input_datas;

    for (std::string &experiment: settings.experiments_for_LFNS) {
        std::vector<double> periods = interpreter.getPulsePeriods(experiment);
        if (!periods.empty()) {
            std::vector<double> strength = interpreter.getPulseStrengths(experiment);
            std::vector<double> duration = interpreter.getPulseDurations(experiment);
            std::vector<int> num_pulses = interpreter.getNumPulse(experiment);
            std::vector<std::string> names = interpreter.getPulseInputNames(experiment);
            std::vector<double> starting_times = interpreter.getStartingTimes(experiment);

            std::vector<lfns::InputData> datas;
            for (int i = 0; i < periods.size(); i++) {
                datas.push_back(lfns::InputData(periods[i], strength[i], duration[i], num_pulses[i], names[i],
                                                starting_times[i]));
            }
            settings.input_datas[experiment] = datas;
        }
    }


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
    settings.N = interpreter.getNForLFNS();
    settings.r = interpreter.getRForLFNS();
    settings.H = interpreter.getHForLFNS();
    settings.log_termination = std::log(interpreter.getEpsilonForLFNS());


    settings.output_file = options.output_file_name;
    if (options.vm.count("LFNSparticles") > 0) { settings.N = options.N; }
    if (options.vm.count("numberprallelsamples") > 0) { settings.r = options.r; }
    if (options.vm.count("smcparticles") > 0) { settings.H = options.H; }
    if (options.vm.count("tolerance") > 0) {
        settings.log_termination = std::log(options.LFNS_tolerance);
    }
    if (options.vm.count("prematurecancelling") >
        0) { settings.use_premature_cancelation = options.use_premature_cancelation; }
    if (options.vm.count("previous_pop") >
        0) { settings.previous_log_file = options.previous_population_file; }
    if (options.vm.count("numuseddata") > 0) { settings.num_used_trajectories = options.num_used_data; }
    if (options.vm.count("printinterval") > 0) { settings.print_interval = options.print_interval; }
    if (options.vm.count("rej_quan") >
        0) { settings.rejection_quantile_for_density_estimation = options.rejection_quantile; }
}


models::FullModel_ptr LFNSSetup::createModel(base::RngPtr rng, lfns::LFNSSettings &settings) {
    models::ChemicalReactionNetwork_ptr dynamics = std::make_shared<models::ChemicalReactionNetwork>(
            settings.model_file);
    models::InitialValueData init_data(settings.initial_value_file);
    models::InitialValueProvider_ptr init_value = std::make_shared<models::InitialValueProvider>(rng, init_data);
    models::MeasurementModelData measure_data(settings.measurement_file);
    models::MeasurementModel_ptr measurement = std::make_shared<models::MeasurementModel>(rng, measure_data);
    measurement->setStateOrder(dynamics->getSpeciesNames());

    models::FullModel_ptr full_mode = std::make_shared<models::FullModel>(dynamics, init_value, measurement);

    for (int i = 0; i < settings.parameters.size(); i++) {
        lfns::ParameterSetting setting = settings.parameters[i];
        if (setting.fixed) {
            full_mode->fixParameter(setting.name, setting.fixed_value);
        }
    }
    return full_mode;
}


TrajectorySet LFNSSetup::createData(int num_outputs, std::string experiment, lfns::LFNSSettings &settings) {
    if (settings.data_files.count(experiment) == 0) {
        std::stringstream ss;
        ss << "No data file for experiment " << experiment << " provided!" << std::endl;
        throw std::runtime_error(ss.str());
    }
    TrajectorySet data = base::IoUtils::readMultiline(settings.data_files[experiment], num_outputs,
                                                      settings.num_used_trajectories);
    return data;
}

Times LFNSSetup::createDataTimes(std::string experiment, lfns::LFNSSettings &settings) {
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
                           lfns::LFNSSettings &settings) {
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
LFNSSetup::setUpPerturbations(std::string experiment, simulator::Simulator_ptr simulator,
                              models::FullModel_ptr full_model,
                              lfns::LFNSSettings &settings) {

    if (settings.input_datas.count(experiment) > 0) {
        double max_used_time = times_vec.back().back();
        for (models::InputData input_data : settings.input_datas[experiment]) {
            if (input_data.starting_time > max_used_time) {
                std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                          << input_data.starting_time << " is after the last data point time " << max_used_time
                          << ", thus perturbation for experiment " << experiment << " on the parameter "
                          << input_data.pulse_inpt_name << " starting at " << input_data.starting_time
                          << " will be ignored!" << std::endl;
            } else {
                models::InputPulse pulse(input_data.pulse_period, input_data.pulse_strenght, input_data.pulse_duration,
                                         input_data.num_pulses, input_data.pulse_inpt_name, input_data.starting_time,
                                         max_used_time);
                full_models.back()->addInputPulse(pulse);
            }
        }
        simulator->setRootFunction(full_model->getRootFct());
    }
}