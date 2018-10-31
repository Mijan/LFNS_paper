//
// Created by jan on 22/10/18.
//

#include "src/options/SimulationOptions.h"
#include "SimulationSetup.h"
#include "src/base/IoUtils.h"

typedef std::vector<double> Times;
typedef std::vector<double> State;
typedef std::vector<State> Trajectory;
typedef std::vector<Trajectory> TrajectorySet;

static std::string model_summary_suffix = "model_summary";
static std::stringstream summary_stream;


static std::string latent_states_suffix = "latent_states";
static std::string measurement_suffix = "measurements";
static std::string times_suffix = "times";

options::SimulationOptions simulation_options;
SimulationSetup simulation_setup;

int simulate();

int main(int argc, char **argv) {
    try {
        simulation_options.handleCommandLineOptions(argc, argv);
        simulation_setup.setUp(simulation_options);

        simulation_setup.printSettings(summary_stream);
        std::cout << summary_stream.str();
        return simulate();
    } catch (const std::exception &e) {
        std::cerr << "Failed to run simulate, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
}

int simulate() {

    std::string model_summary_file_name = base::IoUtils::appendToFileName(simulation_setup.io_settings.output_file,
                                                                          model_summary_suffix);
    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
    model_summary_file_stream << summary_stream.str();
    model_summary_file_stream.close();

    std::string times_file_name = base::IoUtils::appendToFileName(simulation_setup.io_settings.output_file, times_suffix);
    base::IoUtils::writeVector(times_file_name, simulation_setup.times);

    for (int exp_nbr = 0; exp_nbr < simulation_setup.simulation_settings.experiments_for_simulation.size(); exp_nbr++) {
        std::string experiment = simulation_setup.simulation_settings.experiments_for_simulation[exp_nbr];
        simulator::Simulator_ptr simulator = simulation_setup.simulators[exp_nbr];
        models::FullModel_ptr full_model = simulation_setup.full_models[exp_nbr];

        double t = 0.0;
        State latentstate(full_model->dynamics->getNumSpecies(), 0.0);
        State measurement(full_model->measurement_model->getNumMeasurements(), 0.0);

        TrajectorySet latent_state_traj;
        latent_state_traj.reserve(simulation_setup.parameters.size() * simulation_setup.simulation_settings.n);

        TrajectorySet measurement_traj;
        measurement_traj.reserve(simulation_setup.parameters.size() * simulation_setup.simulation_settings.n);


        for (std::vector<double> &param : simulation_setup.parameters) {
            full_model->setParameter(param);
            for (int sim_nbr = 0; sim_nbr < simulation_setup.simulation_settings.n; sim_nbr++) {
                Trajectory latent_states;
                latent_states.reserve(simulation_setup.times.size());
                Trajectory measurements;
                measurements.reserve(simulation_setup.times.size());

                full_model->initial_value_provider->computeInitialState(&latentstate, &t);
                simulator->reset(latentstate, t);
                for (double sim_time : simulation_setup.times) {
                    simulator->simulate(sim_time);
                    full_model->measurement_model->computeMeasurement(&measurement, latentstate, t);

                    latent_states.push_back(latentstate);
                    measurements.push_back(measurement);
                }
                latent_state_traj.push_back(latent_states);
                measurement_traj.push_back(measurements);
            }
        }

        std::string experiment_file_name = base::IoUtils::appendToFileName(simulation_setup.io_settings.output_file,
                                                                           experiment);
        std::string latent_file_name = base::IoUtils::appendToFileName(experiment_file_name, latent_states_suffix);
        std::ofstream latent_file(latent_file_name.c_str());
        latent_file << "";
        latent_file.close();
        for (Trajectory &latent_traj : latent_state_traj) {
            base::IoUtils::writeVectorOfVectors(latent_file_name, latent_traj, true, std::ios_base::app);
        }

        std::string measurement_file_name = base::IoUtils::appendToFileName(experiment_file_name, measurement_suffix);
        std::ofstream measurement_file(measurement_file_name.c_str());
        measurement_file << "";
        measurement_file.close();
        for (Trajectory &measurement : measurement_traj) {
            base::IoUtils::writeVectorOfVectors(measurement_file_name, measurement, true, std::ios_base::app);
        }
    }

    std::cout << "\n\nSimulation successfully finished!" << std::endl;
    return 1;
}