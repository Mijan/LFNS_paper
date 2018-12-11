//
// Created by jan on 27/10/18.
//

#ifndef LFNS_FULLMODEL_H
#define LFNS_FULLMODEL_H

#include "ChemicalReactionNetwork.h"
#include "InitialValueProvider.h"
#include "MeasurementModel.h"
#include "ModelSettings.h"

namespace models {
    typedef std::function<void(const std::vector<double> &theta)> SetParameterFct;
    typedef std::shared_ptr<SetParameterFct> SetParameterFct_ptr;


    typedef std::function<double(const double *state, double t)> RootFct;
    typedef std::shared_ptr<RootFct> RootFct_ptr;

    class FullModel {
    public:
        FullModel(ChemicalReactionNetwork_ptr dynamics, InitialValueProvider_ptr init_val,
                  MeasurementModel_ptr measurement);

        FullModel(models::ModelSettings &settings, base::RngPtr rng, std::string experiment = "");

        FullModel(const FullModel &rhs);

        virtual ~FullModel();

        SetParameterFct_ptr getParameterSettingFct();

        std::vector<std::string> getUnfixedParamteters();

        void fixParameter(std::string param_name, double val);

        void printInfo(std::ostream &os);

        void addInputPulse(InputPulse pulse);

        void evaluateInput(const double *state, double t);

        PerturbationFct_ptr getInputFunction();

        double root(const double *state, double t);

        RootFct_ptr getRootFct();

        MeasurementFct_ptr getMeasurementFct();

        std::vector<double> getDiscontTimes();

        void setParameter(const std::vector<double> &theta);

        ChemicalReactionNetwork_ptr dynamics;
        InitialValueProvider_ptr initial_value_provider;
        MeasurementModel_ptr measurement_model;

    private:

        std::vector<std::string> _param_names;
        std::vector<double> _parameter;
        std::vector<int> _unfixed_parameter_indices;
        InputPulses _inputs;
    };

    typedef std::shared_ptr<FullModel> FullModel_ptr;
}


#endif //LFNS_FULLMODEL_H
