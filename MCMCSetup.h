//
// Created by jan on 01/02/19.
//

#ifndef MCMCSETUP_H
#define MCMCSETUP_H

#include "GeneralSetup.h"
#include "src/options/MCMCOptions.h"
#include "src/particle_filter/ParticleFilter.h"
#include "src/particle_filter/MultLikelihoodEval.h"
#include "src/particle_filter/ParticleFilterSettings.h"
#include "src/sampler/SamplerSettings.h"
#include "src/MCMC/MCMCSettings.h"

typedef std::vector<double> Times;
typedef std::vector<std::vector<double>> Trajectory;
typedef std::vector<Trajectory> TrajectorySet;


class MCMCSetup : public GeneralSetup {
public:
    MCMCSetup(options::MCMCOptions options);

    virtual ~MCMCSetup();

    std::vector<particle_filter::ParticleFilter_ptr> particle_filters;
    particle_filter::MultLikelihoodEval mult_like_eval;

    particle_filter::ParticleFilterSettings particle_filter_settings;
    sampler::SamplerSettings sampler_settings;

    mcmc::MCMCSettings mcmc_settings;

    std::vector<Times> times_vec;
    std::vector<TrajectorySet> data_vec;


    void setUp();

    void printSettings(std::ostream &os) override;

private:
    options::MCMCOptions _mcmc_options;

    void _readSettingsfromFile() override;

    std::vector<std::string> _readExperiments() override;

    TrajectorySet
    _createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings);

    Times _createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings);

    particle_filter::ParticleFilterSettings _readParticleFilterSettings();

    sampler::SamplerSettings _readSamplerSettings();
};


#endif //MCMCSETUP_H
