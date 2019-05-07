/*
 * IOUtils.h
 *
 *  Created on: Mar 13, 2015
 *      Author: jan
 */

#ifndef DP_GMM2_FILEIO_IOUTILS_H_
#define DP_GMM2_FILEIO_IOUTILS_H_

#include <string>

namespace DP_GMM {

    class IOUtils {
    public:
        IOUtils();

        virtual ~IOUtils();

        static std::string appendToFileName(const std::string fileName, const std::string string);
    };

} /* namespace DP_GMM2 */

#endif /* FILEIO_IOUTILS_H_ */
