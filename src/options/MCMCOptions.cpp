//
// Created by jan on 01/02/19.
//

#include "MCMCOptions.h"
#include "../base/IoUtils.h"


namespace options {
    MCMCOptions::MCMCOptions()
            : DataOptions(), N(-1), H(-1), previous_population_file(""), verbose(true) {

        desc.add_options()("LFNSparticles,N", po::value<int>(&N), "The number of LFNS particles")("smcparticles,H",
                                                                                                  po::value<int>(&H),
                                                                                                  "The number of particles for the particle filter")(
                "previous_pop,P", po::value<std::string>(&previous_population_file),
                "The file name of the previous population files. An according previous_pop_log_file.txt files needs to be provided")(
                "verbose,v", po::value<bool>(&verbose), "Indicates if ODE error messages should be printed out.");
    }

    MCMCOptions::~MCMCOptions() {}

    int MCMCOptions::handleCommandLineOptions(int argc, char **argv) {
        DataOptions::handleCommandLineOptions(argc, argv);

        if (vm.count("previous_pop") > 0) {
            if (!base::IoUtils::isPathAbsolute(previous_population_file)) {
                fs::path full_path(fs::current_path());
                previous_population_file = full_path.string() + "/" + previous_population_file;
            }
            if (previous_population_file.find_last_of(".") == std::string::npos) {
                std::stringstream os;
                os << "The provided previous population file name " << previous_population_file
                   << " is not a file. Please provide a filename in the format '/path/to/previouspopfile.txt'!"
                   << std::endl;
                throw std::runtime_error(os.str());
            }
        }
        return 1;
    }
}