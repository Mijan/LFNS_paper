//
// Created by jan on 31/10/18.
//

#ifndef LFNS_EVALUATELIKELIHOODSETUP_H
#define LFNS_EVALUATELIKELIHOODSETUP_H


#include "src/particle_filter/ParticleFilter.h"
#include "src/models/FullModel.h"
#include "src/simulator/Simulator.h"
#include "src/io/IoSettings.h"
#include "src/particle_filter/ParticleFilterSettings.h"
#include "src/particle_filter/MultLikelihoodEval.h"
#include "src/options/ComputeLikelihoodOptions.h"
#include "src/io/ConfigFileInterpreter.h"
#include "GeneralSetup.h"

typedef std::vector<double> Times;
typedef std::vector<std::vector<double>> Trajectory;
typedef std::vector<Trajectory> TrajectorySet;

class LikelihoodSetup : public GeneralSetup {
public:

    LikelihoodSetup(options::ComputeLikelihoodOptions options);

    virtual ~LikelihoodSetup();

    std::vector<particle_filter::ParticleFilter_ptr> particle_filters;
    particle_filter::ParticleFilterSettings particle_filter_settings;
    particle_filter::MultLikelihoodEval mult_like_eval;

    std::vector<Times> times_vec;
    std::vector<TrajectorySet> data_vec;

    std::vector<std::vector<double> > parameters;
    std::vector<double> parameter;
    std::string parameter_file = "";

    int num_computations = 1;


    void setUp();

    void printSettings(std::ostream &os) override;

private:
    options::ComputeLikelihoodOptions _likelihood_options;

    void _readSettingsfromFile() override;

    std::vector<std::string> _readExperiments() override;

    TrajectorySet
    createData(int num_outputs, std::string experiment, particle_filter::ParticleFilterSettings &settings);

    Times _createDataTimes(std::string experiment, particle_filter::ParticleFilterSettings &settings);

    void _createParameterVector();

    particle_filter::ParticleFilterSettings _readParticleFilterSettings();
};


#endif //LFNS_EVALUATELIKELIHOODSETUP_H
