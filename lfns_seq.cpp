#include <iostream>
#include "src/options/LFNSOptions.h"
#include "LFNSSetup.h"
#include "src/base/IoUtils.h"
#include "src/LFNS/seq/LFNSSeq.h"
#include "src/sampler/UniformSampler.h"
#include "src/sampler/EllipsoidSampler.h"
#include "src/sampler/DpGmmSampler.h"
#include "src/sampler/GaussianSampler.h"
#include "src/sampler/KernelDensityEstimation.h"

static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;


options::LFNSOptions lfns_options;

int runLFNS(LFNSSetup &lfns_setup);

int main(int argc, char **argv) {
    try {
        lfns_options.handleCommandLineOptions(argc, argv);


        LFNSSetup likelihood_setup(lfns_options);
        likelihood_setup.setUp();
        return runLFNS(likelihood_setup);
    } catch (const std::exception &e) {
        std::cerr << "Failed to run LFNS, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
    return 1;

}

// TODO add check if output files can be saved BEFORE code runs!
int runLFNS(LFNSSetup &lfns_setup) {
    lfns_setup.printSettings(model_summary_stream);
    std::cout << model_summary_stream.str();

    std::string model_summary_file_name = base::IoUtils::appendToFileName(lfns_setup.io_settings.output_file,
                                                                          model_summary_suffix);
    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
    model_summary_file_stream << model_summary_stream.str();
    model_summary_file_stream.close();


    lfns::seq::LFNSSeq lfns(lfns_setup.lfns_settings, lfns_setup.sampler_settings, lfns_setup.rng,
                            lfns_setup.mult_like_eval.getLogLikeFun());
    if (!lfns_setup.lfns_settings.previous_log_file.empty()) {
        lfns.resumeRum(lfns_setup.lfns_settings.previous_log_file);
    }
    if (lfns_setup.particle_filter_settings.use_premature_cancelation) {
        for (particle_filter::ParticleFilter_ptr filter : lfns_setup.particle_filters) {
            filter->setThresholdPtr(lfns.getPointerToThreshold());
        }
        lfns_setup.mult_like_eval.setThresholdPtr(lfns.getPointerToThreshold());
    }

    lfns.runLFNS();
    return 1;
}
