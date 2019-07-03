//
// Created by jan on 22/10/18.
//

#include "src/LFNS/mpi/MpiTags.h"
#include "src/base/IoUtils.h"
#include "src/particle_filter/ParticleFilter.h"
#include "src/LFNS/mpi/LFNSWorker.h"
#include "src/options/LFNSOptions.h"
#include "src/base/Utils.h"
#include "LFNSSetup.h"
#include "src/LFNS/mpi/LFNSMpi.h"


static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;


options::LFNSOptions lfns_options;

int my_rank;
int num_tasks;

namespace bmpi = boost::mpi;

void runMaster(LFNSSetup &lfns_setup);

void runWorker(LFNSSetup &lfns_setup);

int main(int argc, char *argv[]) {
    bmpi::environment env;
    bmpi::communicator world;

    my_rank = world.rank();
    num_tasks = world.size();

    if (num_tasks == 1) {
        std::cerr << "When using mpi version of LFNS at least two processes must be started!" << std::endl;
        abort();
    }

    try {
        lfns_options.handleCommandLineOptions(argc, argv);

        LFNSSetup lfns_setup(lfns_options, my_rank);
        lfns_setup.setUp();
        if (my_rank == 0) { runMaster(lfns_setup); }
        else { runWorker(lfns_setup); }
    } catch (const std::exception &e) {
        std::cerr << "Failed to run LFNS, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
}


void runMaster(LFNSSetup &lfns_setup) {

    lfns::mpi::LFNSMpi lfns(lfns_setup.lfns_settings, num_tasks);
    lfns.setSampler(lfns_setup.prior, lfns_setup.density_estimation, lfns_setup.rng);
    lfns.setLogParams(lfns_setup.sampler_settings.getLogParams());
    if (!lfns_setup.lfns_settings.previous_log_file.empty()) {
        lfns.resumeRum(lfns_setup.lfns_settings.previous_log_file);
    }
    lfns.runLFNS();
}

void runWorker(LFNSSetup &lfns_setup) {

    if (my_rank == 1) {
        lfns_setup.printSettings(model_summary_stream);
        std::cout << model_summary_stream.str();

        std::string model_summary_file_name = base::IoUtils::appendToFileName(
                lfns_setup.lfns_settings.output_file,
                model_summary_suffix);
        std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
        model_summary_file_stream << model_summary_stream.str();
        model_summary_file_stream.close();
    }

    lfns::mpi::LFNSWorker worker(my_rank, lfns_setup.full_models.front()->getUnfixedParamteters().size(),
                                 lfns_setup.mult_like_eval.getLogLikeFun());
    worker.setSampler(lfns_setup.prior, lfns_setup.density_estimation, lfns_setup.rng);
    worker.setLogParams(lfns_setup.sampler_settings.getLogParams());

    for (int i = 0; i < lfns_setup.particle_filters.size(); i++) {
        lfns_setup.simulators[i]->addStoppingCriterion(worker.getStoppingFct());
        lfns_setup.particle_filters[i]->addStoppingCriterion(worker.getStoppingFct());
        if (lfns_setup.particle_filter_settings.use_premature_cancelation) {
            lfns_setup.particle_filters[i]->setThresholdPtr(worker.getEpsilonPtr());
        }
    }
    if (lfns_setup.particle_filter_settings.use_premature_cancelation) {
        lfns_setup.mult_like_eval.setThresholdPtr(worker.getEpsilonPtr());
    }
    worker.run();
}
