/*
 ============================================================================
 Name        : 	estimate_density.cpp
 Author      : 	Jan Mikelson
 Version     :
 Copyright   : 	This free software is available under the Creative Commons Attribution Share Alike License.
 You are permitted to use, redistribute and adapt this software as long as appropriate credit
 is given to the original author, and all derivative works are distributed under the same
 license or a compatible one.
 For more information, visit http://creativecommons.org/licenses/by-sa/3.0/ or send a letter to
 Creative Commons, 171 2nd Street, Suite 300, San Francisco, California, 94105, USA.
 Description : 	Provides a DP-GMM density estimation of the given data
 ============================================================================
 */

#include <stdio.h>
#include <fstream>
#include <iostream>

#include "EstimationOptions.h"
#include "MatrixTypes.h"
#include "HyperParameters.h"
#include "EstimationMixtureComponent.h"
#include "DPGMMEstimator.h"
#include "DataReader.h"
#include "IOUtils.h"

int main(int argc, char *argv[]) {

    DP_GMM::EstimationOptions options;
    options.handleOptions(argc, argv);

    DP_GMM::EiMatrix data = DP_GMM::DataReader::readData(
            options.data_file_name);
    std::size_t num_data_points = data.rows();
    std::size_t num_dimensions = data.cols();

    options.printInfo(num_data_points, num_dimensions);

    int seed = time(NULL);
#ifdef DEBUG
    seed = 5;
#endif
    DP_GMM::RngPtr rng = std::make_shared<DP_GMM::RandomNumberGenerator>(
            seed);

    DP_GMM::DPGMMEstimator estimator(rng, options.num_dp_iterations);

    DP_GMM::DPGMMLogger logger(options.output_file_name,
                               options.print_frequency);
    DP_GMM::EstimationMixtureComponentSet mixtures;
    if (options.read_hyper_params) {
        DP_GMM::HyperParameters hyper_parameters(data, rng, options.hyper_params_file_name);
        estimator.estimate(data, hyper_parameters, &mixtures, true, &logger);
        logger.writeDataToFile(&hyper_parameters, &mixtures);
    } else {
        DP_GMM::HyperParameters hyper_parameters(data, rng);
        estimator.estimate(data, hyper_parameters, &mixtures, true, &logger);
        logger.writeDataToFile(&hyper_parameters, &mixtures);
    }

    return 1;
}

