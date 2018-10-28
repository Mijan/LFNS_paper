//
// Created by jan on 22/10/18.
//

#include "src/models/ChemicalReactionNetwork.h"
#include "src/models/InitialValueProvider.h"
#include "src/models/MeasurementModel.h"
#include "src/simulator/SimulatorOde.h"

int num_simulations = 1;
double interval = 2;
double final_time = 100;

std::string model_file = "/home/jan/crypt/Dropbox/BSSE/Transvac/models/tcell_model.txt";
std::string initial_value_file = "/home/jan/crypt/Dropbox/BSSE/Transvac/models/tcell_initial.txt";
std::string measurement_file = "/home/jan/crypt/Dropbox/BSSE/Transvac/models/tcell_measurement.txt";
int main() {



    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

    models::ChemicalReactionNetwork dynamics(model_file);
    dynamics.setParameterOrder(dynamics.getParameterNames());

    models::InitialValueData init_data(initial_value_file);
    models::InitialValueProvider init_value(init_data);
    init_value.setParameterOrder(dynamics.getParameterNames());


    models::MeasurementModelData measure_data(measurement_file);
    models::MeasurementModel measurement(rng, measure_data);
    measurement.setParameterOrder(dynamics.getParameterNames());
    measurement.setStateOrder(dynamics.getSpeciesNames());

//        simulator::RhsFct_ptr rhs_fct = std::make_shared<simulator::RhsFct>(rhs_BD);
    simulator::OdeSettings settings;
    simulator::SimulatorOde simulator_ode(settings, dynamics.getRhsFct(), dynamics.getNumSpecies());

    std::vector<double> current_state = std::vector<double>(dynamics.getNumSpecies(), 0.0);
    double current_time = 0;
    simulator_ode.initialize(current_state, current_time);
//    simulator_ode.setRootFunction(root_fct);


    std::vector<double> theta = {};
    for(int i = 0; i< num_simulations; i++) {
        std::vector<double> current_state = std::vector<double>(dynamics.getNumSpecies(), 0.0);
        double current_time = 0;
        init_value.computeInitialState(&current_state, &current_time, theta);

        simulator_ode.reset(current_state, current_time);

        std::vector<double> time_ode;
        std::vector<double> states_ode;
        std::vector<double> measurement_ode;
        double t = current_time;

        base::NormalDistribution dist(0, 1);
        while (current_time < final_time) {
            simulator_ode.simulate(t, theta);
            time_ode.push_back(current_time);
            states_ode.push_back(current_state[0]);
            double rnd = dist(*rng);

            measurement_ode.push_back(current_state[0] + rnd);
            t += interval;
        }

//        std::string output_times_ode_file_name = base::IoUtils::appendToFileName(output_file, "times_ode");
//        std::string output_state_ode_file_name = base::IoUtils::appendToFileName(output_file, "state_ode_2");
//        std::string output_measure_ode_file_name = base::IoUtils::appendToFileName(output_file, "measure_ode");
    }

}