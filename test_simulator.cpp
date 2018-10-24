#include <iostream>
#include <vector>
#include "src/base/EigenMatrices.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/base/Utils.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/base/MathUtils.h"


std::vector<double> pulse_start = {15.0, 55.0};
std::vector<double> pulse_end = {30, 70};
std::vector<double> pulse_strength = {2, -0.5};

std::vector<double> theta = {1, 0.1};
double initial_time = 0;
double final_time = 100;

double root_BD(std::vector<double> &state, double t) {
    double r = 1;
    for (int i = 0; i < pulse_strength.size(); i++) {
        r *= (t - pulse_start[i]) * (t - pulse_end[i]) / 70.0;
    }
    return r;
}

void rhs_BD(double *dx, const double *state, double t, const std::vector<double> &theta) {
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

void propensity_BD(std::vector<double> &propensities, std::vector<double> &state, double t,
                   const std::vector<double> &theta) {
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

int testSsaSimulator(std::string output_file) {

    simulator::PropensityFct_ptr prop_fct = std::make_shared<simulator::PropensityFct>(propensity_BD);
    simulator::ReactionFct_ptr reaction_fct = std::make_shared<simulator::ReactionFct>(reactions_BD);
    simulator::RootFct_ptr root_fct = std::make_shared<simulator::RootFct>(root_BD);

    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

    simulator::SimulatorSsa simulator_ssa(rng, prop_fct, reaction_fct, 2);
    simulator_ssa.setRootFunction(root_fct);

    std::vector<double> current_state = {0};
    double current_time = initial_time;


    std::vector<double> time_ssa;
    std::vector<double> states_ssa;
    std::vector<double> measurement_ssa;
    base::NormalDistribution dist(0, 1);
    while (current_time < final_time) {
        simulator_ssa.simulateReaction(current_state, current_time, final_time, theta);
        time_ssa.push_back(current_time);
        states_ssa.push_back(current_state[0]);

        double rnd = dist(*rng);
        measurement_ssa.push_back(current_state[0] + rnd);
    }


    std::string output_times_ssa_file_name = base::IoUtils::appendToFileName(output_file, "times_ssa");
    std::string output_state_ssa_file_name = base::IoUtils::appendToFileName(output_file, "state_ssa");
    std::string output_measure_ssa_file_name = base::IoUtils::appendToFileName(output_file, "measure_ssa");
    base::IoUtils::writeVector<double>(output_times_ssa_file_name, time_ssa, " ");
    base::IoUtils::writeVector<double>(output_state_ssa_file_name, states_ssa, " ");
    base::IoUtils::writeVector<double>(output_measure_ssa_file_name, measurement_ssa, " ");
    return 1;
}

int testReadModel(std::string output_file) {

    simulator::RootFct_ptr root_fct = std::make_shared<simulator::RootFct>(root_BD);

    std::vector<double> current_state = {0};
    double current_time = initial_time;

    simulator::RhsFct_ptr rhs_fct = std::make_shared<simulator::RhsFct>(rhs_BD);
    simulator::OdeSettings settings;
    simulator::SimulatorOde simulator_ode(settings, rhs_fct, 1);

    simulator_ode.initialize(current_state, current_time);
    simulator_ode.setRootFunction(root_fct);

    std::vector<double> time_ode;
    std::vector<double> states_ode;
    std::vector<double> measurement_ode;
    double t = initial_time;
    double interval = 2;


    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
    base::NormalDistribution dist(0, 1);
    while (current_time < final_time) {
        simulator_ode.continueSimulate(t, theta);
        time_ode.push_back(current_time);
        states_ode.push_back(current_state[0]);
        double rnd = dist(*rng);

        measurement_ode.push_back(current_state[0] + rnd);
        t += interval;
    }

    std::string output_times_ode_file_name = base::IoUtils::appendToFileName(output_file, "times_ode");
    std::string output_state_ode_file_name = base::IoUtils::appendToFileName(output_file, "state_ode_2");
    std::string output_measure_ode_file_name = base::IoUtils::appendToFileName(output_file, "measure_ode");
    base::IoUtils::writeVector<double>(output_times_ode_file_name, time_ode, " ");
    base::IoUtils::writeVector<double>(output_state_ode_file_name, states_ode, " ");
    base::IoUtils::writeVector<double>(output_measure_ode_file_name, measurement_ode, " ");
    return 1;
}

int main() {

    std::string output_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/bd_simulation.txt";

    int ode = testReadModel(output_file);
    int ssa = testSsaSimulator(output_file);

    return ode * ssa;
}