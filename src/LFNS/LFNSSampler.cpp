//
// Created by jan on 09/10/18.
//

#include "LFNSSampler.h"
#include "../sampler/UniformSampler.h"
#include "../sampler/RejectionSupportSampler.h"
#include "../sampler/DpGmmSampler.h"
#include "../sampler/EllipsoidSampler.h"
#include "../sampler/GaussianSampler.h"
#include "../sampler/KernelSupportEstimation.h"
#include "../particle_filter/ParticleFilterSettings.h"
#include "boost/serialization/shared_ptr.hpp"
#include "../sampler/SliceSampler.h"

namespace lfns {

    LFNSSampler::LFNSSampler(sampler::Sampler_ptr prior, sampler::DensityEstimation_ptr density_estimation,
                             base::RngPtr rng) : _rng(rng), _prior(prior), _density_estimation(density_estimation),
                                                 _max_live_prior_value(0), _dist(), _uniform_prior(true),
                                                 _log_params() {
        //        std::vector<std::string> unfixed_params = settings.param_names;
//        _log_params = settings.getLogParams(unfixed_params);
    }

//    LFNSSampler::LFNSSampler(LFNSSettings &lfns_settings, sampler::SamplerSettings &settings, base::RngPtr rng) : _rng(
//            rng), _prior(), _density_estimation(),
//                                                                                                                  _max_live_prior_value(
//                                                                                                                          0),
//                                                                                                                  _dist(),
//                                                                                                                  _uniform_prior(
//                                                                                                                          true),
//                                                                                                                  _log_params() {
//
//        std::vector<std::string> unfixed_params = settings.param_names;
//        sampler::SamplerData sampler_data(unfixed_params.size());
//        sampler_data.bounds = settings.getBounds(unfixed_params);
//        _log_params = settings.getLogParams(unfixed_params);
//
//        for (int i : _log_params) {
//            sampler_data.bounds[i].first = std::log10(sampler_data.bounds[i].first);
//            sampler_data.bounds[i].second = std::log10(sampler_data.bounds[i].second);
//        }
//
//        if (lfns_settings.uniform_prior) {
//            sampler::UniformSamplerData uni_data(sampler_data);
//            _prior = std::make_shared<sampler::UniformSampler>(_rng, uni_data);
//        }
//
//        switch (lfns_settings.estimator) {
//            case REJECT_DPGMM  : {
//                sampler::DpGmmSamplerData dpgmm_data(sampler_data);
//                dpgmm_data.num_dp_iterations = 50;
//                sampler::DpGmmSampler_ptr dpgmm_sampler = std::make_shared<sampler::DpGmmSampler>(_rng, dpgmm_data);
//
//
//                sampler::RejectionSamplerData rej_data(sampler_data);
//                rej_data.rejection_quantile = lfns_settings.rejection_quantile_for_density_estimation;
//                _density_estimation = std::make_shared<sampler::RejectionSupportSampler>(_rng, dpgmm_sampler, rej_data);
//                break;
//            }
//            case KDE_GAUSS: {
//                sampler::NormalSamplerData normal_data(sampler_data);
//                normal_data.cov = base::EiMatrix::Identity(sampler_data.size(), sampler_data.size()) * 0.1;
//                sampler::GaussianSampler_ptr gauss_kernel = std::make_shared<sampler::GaussianSampler>(_rng,
//                                                                                                       normal_data);
//
//                _density_estimation = std::make_shared<sampler::KernelSupportEstimation>(_rng, gauss_kernel,
//                                                                                         sampler_data);
//                break;
//            }
//            case KDE_UNIFORM: {
//                sampler::UniformSamplerData unif_data(sampler_data);
//                sampler::KernelSampler_ptr unif_kernel = std::make_shared<sampler::UniformSampler>(_rng, unif_data);
//                _density_estimation = std::make_shared<sampler::KernelSupportEstimation>(_rng, unif_kernel,
//                                                                                         sampler_data);
//                break;
//            }
//            case ELLIPS: {
//                _density_estimation = std::make_shared<sampler::EllipsoidSampler>(_rng, sampler_data);
//                break;
//            }
//            case SLICE: {
////                _density_estimation = std::make_shared<sampler::SliceSampler>()
//            }
//        }
//    }

    LFNSSampler::~LFNSSampler() = default;

    const std::vector<double> &LFNSSampler::samplePrior() {
        std::vector<double> &sa = _prior->sample();
        std::vector<double> tmp_vec_a(1);
        tmp_vec_a[0] = sa[0];
        std::vector<double> &sb = _scaleSample(sa);
        std::vector<double> tmp_vec_b = sb;
        return sb;
    }


    const std::vector<double> &LFNSSampler::sampleConstrPrior() {
        std::vector<double> &sample = _density_estimation->sample();
        if (!_uniform_prior) {
            double prior_val = _prior->getLogLikelihood(sample);
            double u = _dist(*_rng) * _max_live_prior_value;
            while (u > prior_val) {
                sample = _density_estimation->sample();
                prior_val = _prior->getLogLikelihood(sample);
            }
        }
        return _scaleSample(sample);
    }

    void LFNSSampler::updateLiveSamples(lfns::LiveParticleSet &live_points) {

        std::multiset<LFNSParticle>::iterator it;
        _max_live_prior_value = -DBL_MAX;
        if (!_uniform_prior) {
            for (it = live_points.begin(); it != live_points.end(); it++) {
                double prior_val = _prior->getLogLikelihood(it->getTheta());
                _max_live_prior_value = _max_live_prior_value > prior_val ? _max_live_prior_value : prior_val;
            }
        }

        base::EiMatrix live_points_matrix = live_points.toMatrix();
        for (int &index: _log_params) {
            live_points_matrix.col(index) = live_points_matrix.col(index).array().log10();
        }
        _density_estimation->updateDensitySamples(live_points_matrix);
    }


    void LFNSSampler::updateSerializedSampler(std::stringstream &stream) {
        boost::archive::binary_iarchive iar(stream);
        iar >> _density_estimation;
        _density_estimation->setRng(_rng);
    }

    void LFNSSampler::getSerializedSampler(std::stringstream &stream) {
        boost::archive::binary_oarchive oar(stream);
        oar << _density_estimation;
    }

    void LFNSSampler::writeToStream(std::ostream &stream) {
        stream << "Prior used: " << std::endl;
        _prior->writeToStream(stream);
        stream << "\nDensity estimation used: " << std::endl;
        _density_estimation->writeToStream(stream);
        stream << std::endl;
    }


    sampler::DensityEstimation_ptr LFNSSampler::getDensityEstimation() { return _density_estimation; }

    std::vector<double> &LFNSSampler::_scaleSample(std::vector<double> &sample) {
        if (_log_params.empty()) { return sample; }
        else {
            for (int &index : _log_params) { sample[index] = std::pow(10, sample[index]); }
            return sample;
        }
    }

    void LFNSSampler::setLogParams(std::vector<int> log_params) {
        for (int param_index : log_params) {
            if (std::find(_log_params.begin(), _log_params.end(), param_index) == _log_params.end()) {
                _log_params.push_back(param_index);
                _prior->setLogScale(param_index);
                _density_estimation->setLogScale(param_index);
            }
        }
    }
}
