#include <iostream>
#include "src/options/LFNSOptions.h"
#include "LFNSSetup.h"
#include "src/base/IoUtils.h"
#include "src/LFNS/seq/LFNSSeq.h"
#include "MCMCSetup.h"
#include "src/MCMC/MCMC.h"

static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;


options::MCMCOptions mcmc_options;

int runMCMC(MCMCSetup &lfns_setup);

int main(int argc, char **argv) {
    try {
        mcmc_options.handleCommandLineOptions(argc, argv);


        MCMCSetup mcmc_setup(mcmc_options);
        mcmc_setup.setUp();
        return runMCMC(mcmc_setup);
    } catch (const std::exception &e) {
        std::cerr << "Failed to run MCMC, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
    return 1;

}

// TODO add check if output files can be saved BEFORE code runs!
int runMCMC(MCMCSetup &mcmc_setup) {
    mcmc_setup.printSettings(model_summary_stream);
    std::cout << model_summary_stream.str();

    std::string model_summary_file_name = base::IoUtils::appendToFileName(mcmc_setup.io_settings.output_file,
                                                                          model_summary_suffix);
    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
    model_summary_file_stream << model_summary_stream.str();
    model_summary_file_stream.close();


    mcmc::MCMC mcmc(mcmc_setup.mcmc_settings, mcmc_setup.sampler_settings, mcmc_setup.rng,
                    mcmc_setup.mult_like_eval.getLogLikeFun());
    if (!mcmc_setup.mcmc_settings.previous_pop_file.empty()) {
        mcmc.resumeRum(mcmc_setup.mcmc_settings.previous_pop_file);
    }
    mcmc.runMCMC();
    return 1;
}
