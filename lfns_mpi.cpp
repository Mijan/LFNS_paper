//
// Created by jan on 22/10/18.
//
int my_rank;
int num_tasks;

#include "src/LFNS/mpi/MpiTags.h"
#include "src/LFNS/LFNSSettings.h"
#include "src/base/RandomDistributions.h"
#include "src/LFNS/mpi/LFNSMpi.h"
#include "src/models/ChemicalReactionNetwork.h"
#include "src/models/InitialValueProvider.h"
#include "src/models/MeasurementModel.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/base/IoUtils.h"
#include "src/particle_filter/ParticleFilter.h"
#include "src/LFNS/mpi/LFNSWorker.h"

namespace bmpi = boost::mpi;

void runMaster(lfns::LFNSSettings settings, base::RngPtr rng);

void runWorker(lfns::LFNSSettings settings, base::RngPtr rng);


lfns::LFNSSettings getLacSettings();

lfns::LFNSSettings getBdSettings();

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

        lfns::LFNSSettings settings_lac = getLacSettings();
        lfns::LFNSSettings settings_bd = getBdSettings();

        lfns::LFNSSettings settings = settings_bd;

        base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
        if (my_rank == 0) { runMaster(settings, rng); }
        else { runWorker(settings, rng); }
    } catch (const std::exception &e) {
        std::cerr << "Failed to run LFNS, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
}


void runMaster(lfns::LFNSSettings settings, base::RngPtr rng) {

    lfns::mpi::LFNSMpi lfns(settings, rng, num_tasks);
    if (!settings.previous_log_file.empty()) { lfns.resumeRum(settings.previous_log_file); }
    lfns.runLFNS();
}

void runWorker(lfns::LFNSSettings settings, base::RngPtr rng) {

    models::ChemicalReactionNetwork dynamics(settings.model_file);
    dynamics.setInputParameterOrder(settings.getUnfixedParameters());

    models::InitialValueData init_data(settings.initial_value_file);
    models::InitialValueProvider init_value(rng, init_data);
    init_value.setInputParameterOrder(settings.getUnfixedParameters());


    models::MeasurementModelData measure_data(settings.measurement_file);
    models::MeasurementModel measurement(rng, measure_data);
    measurement.setInputParameterOrder(settings.getUnfixedParameters());
    measurement.setInputStateOrder(dynamics.getSpeciesNames());

    for (int i = 0; i < settings.parameters.size(); i++) {
        lfns::ParameterSetting setting = settings.parameters[i];
        if (setting.fixed) {
            if (dynamics.isParameter(setting.name)) { dynamics.fixParameter(setting.name, setting.fixed_value); }
            if (init_value.isParameter(setting.name)) {
                init_value.fixParameter(setting.name, setting.fixed_value);
            }
            if (measurement.isParameter(setting.name)) {
                measurement.fixParameter(setting.name, setting.fixed_value);
            }
        }
    }


    simulator::SimulatorSsa simulator_ssa(rng, dynamics.getPropensityFct(), dynamics.getReactionFct(),
                                          dynamics.getNumReactions());
    particle_filter::ParticleFilter part_filter(rng, simulator_ssa.getSimulationFct(), measurement.getLikelihoodFct(),
                                                init_value.getInitialStateFct(), dynamics.getNumSpecies(), settings.H);


    std::string input_times_file_name = base::IoUtils::appendToFileName(settings.data_file, "times");

    std::vector<double> times = base::IoUtils::readVector(input_times_file_name);
    std::vector<std::vector<std::vector<double>>> data = base::IoUtils::readMultiline(settings.data_file,
                                                                                      measurement.getMeasurementNames().size());
    std::vector<std::vector<double> > traj = data[0];

    if (my_rank == 1) {
//        dynamics.printInfo(std::cout);
//        init_value.printInfo(std::cout);
//        measurement.printInfo(std::cout);
    }

    lfns::mpi::LFNSWorker worker(my_rank, settings.getUnfixedParameters().size(),
                                 part_filter.getLikelihoodEvaluationForData(&traj, &times));

    simulator_ssa.addStoppingCriterion(worker.getStoppingFct());
    part_filter.addStoppingCriterion(worker.getStoppingFct());
    worker.run();
}

lfns::LFNSSettings getLacSettings() {
    lfns::LFNSSettings settings;
    settings.parameters.push_back(lfns::ParameterSetting("theta_1", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_2", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_3", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_4", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_5", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_6", {1e-2, 3000}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_7", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_8", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_9", {1e-5, 1}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_10", {1e-2, 500}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_11", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_12", {1e-2, 500}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_13", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_14", {1e-5, 1}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_15", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_16", {1e-2, 50}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_17", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("theta_18", {1e-5, 10}));
    settings.parameters.push_back(lfns::ParameterSetting("IPTG", {1e-5, 10}, true, true, 10));
    settings.parameters.push_back(lfns::ParameterSetting("fl_mean", {1e-5, 100}, true, true, 22));
    settings.parameters.push_back(lfns::ParameterSetting("fl_sigma", {1e-5, 100}, true, true, 5));

    settings.uniform_prior = true;
    settings.estimator = lfns::REJECT_DPGMM;
    settings.model_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_model.txt";
    settings.output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lac_output/lac.txt";
    settings.initial_value_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_initial_states.txt";
    settings.measurement_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_measurement.txt";
    settings.data_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_data.txt";
    settings.log_termination = -4.6;
    settings.N = 1000;
    settings.r = 100;
    settings.H = 200;

    return settings;
}

lfns::LFNSSettings getBdSettings() {
    lfns::LFNSSettings settings;
    settings.parameters.push_back(lfns::ParameterSetting("k", {0.6310, 2.5119}));
    settings.parameters.push_back(lfns::ParameterSetting("gamma", {0.6310, 2.5119}, true, true, 0.1));

    settings.uniform_prior = true;
    settings.estimator = lfns::REJECT_DPGMM;
    settings.model_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_model.txt";
    settings.output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_output/bd_cont.txt";
//    settings.previous_log_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_output/bd_log_file.txt";
    settings.initial_value_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_initial.txt";
    settings.measurement_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_measurement.txt";
    settings.data_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_data.txt";
    settings.log_termination = std::log(0.01);
    settings.N = 50;
    settings.r = 8;
    settings.H = 1000;

    return settings;
}
