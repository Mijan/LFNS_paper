//
// Created by jan on 24/10/18..
//

#ifndef LFNS_LFNSOPTIONS_H
#define LFNS_LFNSOPTIONS_H


#include "DataOptions.h"

namespace options {
    class LFNSOptions : public DataOptions {
    public:
        LFNSOptions();

        virtual ~LFNSOptions();

        int num_dpgmm_iterations;
        int H;
        int N;
        bool use_premature_cancelation;


        int r;
        double rejection_quantile;
        double rejection_quantile_low_accept;
        double thresh_accept_rate;
        double LFNS_tolerance;
        int _sampler_index;
        std::string previous_population_file;

        int print_interval;
        std::string prior_file;

        bool verbose;

        virtual int handleCommandLineOptions(int argc, char **argv);

        bool useProperResamplingSet();

        bool samplerSet();
    };
}


#endif //LFNS_LFNSOPTIONS_H
