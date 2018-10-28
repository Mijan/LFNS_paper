#include <iostream>
#include "src/LFNS/LFNSSettings.h"
#include "src/LFNS/LFNS.h"
#include "src/LFNS/seq/LFNSSeq.h"


double log_likelihood_function(const std::vector<double> &param) {
    return (-100 * param[0]);
}

int main() {
//    lfns::LFNSSettings settings;
//    settings.parameters.push_back(lfns::ParameterSetting("param_1", {0, 1}, false, false));
//    settings.uniform_prior = true;
//    settings.estimator = lfns::REJECT_DPGMM;
//    settings.output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lfns_output.txt";
//    settings.log_termination = -4.6;
//    settings.N = 1000;
//    settings.r = 100;
//
//    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
//    std::function<double(const std::vector<double>)> fct = log_likelihood_function;
//    lfns::LogLikelihodEvalFct_ptr fct_ptr = std::make_shared<lfns::LogLikelihodEvalFct>(fct);
//    lfns::seq::LFNSSeq lfns(settings, rng, fct_ptr);
//    lfns.runLFNS();
//    return 1;
}