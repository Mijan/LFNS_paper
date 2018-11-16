//
// Created by jan on 08/10/18.
//

#ifndef LFNS_MATHUTILS_H
#define LFNS_MATHUTILS_H


#include <vector>
#include "EigenMatrices.h"

namespace base {
    class MathUtils {
    public:
        static int findIndexSmallerThan(std::vector<double> &vec, double entry);

        static int findIndexLargerThan(std::vector<double> &vec, double entry);

        static std::vector<double>::iterator binarySearchLatter(std::vector<double>::iterator begin,
                                                                std::vector<double>::iterator end, double entry);

        static std::vector<double>::iterator binarySearchFormer(std::vector<double>::iterator begin,
                                                                std::vector<double>::iterator end,
                                                                double entry);

        static double binomial(double n, double k);

        template<typename T>
        static int sgn(T val) { return (T(0) < val) - (val < T(0)); }

        static EiMatrix computeCovariance(EiMatrix &data);

        static double diGamma(int x);

        static double sumOfLog(double log_1, double log_2);
        static double diffOfLog(double log_1, double log_2);


        static void sumOfLog(const std::vector<double> &logs, const std::vector<int> &signs, double * log_result, int* sign_result);

        static double sumOfLog(const std::vector<double> &log_s);
        static double sumOfLog(const std::vector<double> &logs, const std::vector<int> &signs);

        static double sumOfLog(const std::vector<double> &logs, double max_log);
        static double sumOfLog(const std::vector<double> &log_s, const std::vector<int> &signs,  double max_log);
        static void sumOfLog(const std::vector<double> &log_s, const std::vector<int> &signs,  double max_log, double *log_result, int* sign_result);

    };
}


#endif //LFNS_MATHUTILS_H
