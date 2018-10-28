#include <iostream>
#include <vector>
#include "src/base/EigenMatrices.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/base/Utils.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/particle_filter/ParticleFilter.h"
#include "src/models/ChemicalReactionNetwork.h"
#include "src/models/InitialValueData.h"
#include "src/models/InitialValueProvider.h"
#include "src/models/MeasurementModel.h"
#include "src/base/MathUtils.h"

using namespace std::placeholders;

std::vector<double> pulse_start = {15.0, 55.0};
std::vector<double> pulse_end = {30, 70};
std::vector<double> pulse_strength = {2, -0.5};


std::vector<double> theta = {1, 0.1};


void simulate_ode_BD(std::vector<double> &state, double &t, double final_t, const std::vector<double> &theta,
                     simulator::SimulatorOde_ptr simulator) {
    simulator->reset(state, t);
    simulator->continueSimulate(final_t, theta);
}

double
loglikelihood_BD(const std::vector<double> &state, const std::vector<double> &data, const std::vector<double> &theta) {
    double log_like = -2 * (std::abs(state[0] - data[0])) - 0.5 * std::log(2 * M_PI);
    return log_like;
}

void initial_state_BD(std::vector<double> *state_0, double *t_0, const std::vector<double> &theta) {
    (*state_0)[0] = 0;
    *t_0 = 0;
}

double root_BD(std::vector<double> &state, double t) {
    double r = 1;
    for (int i = 0; i < pulse_strength.size(); i++) {
        r *= ((t - pulse_start[i]) * (t - pulse_end[i])) / 70.0;
    }
    return r;
}

void rhs_BD(double *dx, double *state, double t, const std::vector<double> &theta) {
    double perturb = 0;
    for (int i = 0; i < pulse_strength.size(); i++) {
        if (t >= pulse_start[i] && t < pulse_end[i]) {
            perturb = pulse_strength[i];
        }
    }
    double k = theta[0] + perturb;
    double gamma = theta[1];

    double mRNA = state[0];

    dx[0] = k - gamma * mRNA;
}

void propensity_BD(double *propensities, std::vector<double> &state, double t, const std::vector<double> &theta) {
    double perturb = 0;
    for (int i = 0; i < pulse_strength.size(); i++) {
        if (t >= pulse_start[i] && t < pulse_end[i]) {
            perturb = pulse_strength[i];
        }
    }

    double k = theta[0] + perturb;
    double gamma = theta[1];

    int mRNA = state[0];

    propensities[0] = k;
    propensities[1] = mRNA * gamma;
}

void reactions_BD(std::vector<double> &state, int reaction_index) {
    switch (reaction_index) {
        case 0: {
            state[0]++;
            break;
        }
        case 1: {
            state[0]--;
            break;
        }
    }
}

int testSsaParticleFilter(std::string input_file) {

//    std::string input_times_ssa_file_name = base::IoUtils::appendToFileName(input_file, "times_ssa");
//    std::string input_state_ssa_file_name = base::IoUtils::appendToFileName(input_file, "measure_ssa");
//
//    std::vector<double> times = base::IoUtils::readVector(input_times_ssa_file_name);
//    std::vector<double> data = base::IoUtils::readVector(input_state_ssa_file_name);
//
//    std::vector<std::vector<double> > data_vec;
//    for (double &d : data) {
//        data_vec.push_back({d});
//    }
//
//    simulator::PropensityFct_ptr prop_fct = std::make_shared<simulator::PropensityFct>(propensity_BD);
//    simulator::ReactionFct_ptr reaction_fct = std::make_shared<simulator::ReactionFct>(reactions_BD);
//    simulator::RootFct_ptr root_fct = std::make_shared<simulator::RootFct>(root_BD);
//
//    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
//
//    simulator::SimulatorSsa simulator_ssa(rng, prop_fct, reaction_fct, 2);
//    simulator_ssa.setRootFunction(root_fct);
//
//
//    particle_filter::SimulationFct sim_fct = std::bind(&simulator::SimulatorSsa::simulate, &simulator_ssa, _1, _2, _3,
//                                                       _4);
//    particle_filter::SimulationFct_ptr sim_fct_ptr = std::make_shared<particle_filter::SimulationFct>(sim_fct);
//    particle_filter::LikelihoodFct_ptr likelihood_fct = std::make_shared<particle_filter::LikelihoodFct>(
//            loglikelihood_BD);
//    particle_filter::InitialStateFct_ptr initial_state_fct = std::make_shared<particle_filter::InitialStateFct>(
//            initial_state_BD);
//
//    particle_filter::ParticleFilter particle_filter(rng, sim_fct_ptr, likelihood_fct, initial_state_fct, 1, 100);
//
//    for (int i = 0; i < 1; i++) {
//        std::cout << "log likelihood ssa: " << particle_filter.computeLogLikelihood(data_vec, times, theta)
//                  << std::endl;
//    }

    return 1;
}

int testOdeParticleFilter(std::string input_file) {
    simulator::RootFct_ptr root_fct = std::make_shared<simulator::RootFct>(root_BD);

    std::vector<double> state = {0};
    double t = 0;

    std::string input_times_ode_file_name = base::IoUtils::appendToFileName(input_file, "times_ode");
    std::string input_state_ode_file_name = base::IoUtils::appendToFileName(input_file, "measure_ode");

    std::vector<double> times = base::IoUtils::readVector(input_times_ode_file_name);
    std::vector<double> data = base::IoUtils::readVector(input_state_ode_file_name);
    std::vector<std::vector<double> > data_vec;
    for (double &d : data) {
        data_vec.push_back({d});
    }

//    simulator::RhsFct_ptr rhs_fct = std::make_shared<simulator::RhsFct>(rhs_BD);
//    simulator::OdeSettings settings;
//    simulator::SimulatorOde_ptr simulator_ode_ptr = std::make_shared<simulator::SimulatorOde>(settings, rhs_fct, 1);
//    simulator_ode_ptr->setRootFunction(root_fct);
//
//    simulator_ode_ptr->initialize(state, t);
//
//    particle_filter::SimulationFct sim_fct = std::bind(simulate_ode_BD, _1, _2, _3, _4, simulator_ode_ptr);
//    particle_filter::SimulationFct_ptr sim_fct_ptr = std::make_shared<particle_filter::SimulationFct>(sim_fct);
//    particle_filter::LikelihoodFct_ptr likelihood_fct = std::make_shared<particle_filter::LikelihoodFct>(
//            loglikelihood_BD);
//    particle_filter::InitialStateFct_ptr initial_state_fct = std::make_shared<particle_filter::InitialStateFct>(
//            initial_state_BD);
//
//    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
//
//    particle_filter::ParticleFilter particle_filter(rng, sim_fct_ptr, likelihood_fct, initial_state_fct, 1, 1);
//
//    for (int i = 0; i < 1; i++) {
//        std::cout << "log likelihood ode: " << particle_filter.computeLogLikelihood(data_vec, times, theta)
//                  << std::endl;
//    }
//    return 1;
}


int testLacGfp() {
    std::string model_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_model.txt";
    std::string output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lfns_output.txt";
    std::string initial_value_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_initial_states.txt";
    std::string measurement_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_measurement.txt";
    std::string data_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_data.txt";

    models::ChemicalReactionNetwork lacgfp(model_file);
    lacgfp.fixParameter("IPTG", 10);
    lacgfp.setParameterOrder(lacgfp.getUnfixedParameterNames());


    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

    simulator::SimulatorSsa simulator_ssa(rng, lacgfp.getPropensityFct(), lacgfp.getReactionFct(),
                                          lacgfp.getNumReactions());

    models::InitialValueData init_data(initial_value_file);
    models::InitialValueProvider lac_init_value(rng, init_data);
    lac_init_value.setParameterOrder(lacgfp.getUnfixedParameterNames());
    lac_init_value.setStateOrder(lacgfp.getSpeciesNames());


    models::MeasurementModelData measure_data(measurement_file);
    models::MeasurementModel lac_measure(rng, measure_data);
    lac_measure.fixParameter("fl_mean", 22);
    lac_measure.fixParameter("fl_sigma", 5);
    lac_measure.setParameterOrder(lacgfp.getUnfixedParameterNames());
    lac_measure.setStateOrder(lacgfp.getSpeciesNames());

    particle_filter::ParticleFilter part_filter(rng, simulator_ssa.getSimulationFct(), lac_measure.getLikelihoodFct(),
                                                lac_init_value.getInitialStateFct(), lacgfp.getNumSpecies(), 1000);


    std::string input_times_file_name = base::IoUtils::appendToFileName(data_file, "times");

    std::vector<double> times = base::IoUtils::readVector(input_times_file_name);
    std::vector<double> data = base::IoUtils::readVector(data_file);

    std::vector<std::vector<std::vector<double> > > data_vec = base::IoUtils::readMultiline(data_file, 2);


    lacgfp.printInfo(std::cout);
    lac_measure.printInfo(std::cout);


    std::vector<double> theta = {1.5, 7.5, 1.5, 4.5, 5, 1650, 6, 0.48, 0.5, 230, 0.4, 125, 0.2, 0.01, 1.5, 32, 1, 2.2};
    std::cout << "\n\nLog-Likelihoods" << std::endl;
    time_t tic = clock();
    int num_runs = 100;
    std::vector<double> log_likes(num_runs);
    for (int num_run = 0; num_run < num_runs; num_run++) {
        double log_like = part_filter.computeLogLikelihood(data_vec[0], times, theta);
        std::cout << log_like << std::endl;
        log_likes[num_run] = log_like;
    };
    time_t toc = clock();
    std::cout << "Total time: " << base::Utils::getTimeFromClocks(toc - tic) << std::endl;

    double log_mean = base::MathUtils::sumOfLog(log_likes) - std::log(num_runs);

    double log_std = 0;
    for (int num_run = 0; num_run < num_runs; num_run++) {
        log_std += std::pow((std::exp(log_likes[num_run] - log_mean) - 1), 2);
    }
    log_std = log_std / (double) (num_runs - 1);
    log_std = std::log(log_std) + 2 * log_mean;
    std::cout << "log-mean:\t" << log_mean << "; log-stddev:\t" << 0.5 * log_std << std::endl;
    return 1;
}

int testBD() {
    std::string model_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_model.txt";
    std::string output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_output.txt";
    std::string initial_value_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_initial.txt";
    std::string measurement_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_measurement.txt";
    std::string data_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_data.txt";

    models::ChemicalReactionNetwork bd_model(model_file);
    bd_model.setParameterOrder(bd_model.getUnfixedParameterNames());


    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

    simulator::SimulatorSsa simulator_ssa(rng, bd_model.getPropensityFct(), bd_model.getReactionFct(),
                                          bd_model.getNumReactions());

    models::InitialValueData init_data(initial_value_file);
    models::InitialValueProvider bd_init_value(rng, init_data);
    bd_init_value.setParameterOrder(bd_model.getUnfixedParameterNames());
    bd_init_value.setStateOrder(bd_model.getSpeciesNames());


    models::MeasurementModelData measure_data(measurement_file);
    models::MeasurementModel bd_measure(rng, measure_data);
    bd_measure.setParameterOrder(bd_model.getUnfixedParameterNames());
    bd_measure.setStateOrder(bd_model.getSpeciesNames());

    particle_filter::ParticleFilter part_filter(rng, simulator_ssa.getSimulationFct(), bd_measure.getLikelihoodFct(),
                                                bd_init_value.getInitialStateFct(), bd_model.getNumSpecies(), 200);


    std::string input_times_file_name = base::IoUtils::appendToFileName(data_file, "times");

    std::vector<double> times = base::IoUtils::readVector(input_times_file_name);
    std::vector<double> data = base::IoUtils::readVector(data_file);

    std::vector<std::vector<double> > data_vec;
    for (double &d : data) {
        data_vec.push_back({d});
        std::cout << "data point: " << d << std::endl;
    }


    bd_model.printInfo(std::cout);
    bd_measure.printInfo(std::cout);


    std::vector<double> theta = {1.0, 0.1};
    std::cout << "\n\nLog-Likelihoods" << std::endl;
    time_t tic = clock();
    int num_runs = 100;
    std::vector<double> log_likes(num_runs);
    for (int num_run = 0; num_run < num_runs; num_run++) {
        double log_like = part_filter.computeLogLikelihood(data_vec, times, theta);
        std::cout << log_like << std::endl;
        log_likes[num_run] = log_like;
    };
    time_t toc = clock();
    std::cout << "Total time: " << base::Utils::getTimeFromClocks(toc - tic) << std::endl;

    double log_mean = base::MathUtils::sumOfLog(log_likes) - std::log(num_runs);

    double log_std = 0;
    for (int num_run = 0; num_run < num_runs; num_run++) {
        log_std += std::pow((std::exp(log_likes[num_run] - log_mean) - 1), 2);
    }
    log_std = log_std / (double) (num_runs - 1);
    log_std = std::log(log_std) + 2 * log_mean;
    std::cout << "log-mean:\t" << log_mean << "; log-stddev:\t" << 0.5 * log_std << std::endl;
    return 1;
}

int main() {

    std::string input_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_simulation.txt";

//    int ode = testOdeParticleFilter(input_file);
//    int ssa = testSsaParticleFilter(input_file);
    int lac = testLacGfp();

    return 1;
}