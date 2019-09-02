//
// Created by jan on 11/10/18.
//

#include <cfloat>
#include <iostream>
#include <sstream>
#include <muParserError.h>
#include "ParticleFilter.h"
#include "../simulator/SimulatorExceptions.h"
#include "../models/ModelExceptions.h"

namespace particle_filter {
    using namespace std::placeholders;

    ParticleFilter::ParticleFilter(base::RngPtr rng, SetParameterFct_ptr setting_parameter_fc,
                                   SimulationFct_ptr simulation_fct, ResetFct_ptr _reset_fct,
                                   LikelihoodFct_ptr likelihood_fct, InitialStateFct_ptr initial_state_fct,
                                   int num_states, int num_particles) : _rng(rng),
                                                                        _setting_parameter_fct(setting_parameter_fc),
                                                                        _simulation_fct(simulation_fct),
                                                                        _reset_fct(_reset_fct),
                                                                        _likelihood_fct(likelihood_fct),
                                                                        _initial_state_fct(initial_state_fct),
                                                                        _smc_particles(
                                                                                std::make_shared<SmcParticleSet>(rng)),
                                                                        _log_likelihoods_tmps(num_particles),
                                                                        _threshold_ptr(nullptr) {
        _smc_particles->setTotalNumParticles(num_particles, num_states);
    }

    ParticleFilter::~ParticleFilter() {}

    double ParticleFilter::computeLogLikelihood(const std::vector<std::vector<double> > &data,
                                                const std::vector<double> &times, const std::vector<double> &theta) {
        if (times.size() != data.size()) {
            std::stringstream ss;
            ss << "Failed to run ParticleFilter. Provided trajectory has " << data.size()
               << " entries, but provided times have " << times.size() << " entries!" << std::endl;
            throw std::runtime_error(ss.str());
        }

        try {
            (*_setting_parameter_fct)(theta);

            for (int particle_nbr = 0; particle_nbr < _smc_particles->getTotalNumParticles(); particle_nbr++) {
                SmcParticle_ptr particle = _smc_particles->getParticle(particle_nbr);

                (*_initial_state_fct)(&particle->state, &particle->time);
                _smc_particles->setWeight(particle_nbr, 1.0);
            }
            _smc_particles->clearAncestorIndices();
            _smc_particles->resetFullWeight();
            if (_smc_particles->getTotalNumParticles() == 1) {
                return computeLogLikelihoodNoResampling(data, times);
            } else {
                double log_likelihood = 0;
                for (int t = 0; t < times.size(); t++) {
                    log_likelihood += _runFilteringStep(data[t], times[t]);
                    if (_stopping_criterions.processStopped()) { return -DBL_MAX; }
                    if (_threshold_ptr && log_likelihood < *_threshold_ptr) { return -DBL_MAX; }
                    _smc_particles->resampleOldParticles();
                }
                return log_likelihood;
            }
        } catch (mu::ParserError &e) {
            std::ostringstream os;
            os << "Parser error for expression : " << e.GetExpr() << std::endl;
            os << "Message:  " << e.GetMsg() << "\n";
            os << "Formula:  " << e.GetExpr() << "\n";
            os << "Token:    " << e.GetToken() << "\n";
            os << "Position: " << e.GetPos() << "\n";
            os << "Errc:     " << e.GetCode() << "\n";
            throw std::runtime_error(os.str());
        }catch(const simulator::SimulationAborted & e){
            std::cerr << "simulation for paramter ";
            for (double d : theta) { std::cerr  << d << " "; }
            std::cerr << " aborted at timepoint " << e.terminationTime() << std::endl;
            return -DBL_MAX;
        }catch(const models::ModelException & e){
            std::cerr << "Model related error for parameter ";
            for (double d : theta) { std::cerr  << d << " "; }
            std::cerr << " with error mssage:\n\t" << e.what() << std::endl;
            return -DBL_MAX;
        }
    }

    double ParticleFilter::computeLogLikelihoodNoResampling(const std::vector<std::vector<double> > &data,
                                                            const std::vector<double> &times) {
        double log_likelihood = 0;
        SmcParticle_ptr particle = _smc_particles->getParticle(0);
        (*_reset_fct)(particle->state, particle->time);
        for (int t = 0; t < times.size(); t++) {
            double final_time = times[t];
            (*_simulation_fct)(final_time);

            double log_likelihood_part = (*_likelihood_fct)(particle->state, data[t], particle->time);
            log_likelihood += log_likelihood_part;
        }
        return log_likelihood;
    }


    LogLikelihodEvalFct_ptr
    ParticleFilter::getLikelihoodEvaluationForData(const std::vector<std::vector<double> > *data,
                                                   const std::vector<double> *times) {
        return std::make_shared<LogLikelihodEvalFct>(
                std::bind(&ParticleFilter::computeLogLikelihood, this, *data, *times, _1));
    }

    double
    ParticleFilter::_runFilteringStep(const std::vector<double> &data, double final_time) {

        double max_log = -DBL_MAX;

        for (int particle_nbr = 0; particle_nbr < _smc_particles->getTotalNumParticles(); particle_nbr++) {
            SmcParticle_ptr particle = _smc_particles->getParticle(particle_nbr);
            (*_reset_fct)(particle->state, particle->time);
            (*_simulation_fct)(final_time);

            double log_likelihood_part = (*_likelihood_fct)(particle->state, data, particle->time);

            _log_likelihoods_tmps[particle_nbr] = log_likelihood_part;
            max_log = max_log > log_likelihood_part ? max_log : log_likelihood_part;
            if (_stopping_criterions.processStopped()) {
                return -DBL_MAX;
            }
        }


        double log_likelihood = 0;
        for (int particle_nbr = 0; particle_nbr < _smc_particles->getTotalNumParticles(); particle_nbr++) {
            double norm_like = _log_likelihoods_tmps[particle_nbr] - max_log;
            log_likelihood += std::exp(norm_like);
            _smc_particles->setWeight(particle_nbr, std::exp(norm_like));
        }
        log_likelihood = std::log(log_likelihood) + max_log - std::log(_smc_particles->getTotalNumParticles());
        return log_likelihood;
    }


    void ParticleFilter::setThresholdPtr(double *threshold) { _threshold_ptr = threshold; }

    void ParticleFilter::addStoppingCriterion(simulator::StoppingFct_ptr stopping_criterion) {
        _stopping_criterions.push_back(stopping_criterion);
    }
}