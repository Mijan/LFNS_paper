//
// Created by jan on 31/10/18.
//

#ifndef LFNS_EVALUATELIKELIHOODSETUP_H
#define LFNS_EVALUATELIKELIHOODSETUP_H


#include "src/particle_filter/ParticleFilter.h"
#include "src/models/FullModel.h"
#include "src/simulator/Simulator.h"
#include "src/simulator/SimulationSettings.h"
#include "src/io/IoSettings.h"
#include "src/particle_filter/ParticleFilterSettings.h"
#include "src/particle_filter/MultLikelihoodEval.h"
#include "src/options/ComputeLikelihoodOptions.h"
#include "src/io/ConfigFileInterpreter.h"

typedef std::vector<double> Times;
typedef std::vector<std::vector<double>> Trajectory;
typedef std::vector<Trajectory> TrajectorySet;

class EvaluateLikelihoodSetup {
public:
    std::vector<simulator::Simulator_ptr> simulators;
    std::vector<particle_filter::ParticleFilter_ptr> particle_filters;
    std::vector<models::FullModel_ptr> full_models;
    std::vector<Times> times_vec;
    std::vector<TrajectorySet> data_vec;

    simulator::SimulationSettings simulation_settings;
    io::IoSettings io_settings;
    models::ModelSettings model_settings;
    particle_filter::ParticleFilterSettings particle_filter_settings;

    base::RngPtr rng;
    particle_filter::MultLikelihoodEval mult_like_eval;
    std::vector<std::vector<double> > parameters;

    std::vector<double>  parameter;
    std::string parameter_file = "";

    std::vector<std::string> experiments;
    int num_computations = 1;


    void setUp(options::ComputeLikelihoodOptions &options);

    void readSettingsfromFile(options::ComputeLikelihoodOptions &options);

    TrajectorySet
    createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings);

    Times createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings);

    simulator::Simulator_ptr
    createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
                    simulator::SimulationSettings &settings);

    void createParameterVector();

    void printSettings(std::ostream &os);

private:
    void _readModelSettings(io::ConfigFileInterpreter &interpreter, std::vector<std::string> experiments);

    void _readParticleFilterSettings(io::ConfigFileInterpreter &interpreter, options::ComputeLikelihoodOptions &options);

    void _setInputData(models::ModelSettings &model_settings, io::ConfigFileInterpreter &interpreter);
};


#endif //LFNS_EVALUATELIKELIHOODSETUP_H
