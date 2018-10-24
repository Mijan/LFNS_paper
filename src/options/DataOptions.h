//
// Created by jan on 24/10/18..
//

#ifndef LFNS_DATAOPTIONS_H
#define LFNS_DATAOPTIONS_H

#include "CommandLineOptions.h"

namespace options {
    class DataOptions : public CommandLineOptions {

    public:
        DataOptions() : CommandLineOptions(), num_used_data(0) {
            desc.add_options()(
                    "numuseddata,u",
                    po::value<int>(&num_used_data),
                    "The maximal number of trajectories to be used for the likelihood computation (per default, all provided trajectories or trees will be used)");
        };

        virtual ~DataOptions() {};

        int num_used_data;

    };
}

#endif //LFNS_DATAOPTIONS_H
