//
// Created by jan on 25/02/19.
//

#ifndef LFNS_MCMCSETTINGS_H
#define LFNS_MCMCSETTINGS_H

namespace mcmc {

    enum KERNEL_TYPE {
        GAUSS
    };

    class MCMCSettings {
    public:
        bool uniform_prior = true;
        KERNEL_TYPE kernel_type = GAUSS;
        std::string output_file;
        int N = 10000;

        std::string previous_pop_file;

    };
}


#endif //LFNS_MCMCSETTINGS_H
