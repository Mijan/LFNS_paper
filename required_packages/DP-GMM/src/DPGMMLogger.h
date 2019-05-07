/*
 * DPGMMLogger.h
 *
 *  Created on: Mar 6, 2015
 *      Author: jan
 */

#ifndef DPGMM_LOGGER_H_
#define DPGMM_LOGGER_H_


#include "HyperParameters.h"
#include "DPMixtureComponent.h"


namespace DP_GMM {
    class DPGMMLogger {
    public:
        DPGMMLogger(std::string output_file_name, int print_frequency);

        virtual ~DPGMMLogger();

        void writeData(HyperParameters *hyper_parameters,
                       MixtureComponentSet *mixture_components, int iteration_nbr);

        void writeData(HyperParameters *hyper_parameters,
                       MixtureComponentSet *mixture_components);

        void writeData(MixtureComponentSet *mixture_components);

        static void writeData(MixtureComponentSet *mixture_components, std::string output_file_name);

        static void writeData(GaussMixtureComponentSet &mixture_components, std::string output_file_name);

        static void writeHyperParams(HyperParameters *hyper_parameters, std::string output_file_name);

        void writeData(std::vector<EiVector> means, std::vector<EiMatrix> covs);

    private:
        std::string _output_file_name;
        int _print_frequncy;

    };

} /* namespace DP_GMM2 */

#endif /* DPGMM_LOGGER_H_ */
