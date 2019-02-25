//
// Created by jan on 25/02/19.
//

#include "MCMCSampler.h"
#include "../sampler/UniformSampler.h"
#include "../sampler/GaussianSampler.h"

namespace mcmc {

    MCMCSampler::MCMCSampler(MCMCSettings &mcmc_settings, sampler::SamplerSettings &settings, base::RngPtr rng) : _rng(
            rng), _prior(), _uniform_prior(true), _log_params() {

        std::vector<std::string> unfixed_params = settings.param_names;
        sampler::SamplerData sampler_data(unfixed_params.size());
        sampler_data.bounds = settings.getBounds(unfixed_params);
        _log_params = settings.getLogParams(unfixed_params);

        for (int i : _log_params) {
            sampler_data.bounds[i].first = std::log10(sampler_data.bounds[i].first);
            sampler_data.bounds[i].second = std::log10(sampler_data.bounds[i].second);
        }

        if (mcmc_settings.uniform_prior) {
            sampler::UniformSamplerData uni_data(sampler_data);
            _prior = std::make_shared<sampler::UniformSampler>(_rng, uni_data);
        }

        switch (mcmc_settings.kernel_type) {
            case GAUSS  : {
                sampler::NormalSamplerData normal_data(sampler_data);
                normal_data.cov = base::EiMatrix::Identity(sampler_data.size(), sampler_data.size()) * 0.5;
                sampler::GaussianSampler_ptr gauss_kernel = std::make_shared<sampler::GaussianSampler>(_rng,
                                                                                                       normal_data);

                _kernel_sampler = gauss_kernel;
                break;
            }
        }
    }


    const std::vector<double> &MCMCSampler::samplePrior() { return _scaleUpSample(_prior->sample()); }

    const std::vector<double> &MCMCSampler::sampleKernel(const std::vector<double> &kernel_center) {
        std::vector<double> scale_center = kernel_center;
        scale_center = _scaleDownSample(scale_center);
        std::vector<double> &sample = _kernel_sampler->sample(scale_center);
        while (!_prior->isSampleFeasible(sample)) {
            sample = _kernel_sampler->sample(scale_center);
        }
        return _scaleUpSample(sample);
    }


    std::vector<double> &MCMCSampler::_scaleUpSample(std::vector<double> &sample) {
        if (_log_params.empty()) { return sample; }
        else {
            for (int &index : _log_params) {
                sample[index] = std::pow(10, sample[index]);
            }
            return sample;
        }
    }

    std::vector<double> &MCMCSampler::_scaleDownSample(std::vector<double> &sample) {
        if (_log_params.empty()) { return sample; }
        else {
            for (int &index : _log_params) {
                sample[index] = std::log10(sample[index]);
            }
            return sample;
        }
    }

}