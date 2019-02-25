//
// Created by jan on 01/02/19.
//

#ifndef LFNS_MCMCOPITIONS_H
#define LFNS_MCMCOPITIONS_H

#include "DataOptions.h"

namespace options {
    class MCMCOptions : public DataOptions {
    public:
        MCMCOptions();

        virtual ~MCMCOptions();

        int H;
        int N;

        std::string previous_population_file;

        bool verbose;

        virtual int handleCommandLineOptions(int argc, char **argv);
    };
}


#endif //LFNS_MCMCOPITIONS_H
