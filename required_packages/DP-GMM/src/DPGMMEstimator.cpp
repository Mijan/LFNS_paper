/*
 * DPGMMEstimator.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: jan
 */


#include "DPGMMEstimator.h"

#include "HyperParameters.h"
#include "IndicatorParameters.h"

namespace DP_GMM {

    DPGMMEstimator::DPGMMEstimator(RngPtr r, int num_dp_iterations) :
            _r(r), _num_dp_iterations(num_dp_iterations) { }

    DPGMMEstimator::~DPGMMEstimator() { }

    void DPGMMEstimator::estimate(const EiMatrix &data,
                                  HyperParameters &hyper_parameters,
                                  EstimationMixtureComponentSet *mixture_components, bool verbose,
                                  DPGMMLogger *logger) {

        _estimate(data, hyper_parameters, mixture_components, verbose, logger);
    }

    HyperParameters DPGMMEstimator::estimate(const Data &data,
                                             EstimationMixtureComponentSet *mixture_components, bool verbose,
                                             DPGMMLogger *logger) {

        HyperParameters hyper_parameters(data, _r);
        _estimate(data, hyper_parameters, mixture_components, verbose, logger);
        return hyper_parameters;
    }

    void DPGMMEstimator::_estimate(const Data &data,
                                   HyperParameters &hyper_parameters,
                                   EstimationMixtureComponentSet *mixture_components, bool verbose,
                                   DPGMMLogger *logger) {

        int num_samples = (int) data.rows();

        IndicatorParameters indicator_parameters(data, mixture_components, _r);

        indicator_parameters.sampleIndicators(hyper_parameters);
        hyper_parameters.updateAlpha(num_samples,
                                     indicator_parameters.getNumberComponents());

        for (EstimationMixtureComponentSet::iterator it = mixture_components->begin();
             it != mixture_components->end(); it++) {
            EstimationMixtureComponentPtr comp = *it;
            comp->updateMixtureParameters(hyper_parameters.getXi(),
                                          hyper_parameters.getRho(), hyper_parameters.getW(),
                                          hyper_parameters.getBeta());
        }

        if (verbose) {
            std::cout << std::endl;
            std::cout << "DP-GMM estimation started. Number of Components : "
            << std::endl;
            std::cout << indicator_parameters.getNumberComponents() << ", "
            << std::flush;
        }
        for (int it = 0; it < _num_dp_iterations; it++) {

            hyper_parameters.updateRho(*mixture_components);
            hyper_parameters.updateW(*mixture_components);
            hyper_parameters.updateXi(*mixture_components);
            hyper_parameters.updateBeta(*mixture_components);

            indicator_parameters.sampleIndicators(hyper_parameters);
            if (verbose) {
                std::cout << indicator_parameters.getNumberComponents() << ", "
                << std::flush;
                if (it % 50 == 0 && it != 0) {
                    std::cout << std::endl;
                }
            }
            if (logger != NULL) {
                logger->writeDataToFile(&hyper_parameters, mixture_components, it);
            }
            hyper_parameters.updateAlpha(num_samples,
                                         indicator_parameters.getNumberComponents());
            for (EstimationMixtureComponentSet::iterator it = mixture_components->begin();
                 it != mixture_components->end(); it++) {
                EstimationMixtureComponentPtr comp = *it;
                comp->updateMixtureParameters(hyper_parameters.getXi(),
                                              hyper_parameters.getRho(), hyper_parameters.getW(),
                                              hyper_parameters.getBeta());
            }

        }

        std::cout << std::endl;
    }

} /* namespace DP_GMM */
