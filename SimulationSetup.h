//
// Created by jan on 29/10/18.
//

#ifndef LFNS_SIMULATIONSETUP_H
#define LFNS_SIMULATIONSETUP_H


#include "src/simulator/SimulationSettings.h"
#include "src/base/RandomDistributions.h"
#include "src/simulator/Simulator.h"
#include "src/models/FullModel.h"
#include "src/options/SimulationOptions.h"
#include "LFNSSetup.h"
#include "src/base/IoUtils.h"
#include "src/io/IoSettings.h"
#include "src/models/ModelSettings.h"

class SimulationSetup {

public:
    std::vector<simulator::Simulator_ptr> simulators;
    std::vector<models::FullModel_ptr> full_models;
    simulator::SimulationSettings simulation_settings;

    io::IoSettings io_settings;
    models::ModelSettings model_settings;

    base::RngPtr rng;
    std::vector<std::vector<double> > parameters;
    Times times;


    int number_simulations = 1;
    std::vector<std::string> experiments;
    std::vector<double> parameter = {};
    std::string parameter_file = "";
    double initial_time = 0;
    double final_time = 100;
    double interval = 1;

    void setUp(options::SimulationOptions &options);

    void readSettingsfromFile(options::SimulationOptions &options);

    simulator::Simulator_ptr
    createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
                    simulator::SimulationSettings &settings);

    void createParameterVector();

    void createOutputTimes();

    void printSettings(std::ostream &os);


private:
    models::ModelSettings
    _readModelSettings(io::ConfigFileInterpreter &interpreter, std::vector<std::string> experiments);

    void _setInputData(models::ModelSettings &model_settings, io::ConfigFileInterpreter &interpreter);

    simulator::SimulationSettings
    _setSimulationSettings(io::ConfigFileInterpreter &interpreter, options::SimulationOptions &options);


};


#endif //LFNS_SIMULATIONSETUP_H
