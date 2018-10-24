#include <iostream>
#include "src/LFNS/DeadParticleSet.h"
#include "src/LFNS/LiveParticleSet.h"
#include "src/LFNS/PosteriorEstimator.h"
#include "src/sampler/DpGmmSampler.h"
#include "src/sampler/EllipsoidSampler.h"
#include "src/sampler/KernelDensityEstimation.h"
#include "src/sampler/UniformSampler.h"
#include "src/sampler/GaussianSampler.h"
#include "src/sampler/KernelSupportEstimation.h"
#include "src/sampler/RejectionSupportSampler.h"
#include "src/LFNS/LFNSSettings.h"
#include "src/LFNS/LFNS.h"

int main() {
    std::string dead_points_file_name = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/H100_n1000_posterior.txt";

    lfns::DeadParticleSet dead_points;
    dead_points.readFromFile(dead_points_file_name);
    dead_points.writeToFile(dead_points_file_name, "new");


    std::string live_points_file_name = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/H100_n1000_57.txt";
    lfns::LiveParticleSet live_points;
    live_points.readFromFile(live_points_file_name);
    live_points.writeToFile(live_points_file_name, "new");

    lfns::PosteriorEstimator estimator(1000, 100);
    lfns::PosteriorQuantitites quantities = estimator.estimatePosteriorQuantities(live_points, dead_points);

    std::string posterior_file_name = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/birth_death_one_param/results/H100_n1000_2/new_posterior.txt";
    estimator.writeToFile(posterior_file_name, live_points, dead_points, quantities);

    std::cout << "dead points read from : " << dead_points_file_name << std::endl;
    std::cout << "live points read from : " << live_points_file_name << std::endl << std::endl;
    std::cout << "log(Ztot) : " << "\t\t" << quantities.log_ztot << std::endl;
    std::cout << "log(Var(Ztot)) : " << "\t" << quantities.log_ztot_var << std::endl;
    std::cout << "log(ZD) : " << "\t\t\t" << quantities.log_zd << std::endl;
    std::cout << "log(Var(ZD)) : " << "\t\t" << quantities.log_zd_var << std::endl;
    std::cout << "log(ZL) : " << "\t\t\t" << quantities.log_zl << std::endl;
    std::cout << "log(Var(ZL)) : " << "\t\t" << quantities.log_zl_var << std::endl;

    return 1;
}