//
// Created by jan on 25/02/19.
//

#ifndef MCMCSAMPLER_H
#define MCMCSAMPLER_H

#include "../sampler/Sampler.h"
#include "../sampler/SamplerSettings.h"
#include "MCMCSettings.h"

namespace mcmc {
    class MCMCSampler {
    public:
        MCMCSampler(MCMCSettings &mcmc_settings, sampler::SamplerSettings &settings, base::RngPtr rng);


        const std::vector<double> &samplePrior();
        const std::vector<double> &sampleKernel(const std::vector<double> &kernel_center);

    private:
        base::RngPtr _rng;
        sampler::Sampler_ptr _prior;
        sampler::KernelSampler_ptr _kernel_sampler;
        bool _uniform_prior;
        std::vector<int> _log_params;



        std::vector<double> &_scaleUpSample(std::vector<double> &sample);
        std::vector<double> &_scaleDownSample(std::vector<double> &sample);

    };
}


#endif //MCMCSAMPLER_H
