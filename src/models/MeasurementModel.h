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
                                 double t)> LikelihoodFct;
    typedef std::shared_ptr<LikelihoodFct> LikelihoodFct_ptr;

    typedef std::function<void(std::vector<double> *measurement, const std::vector<double> &state, double t)> MeasurementFct;
    typedef std::shared_ptr<MeasurementFct> MeasurementFct_ptr;

    class MeasurementModel : public ParserBaseObject {
    public:
        MeasurementModel(base::RngPtr rng, MeasurementModelData measurement_model_data);

        virtual ~MeasurementModel();

        virtual void computeMeasurement(std::vector<double> *measurement, const std::vector<double> &state, double t);

        virtual double
        getLogLikelihood(const std::vector<double> &state, const std::vector<double> &measurements, double t);

        LikelihoodFct_ptr getLikelihoodFct();

        void printInfo(std::ostream &os);

        virtual void printPointer(std::ostream &os) const;

        virtual void printPointerValue(std::ostream &os) const;

        const std::vector<std::string> &getMeasurementNames() const;

        std::size_t getNumMeasurements() const;

    private:
        bool _initialized;

        std::vector<mu::Parser> _measurement_parsers;
        std::vector<mu::Parser> _log_likelihood_parsers;
        MeasurementModelData _measurement_model_data;

        std::vector<double *> _measurement_ptr;
        std::vector<double> _measurement;

        void _initialize();

        virtual bool _allPointerSet() const;

        void _updateMeasurement(const std::vector<double> &measurement);
    };

    typedef std::shared_ptr<MeasurementModel> MeasurementModel_ptr;
}


#endif //LNSV4_MEASUREMENTMODELREADIN_H
