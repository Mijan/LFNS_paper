//
// Created by jan on 01/11/18.
//

#include "GeneralSetup.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/simulator/SimulatorOde.h"
#include "src/simulator/SimulatorHybrid.h"

GeneralSetup::GeneralSetup(options::CommandLineOptions &options, int process_nbr) : interpreter(
        options.config_file_name), rng(nullptr) {
    io_settings.config_file = options.config_file_name;
    io_settings.output_file = options.output_file_name;
    experiments = options.experiments;
    rng = std::make_shared<base::RandomNumberGenerator>(process_nbr * time(NULL));
}

GeneralSetup::~GeneralSetup() {}

void GeneralSetup::_readSettingsfromFile() {
    if (experiments.empty()) { experiments = _readExperiments(); }
    model_settings = _readModelSettings();
}


void GeneralSetup::printSettings(std::ostream &os) { io_settings.print(os); }

models::ModelSettings GeneralSetup::_readModelSettings() {
    models::ModelSettings model_settings;
    model_settings.model_file = interpreter.getModelFileName();
    model_settings.initial_value_file = interpreter.getInitialConditionsFile();
    model_settings.measurement_file = interpreter.getMeasurementModelFile();

    models::ModelReactionData model_data(model_settings.model_file);
    models::InitialValueData init_data(model_settings.initial_value_file);
    models::MeasurementModelData measure_data(model_settings.measurement_file);

    std::vector<std::string> param_names = model_data.getParameterNames();
    base::Utils::addOnlyNew<std::string>(param_names, init_data.getParameterNames());
    base::Utils::addOnlyNew<std::string>(param_names, measure_data.getParameterNames());
    model_settings.param_names = param_names;

    try {
        model_settings.fixed_parameters = interpreter.getFixedParameters();
    } catch (const std::exception &e) {}

    std::string model_type_str = interpreter.getModelType();
    if (models::MODEL_TYPE_NAME.count(model_type_str) == 0) {
        std::stringstream os;
        os << "Modeltype " << model_type_str << " not known. Possible options are: ";
        std::map<std::string, models::MODEL_TYPE>::iterator it = models::MODEL_TYPE_NAME.begin();
        for (it; it != models::MODEL_TYPE_NAME.end(); it++) { os << it->first << ", "; }
        throw std::runtime_error(os.str());
    }
    model_settings.model_type = models::MODEL_TYPE_NAME[model_type_str];

    if (model_settings.model_type == models::MODEL_TYPE::HYBRID) {
        bool species_found = false;
        if (interpreter.detSpeciesProvided()) {
            species_found = true;
            model_settings.det_species_names = interpreter.getDetSpecies();
        }
        if (interpreter.stochSpeciesProvided()) {
            if (species_found) {
                throw std::runtime_error(
                        "For Hybrid model either deterministic (model.detspecies) or stochastic species (model.stochspecies) need to be provided, but not both!");
            }
            model_settings.stoch_species_names = interpreter.getStochSpecies();
            species_found = true;
        }

        if (!species_found) {
            throw std::runtime_error(
                    "For Hybrid model either deterministic (model.detspecies) or stochastic species (model.stochspecies) need to be provided, but none of them are set!");
        }
    }

    return model_settings;
}

std::vector<models::InputData> GeneralSetup::_getInputDatasForExperiment(std::string experiment, double final_time) {
    std::vector<double> periods = interpreter.getPulsePeriods(experiment);
    std::vector<models::InputData> datas;
    if (!periods.empty()) {
        std::vector<double> strength = interpreter.getPulseStrengths(experiment);
        std::vector<double> duration = interpreter.getPulseDurations(experiment);
        std::vector<int> num_pulses = interpreter.getNumPulse(experiment);
        std::vector<std::string> names = interpreter.getPulseInputNames(experiment);
        std::vector<double> starting_times = interpreter.getStartingTimes(experiment);

        for (int i = 0; i < periods.size(); i++) {
            if (starting_times[i] > final_time) {
                std::cerr << "For experiment " << experiment << " perturbation provided, but starting time "
                          << starting_times[i] << " is after the last considered time" << final_time
                          << ", thus perturbation for experiment " << experiment << " on the parameter "
                          << names[i] << " starting at " << starting_times[i] << " will be ignored!" << std::endl;
            } else {
                int max_num_pulses = std::min(num_pulses[i],
                                              (int) std::ceil((final_time - starting_times[i]) / (periods[i])));

                if (max_num_pulses > 0) {
                    datas.push_back(
                            models::InputData(periods[i], strength[i], duration[i], max_num_pulses, names[i],
                                              starting_times[i]));
                }
            }
        }
    }
    return datas;
}

simulator::Simulator_ptr GeneralSetup::_createSimulator(models::ChemicalReactionNetwork_ptr dynamics) {
    simulator::Simulator_ptr sim_ptr;

    if (model_settings.model_type == models::MODEL_TYPE::STOCH) {
        simulator::SimulatorSsa simulator_ssa(rng, dynamics->getPropensityFct(), dynamics->getReactionFct(),
                                              dynamics->getNumReactions());
        sim_ptr = std::make_shared<simulator::SimulatorSsa>(simulator_ssa);
    } else if (model_settings.model_type == models::MODEL_TYPE::HYBRID) {
        if (!model_settings.stoch_species_names.empty()) {
            std::cout << "set stoch speices" << std::endl;
            dynamics->setStochStatesForHybridModel(model_settings.stoch_species_names);
        } else {
            dynamics->setDetStatesForHybridModel(model_settings.det_species_names);
        }
        simulator::SimulatorSsa simulator_ssa(rng, dynamics->getPropensityFct(), dynamics->getReactionFct(),
                                              dynamics->getNumReactions());
        simulator::SimulatorSsa_ptr simulator_ssa_ptr = std::make_shared<simulator::SimulatorSsa>(simulator_ssa);

        simulator::OdeSettings ode_settings;
        simulator::SimulatorOde simulator_ode(ode_settings, dynamics->getRhsFct(), dynamics->getNumSpecies());
        simulator::SimulatorOde_ptr simulator_ode_ptr = std::make_shared<simulator::SimulatorOde>(simulator_ode);

        std::vector<int> stoch_indices = dynamics->getStochIndices();
        std::vector<int> det_indices = dynamics->getDetSpeciesIndices();
        sim_ptr = std::make_shared<simulator::SimulatorHybrid>(simulator_ssa_ptr, simulator_ode_ptr, stoch_indices,
                                                               det_indices);
    } else {
        simulator::OdeSettings ode_settings;
        simulator::SimulatorOde simulator_ode(ode_settings, dynamics->getRhsFct(), dynamics->getNumSpecies());
        simulator::SimulatorOde_ptr simulator_ode_ptr = std::make_shared<simulator::SimulatorOde>(simulator_ode);
        sim_ptr = simulator_ode_ptr;
    }
    return sim_ptr;
}