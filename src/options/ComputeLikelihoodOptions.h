//
// Created by jan on 31/10/18.
//

#ifndef LFNS_COMPUTELIKELIHOODOPTIONS_H
#define LFNS_COMPUTELIKELIHOODOPTIONS_H

#include "DataOptions.h"

namespace options {
    class ComputeLikelihoodOptions : public DataOptions {
    public:
        ComputeLikelihoodOptions();

        virtual ~ComputeLikelihoodOptions();

        int handleCommandLineOptions(int argc, char **argv);

        std::vector<double> params;
        std::string param_file;
        int num_computations;
        int H;
        bool use_proper_resampling;
        double max_time;
    };
}


#endif //LFNS_COMPUTELIKELIHOODOPTIONS_H
