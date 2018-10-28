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
    lfns::LFNSSettings settings;
    base::RngPtr rng;
    lfns::MultLikelihoodEval mult_like_eval;


    void setUp(options::LFNSOptions &options);

    void readSettingsfromFile(options::LFNSOptions &options);

    models::FullModel_ptr createModel(base::RngPtr rng, lfns::LFNSSettings &settings);

    TrajectorySet createData(int num_outputs, std::string experiment, lfns::LFNSSettings &settings);

    Times createDataTimes(std::string experiment, lfns::LFNSSettings &settings);

    simulator::Simulator_ptr
    createSimulator(base::RngPtr rng, models::ChemicalReactionNetwork_ptr dynamics, lfns::LFNSSettings &settings);

    void
    setUpPerturbations(std::string experiment, simulator::Simulator_ptr simulator, models::FullModel_ptr full_model,
                       lfns::LFNSSettings &settings);

};


#endif //LFNS_LFNSSETUP_H
