//
// Created by jan on 31/10/18.
//

#include "ComputeLikelihoodOptions.h"
#include "DataOptions.h"
#include "../base/IoUtils.h"


namespace options {
    ComputeLikelihoodOptions::ComputeLikelihoodOptions()
            : DataOptions(), params(0), num_computations(0), H(0), use_proper_resampling(false), max_time(-1) {
        desc.add_options()("parameter,p", po::value<std::vector<double> >(&params)->multitoken(),
                           "The parameters used for the simulation")("parameter-file,P",
                                                                     po::value<std::string>(&param_file),
                                                                     "The file from which all parameters will be read")(
                "numcomputations,n", po::value<int>(&num_computations),
                "The number of computations to be performed")("numsmcparticles,H", po::value<int>(&H),
                                                              "The number of particles used for the particle filter")(
                "resampling,r", po::value<bool>(&use_proper_resampling),
                "Indicates whether proper resampling (backpropagation up the tree) should be applied for tree data.");
    }

    ComputeLikelihoodOptions::~ComputeLikelihoodOptions() {}


    int ComputeLikelihoodOptions::handleCommandLineOptions(int argc, char **argv) {
        if (!DataOptions::handleCommandLineOptions(argc, argv)) { return 0; }
        if (vm.count("parameter-file") > 0) {
            if (!base::IoUtils::isPathAbsolute(param_file)) {
                fs::path full_path(fs::current_path());
                param_file = full_path.string() + "/" + param_file;
            }
        }
        return 1;
    }
}