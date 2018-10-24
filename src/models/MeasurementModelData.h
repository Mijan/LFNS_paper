//
// Created by jan on 05.01.17.
//

#ifndef LNSV5_MEASUREMENTMODELDATA_H
#define LNSV5_MEASUREMENTMODELDATA_H

#include "ParserBaseObject.h"

namespace models {
    class MeasurementModelData : public ParserData {
    public:

        MeasurementModelData(std::string measurement_model_file_name);

        virtual ~MeasurementModelData();

        std::size_t getNumMeasurements() const;

        void setMeasurementEquation(std::string measurement, std::string equation);

        void setLogLikelihoodEquation(std::string measurement, std::string equation);

        std::string getMeasurementEquationForMeasurment(std::string measurement_name) const;

        std::string getLogLikelihoodEquationForMeasurment(std::string measurement_name) const;

        bool isMeasurementName(std::string name) const;

        std::size_t getMeasurementIndex(std::string measurement_name) const;

        const std::vector<std::string> &getMeasurementNames() const;

    protected:
        std::vector<std::string> measurement_names;
        std::map<std::string, std::string> measurement_equations_by_name;
        std::map<std::string, std::string> log_likelihoods_by_name;

    };
}


#endif //LNSV5_MEASUREMENTMODELDATA_H
