/*
 * DataReader.h
 *
 *  Created on: Mar 13, 2015
 *      Author: jan
 */

#ifndef DATAREADER_H_
#define DATAREADER_H_

#include "EstimationOptions.h"
#include "MatrixTypes.h"
#include "HyperParameters.h"

namespace DP_GMM {

    class DataReader {
    public:
        DataReader();

        virtual ~DataReader();

        static EiMatrix readData(std::string data_file_name);
    };

} /* namespace DP_GMM */

#endif /* DATAREADER_H_ */
