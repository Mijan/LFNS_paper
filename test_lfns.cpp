#include <iostream>
#include "src/LFNS/LFNSSettings.h"
#include "src/LFNS/LFNS.h"


double log_likelihood_function(const std::vector<double> &param) {
    return (-100 * param[0]);
}

int main() {
    lfns::LFNSSettings settings;
    settings.parameter_log_scale = {false};
    settings.parameter_bounds = {std::make_pair(0, 1)};
    settings.uniform_prior = true;
    settings.estimator = lfns::REJECT_DPGMM;
    settings.output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lfns_output.txt";
    settings.log_termination = -4.6;
    settings.N = 1000;
    settings.r = 100;

    std::function<double(const std::vector<double>)> fct = log_likelihood_function;
    lfns::LFNS lfns(settings, fct);
    lfns.runLFNS();
    return 1;
}