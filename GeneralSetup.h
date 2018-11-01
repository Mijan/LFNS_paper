//
// Created by jan on 01/11/18.
//

#ifndef LFNS_GENERALSETUP_H
#define LFNS_GENERALSETUP_H


#include "src/options/ComputeLikelihoodOptions.h"
#include "src/io/IoSettings.h"
#include "src/models/ModelSettings.h"
#include "src/models/FullModel.h"
#include "src/simulator/Simulator.h"
#include "src/io/ConfigFileInterpreter.h"

enum class MODEL_TYPE {
    ODE, STOCH
};
static std::map<std::string, MODEL_TYPE> MODEL_TYPE_NAME = {{"DET",   MODEL_TYPE::ODE},
                                                            {"STOCH", MODEL_TYPE::STOCH}};

class GeneralSetup {

public:
    GeneralSetup(options::CommandLineOptions &options);;

    virtual ~GeneralSetup();

    io::IoSettings io_settings;
    models::ModelSettings model_settings;

    base::RngPtr rng;

    std::vector<simulator::Simulator_ptr> simulators;
    std::vector<models::FullModel_ptr> full_models;
    std::vector<std::string> experiments;

    virtual void printSettings(std::ostream &os);

protected:
    io::ConfigFileInterpreter interpreter;
    MODEL_TYPE model_type;

    virtual void _readSettingsfromFile();

    virtual std::vector<std::string> _readExperiments() = 0;

    models::ModelSettings _readModelSettings(std::vector<std::string> experiments);

    std::vector<models::InputData> _getInputDatasForExperiment(std::string experiment, double final_time);

    simulator::Simulator_ptr _createSimulator(models::ChemicalReactionNetwork_ptr dynamics);


};


#endif //LFNS_GENERALSETUP_H