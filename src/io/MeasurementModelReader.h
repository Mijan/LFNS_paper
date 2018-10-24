//
// Created by jan on 12/10/18.
//

#ifndef LFNS_MEASUREMENTMODELREADER_H
#define LFNS_MEASUREMENTMODELREADER_H


#include "ParserReader.h"

namespace io {
    const std::string MEASUREMENT_CAPTION = "Measurement:";
    const std::string LOGLIKELIHOOD_CAPTION = "Loglikelihood:";
    class MeasurementModelReader : public ParserReader {
    public:
        MeasurementModelReader(std::string measurement_model_file_name);

        virtual ~MeasurementModelReader();

        std::vector<std::string> readMeasurements();

        std::vector<std::string> readOutputNames();

        std::string readLogLikelihood(std::string measurement);
    };
}


#endif //LFNS_MEASUREMENTMODELREADER_H
