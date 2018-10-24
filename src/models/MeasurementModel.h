//
// Created by jan on 05.04.16.
//

#ifndef LNSV4_MEASUREMENTMODELREADIN_H
#define LNSV4_MEASUREMENTMODELREADIN_H

#include <functional>
#include "ParserBaseObject.h"
#include "MeasurementModelData.h"

namespace models {
    typedef std::function<double(const std::vector<double> &state, const std::vector<double> &data,
                                 const std::vector<double> &theta)> LikelihoodFct;
    typedef std::shared_ptr<LikelihoodFct> LikelihoodFct_ptr;

    class MeasurementModel : public ParserBaseObject {
    public:
        MeasurementModel(base::RngPtr rng, MeasurementModelData measurement_model_data);

        virtual ~MeasurementModel();

        virtual void computeMeasurement(std::vector<double> *measurement, const std::vector<double> &state, double t,
                                        const std::vector<double> &theta);

        virtual double getLogLikelihood(const std::vector<double> &state, const std::vector<double> &measurements,
                                        const std::vector<double> &theta);

        LikelihoodFct_ptr getLikelihoodFct();

        void printInfo(std::ostream &os);

        virtual void printPointer(std::ostream &os) const;

        const std::vector<std::string> &getMeasurementNames() const;

        void setOutputMeasurementOrder(std::vector<std::string> measurement_order);

        void setInputMeasurementOrder(std::vector<std::string> measurement_order);


    private:
        bool _initialized;

        std::vector<mu::Parser> _measurement_parsers;
        std::vector<mu::Parser> _log_likelihood_parsers;
        MeasurementModelData _measurement_model_data;

        std::vector<double *> _measurement_ptr;
        std::vector<double> _measurement;
        std::map<int, int> _output_measurement_ext_by_int;
        std::map<int, int> _input_measurement_ext_by_int;

        void _initialize();

        virtual bool _allPointerSet() const;

        void _updateMeasurement(const std::vector<double> &measurement);

        bool _allInputsDefined() override;

        void _printInputs() override;
    };

    typedef std::shared_ptr<MeasurementModel> MeasurementModel_ptr;
}


#endif //LNSV4_MEASUREMENTMODELREADIN_H
