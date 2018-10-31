//
// Created by jan on 28/10/18.
//

#ifndef LFNS_LFNSSETUP_H
#define LFNS_LFNSSETUP_H


#include "src/simulator/Simulator.h"
#include "src/particle_filter/ParticleFilter.h"
#include "src/models/FullModel.h"
#include "src/LFNS/LFNSSettings.h"
#include "src/options/LFNSOptions.h"
#include "src/LFNS/LFNS.h"
#include "src/io/IoSettings.h"
#include "src/particle_filter/ParticleFilterSettings.h"
#include "src/sampler/SamplerSettings.h"
#include "src/particle_filter/MultLikelihoodEval.h"
#include "src/io/ConfigFileInterpreter.h"

typedef std::vector<double> Times;
typedef std::vector<std::vector<double>> Trajectory;
typedef std::vector<Trajectory> TrajectorySet;

class LFNSSetup {
public:
    std::vector<simulator::Simulator_ptr> simulators;
    std::vector<particle_filter::ParticleFilter_ptr> particle_filters;
    std::vector<models::FullModel_ptr> full_models;
    std::vector<Times> times_vec;
    std::vector<TrajectorySet> data_vec;

    lfns::LFNSSettings lfns_settings;
    simulator::SimulationSettings simulation_settings;
    io::IoSettings io_settings;
    models::ModelSettings model_settings;
    particle_filter::ParticleFilterSettings particle_filter_settings;
    sampler::SamplerSettings sampler_settings;

    base::RngPtr rng;
    particle_filter::MultLikelihoodEval mult_like_eval;


    void setUp(options::LFNSOptions &options);

    void readSettingsfromFile(options::LFNSOptions &options);

    TrajectorySet
    createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings);

    Times createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings);

    simulator::Simulator_ptr
    createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics,
                    simulator::SimulationSettings &settings);

    void printSettings(std::ostream &os);

private:
    models::ModelSettings _readModelSettings(io::ConfigFileInterpreter &interpreter, std::vector<std::string> experiments);

    lfns::LFNSSettings _readLFNSSettings(io::ConfigFileInterpreter &interpreter, options::LFNSOptions &options);
};


#endif //LFNS_LFNSSETUP_H
