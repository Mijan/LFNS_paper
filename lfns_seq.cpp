#include <iostream>
#include "src/options/LFNSOptions.h"
#include "LFNSSetup.h"
#include "src/base/IoUtils.h"
#include "src/LFNS/seq/LFNSSeq.h"

static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;


options::LFNSOptions likelihood_options;

int runLFNS(LFNSSetup &likelihood_setup);

int main(int argc, char **argv) {
    try {
        likelihood_options.handleCommandLineOptions(argc, argv);


        LFNSSetup likelihood_setup(likelihood_options);
        likelihood_setup.setUp();
        return runLFNS(likelihood_setup);
    } catch (const std::exception &e) {
        std::cerr << "Failed to run LFNS, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
    return 1;

}

// TODO add check if output files can be saved BEFORE code runs!
int runLFNS(LFNSSetup &likelihood_setup) {
    likelihood_setup.printSettings(model_summary_stream);
    std::cout << model_summary_stream.str();

    std::string model_summary_file_name = base::IoUtils::appendToFileName(likelihood_setup.io_settings.output_file,
                                                                          model_summary_suffix);
    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
    model_summary_file_stream << model_summary_stream.str();
    model_summary_file_stream.close();


    lfns::seq::LFNSSeq lfns(likelihood_setup.lfns_settings, likelihood_setup.sampler_settings, likelihood_setup.rng,
                            likelihood_setup.mult_like_eval.getLogLikeFun());
    if (!likelihood_setup.lfns_settings.previous_log_file.empty()) {
        lfns.resumeRum(likelihood_setup.lfns_settings.previous_log_file);
    }
    if (likelihood_setup.particle_filter_settings.use_premature_cancelation) {
        for (particle_filter::ParticleFilter_ptr filter : likelihood_setup.particle_filters) {
            filter->setThresholdPtr(lfns.getPointerToThreshold());
        }
    }

    lfns.runLFNS();
    return 1;
}
