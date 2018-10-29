//
// Created by jan on 29/10/18.
//

#ifndef LFNS_SIMULATIONOPTIONS_H
#define LFNS_SIMULATIONOPTIONS_H

#include "CommandLineOptions.h"

namespace options {
class SimulationOptions : public CommandLineOptions {
public:
    SimulationOptions();

    virtual ~SimulationOptions();

    int handleCommandLineOptions(int argc, char **argv);

    bool initialTimeProvided();

    std::vector<double> params;
    std::string param_file;
    int num_simulations;
    double initial_time;
    double final_time;
    double interval;
    int max_generation;
};

}


#endif //LFNS_SIMULATIONOPTIONS_H
