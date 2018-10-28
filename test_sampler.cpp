#include <iostream>
#include "src/sampler/DpGmmSampler.h"
#include "src/sampler/EllipsoidSampler.h"
#include "src/sampler/KernelDensityEstimation.h"
#include "src/sampler/UniformSampler.h"
#include "src/sampler/GaussianSampler.h"
#include "src/sampler/KernelSupportEstimation.h"
#include "src/sampler/RejectionSupportSampler.h"
#include "src/LFNS/LiveParticleSet.h"


int main() {
//    std::string samples_file_name = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/birth_death_5.txt";
//    lfns::LiveParticleSet live_set;
//    live_set.readFromFile(samples_file_name);
//    base::EiMatrix samples = live_set.toMatrix();
//
//    time_t seed = time(NULL);
//    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(seed);
//
//    // test dp-gmm estimator
//    sampler::DpGmmSamplerData dp_data(2);
//    dp_data.num_dp_iterations = 50;
//    sampler::DpGmmSampler dp_sampler(rng, dp_data);
//    dp_sampler.updateDensitySamples(samples);
//
//
//    // test ellipsoid sampler
//    sampler::SamplerData ell_data(2);
//    sampler::EllipsoidSampler ell_sampler(rng, ell_data);
//    ell_sampler.updateDensitySamples(samples);
////
////    sampler::UniformSamplerData unif_data(2);
////    sampler::KernelSampler_ptr unif_kernel = std::make_shared<sampler::UniformSampler>(rng, unif_data);
////
////    sampler::NormalSamplerData normal_data(2);
////    normal_data.cov << 0.1, 0, 0, 0.1;
////    sampler::GaussianSampler_ptr gauss_kernel = std::make_shared<sampler::GaussianSampler>(rng, normal_data);
//
////    sampler::KernelSupportEstimation kernel_sampler(rng, gauss_kernel, ell_data);
////    sampler::KernelDensityEstimation kernel_sampler(rng, gauss_kernel, ell_data);
////    kernel_sampler.updateDensitySamples(samples);
//
//    sampler::RejectionSamplerData rejection_data(2);
//    rejection_data.rejection_quantile = 0.001;
//    rejection_data.rejection_quantile_low_accept = 0.1;
//    rejection_data.log_rejection_const = -1;
//    rejection_data.thresh_accept_rate = 0.001;
//    rejection_data.max_rejection_mag = -1;
//
//    sampler::DensityEstimation_ptr dp_estimation = std::make_shared<sampler::DpGmmSampler>(dp_sampler);
//    sampler::RejectionSupportSampler rejection_sampler(rng, dp_estimation, rejection_data);
//    rejection_sampler.updateDensitySamples(samples);
//
//
//    lfns::LiveParticleSet new_samples;
//    for (int i = 0; i < 1000; i++) {
//        new_samples.push_back(dp_sampler.sample(), 0);
//    }
//
//    std::string output_file_name = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/birth_death_output.txt";
//    new_samples.writeToFile(output_file_name);

    return 1;
}