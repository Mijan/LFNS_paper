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


    void setUp(options::SimulationOptions &options);

    void readSettingsfromFile(options::SimulationOptions &options);

    simulator::Simulator_ptr
    createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
                    simulator::SimulationSettings &settings);

    void createParameterVector();
    void createOutputTimes();

    void printSettings(std::ostream &os);


};


#endif //LFNS_SIMULATIONSETUP_H
