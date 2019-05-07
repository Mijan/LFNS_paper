/*
 * IOUtils.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: jan
 */

#include "IOUtils.h"

#include <iostream>

namespace DP_GMM {

    IOUtils::IOUtils() { }

    IOUtils::~IOUtils() { }

    std::string IOUtils::appendToFileName(const std::string fileName,
                                          const std::string string) {
        size_t pos = fileName.find_last_of(".");
        std::string fileStart;
        std::string fileEnd;
        std::string file;
        fileStart.assign(fileName.begin(), fileName.begin() + pos);
        fileEnd.assign(fileName.begin() + pos, fileName.end());
        file = fileStart + "_" + string + fileEnd;
        return file;
    }
} /* namespace DP_GMM */
