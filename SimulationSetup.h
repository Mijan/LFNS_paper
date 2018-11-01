//
// Created by jan on 29/10/18.
//

#ifndef LFNS_SIMULATIONSETUP_H
#define LFNS_SIMULATIONSETUP_H


#include "src/base/RandomDistributions.h"
#include "src/simulator/Simulator.h"
#include "src/models/FullModel.h"
#include "src/options/SimulationOptions.h"
#include "src/base/IoUtils.h"
#include "src/io/IoSettings.h"
#include "src/models/ModelSettings.h"
#include "GeneralSetup.h"

typedef std::vector<double> Times;
typedef std::vector<std::vector<double>> Trajectory;
typedef std::vector<Trajectory> TrajectorySet;

class SimulationSetup : public GeneralSetup {

public:
    SimulationSetup(options::SimulationOptions &options);

    virtual ~SimulationSetup();

    std::vector<std::vector<double> > parameters;
    Times times;


    int number_simulations = 1;
    std::vector<double> parameter = {};
    std::string parameter_file = "";
    std::vector<std::string> sim_param_names;
    double initial_time = 0;
    double final_time = 100;
    double interval = 1;

    void setUp();

    void printSettings(std::ostream &os) override;

private:

    options::SimulationOptions _sim_options;


    void _readSettingsfromFile() override;

    std::vector<std::string> _readExperiments() override;

    void _createParameterVector();

    void _createOutputTimes();

    void _readSimulationSettings();
};


#endif //LFNS_SIMULATIONSETUP_H
