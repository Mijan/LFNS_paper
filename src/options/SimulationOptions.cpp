//
// Created by jan on 29/10/18.
//

#include "SimulationOptions.h"
#include "../base/IoUtils.h"

namespace options {
    SimulationOptions::SimulationOptions() :
            CommandLineOptions(), params(0), param_file(""), num_simulations(-1), initial_time(0.0),
            final_time(0.0), interval(-1), max_generation(-1) {

        desc.add_options()("parameter,p", po::value<std::vector<double> >(&params)->multitoken(),
                           "The parameters used for the simulation")("parameter-file,P",
                                                                     po::value<std::string>(&param_file),
                                                                     "The file from which all parameters will be read");
        desc.add_options()("numsimulations,n", po::value<int>(&num_simulations),
                           "The number of simulations to be performed");
        desc.add_options()("initialtime,I", po::value<double>(&initial_time),
                           "The timepoint at which the simulation starts");
        desc.add_options()("finaltime,F", po::value<double>(&final_time),
                           "The timepoint at which the simulation ends");
        desc.add_options()("interval,i", po::value<double>(&interval),
                           "Interval at which the output should be written");
        desc.add_options()("maxgeneration,g", po::value<int>(&max_generation),
                           "The maximal generation to be simulated for trees");
        desc.add_options();
    }


    SimulationOptions::~SimulationOptions() {}

    int SimulationOptions::handleCommandLineOptions(int argc, char **argv) {
        if (!CommandLineOptions::handleCommandLineOptions(argc, argv)) {
            return 0;
        }
        if (vm.count("initialtime") != vm.count("finaltime")) {
            std::ostringstream os;
            os
                    << "Both, initial time (-I) and final time (-F), or none of those have to be provided!"
                    << std::endl;
            throw std::runtime_error(os.str());
        }

        if (vm.count("parameter") > 0 && vm.count("parameter-file") > 0) {
            std::ostringstream os;
            os <<
               "If simulation parameter (-p) is provided, no simulation parameter file (-P) can be provided!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        if (vm.count("parameter-file") > 0 && vm.count("numsimulations") > 0 && num_simulations > 1) {
            std::ostringstream os;
            os <<
               "If simulation parameter file (-P) is provided, number of simulations (-n) needs to be 1"
               << std::endl;
            throw std::runtime_error(os.str());
        }
        if (vm.count("parameter-file") > 0) {
            if (!base::IoUtils::isPathAbsolute(param_file)) {
                fs::path full_path(fs::current_path());
                param_file = full_path.string() + "/" + param_file;
            }
        }

        if (vm.count("interval") > 0 && vm.count("initialtime") == 0) {
            std::ostringstream os;
            os << "If interval (-i) is provided,  initialtime (-I) must be provided!" << std::endl;
            throw std::runtime_error(os.str());
        }

        return 1;
    }

    bool SimulationOptions::initialTimeProvided() {
        return vm.count("initialtime") > 0;
    }
}