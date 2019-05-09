/*
 * DPGMMLogger.h
 *
 *  Created on: Mar 6, 2015
 *      Author: jan
 */

#ifndef DPGMM_LOGGER_H_
#define DPGMM_LOGGER_H_


#include "HyperParameters.h"
#include "EstimationMixtureComponent.h"


namespace DP_GMM {
    class DPGMMLogger {
    public:
        DPGMMLogger(std::string output_file_name, int print_frequency);

        virtual ~DPGMMLogger();

        void writeDataToFile(HyperParameters *hyper_parameters,
                             EstimationMixtureComponentSet *mixture_components, int iteration_nbr);

        void writeDataToFile(HyperParameters *hyper_parameters,
                             EstimationMixtureComponentSet *mixture_components);

        void writeDataToFile(EstimationMixtureComponentSet *mixture_components);

        static void writeDataToFile(EstimationMixtureComponentSet *mixture_components, std::string output_file_name);

        static void writeDataToStream(EstimationMixtureComponentSet *mixture_components, std::ostream &stream);

        static void writeDataToFile(GaussMixtureComponentSet &mixture_components, std::string output_file_name);

        static void writeDataToStream(GaussMixtureComponentSet &mixture_components, std::ostream &stream);

        static void writeHyperParams(HyperParameters *hyper_parameters, std::string output_file_name);

        void writeData(std::vector<EiVector> means, std::vector<EiMatrix> covs);

    private:
        std::string _output_file_name;
        int _print_frequncy;

    };

} /* namespace DP_GMM2 */

#endif /* DPGMM_LOGGER_H_ */
