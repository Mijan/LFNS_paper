//
// Created by jan on 08/10/18.
//

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cmath>
#include <boost/math/special_functions.hpp>
#include "MathUtils.h"

namespace base {

    int MathUtils::findIndexSmallerThan(std::vector<double> &vec, double entry) {
        int index = binarySearchFormer(vec.begin(), vec.end() - 1, entry) - vec.begin();
        return index;
    }

    int MathUtils::findIndexLargerThan(std::vector<double> &vec, double entry) {
        int index = binarySearchLatter(vec.begin(), vec.end() - 1, entry) - vec.begin();
        return index;
    }

    std::vector<double>::iterator MathUtils::binarySearchLatter(std::vector<double>::iterator begin,
                                                                std::vector<double>::iterator end, double entry) {

        double end_val = *end;
        double begin_val = *begin;
        std::size_t size = end - begin + 1;
        if (entry < begin_val) { return begin; }

        if (entry >= end_val) {
            std::ostringstream os;
            os << "Entry not in range for binary search! Entry: " << entry << ", first element: " << begin_val <<
               ", last element: " << end_val << std::endl;
            throw std::runtime_error(os.str());
        }

        if (size == 2) { return end; }
        std::vector<double>::iterator middle = begin + (int) std::floor((double) size / 2.0);
        double middle_val = *middle;

        if (entry == middle_val) { return middle; }
        if (entry < middle_val) { return binarySearchLatter(begin, middle, entry); }
        else { return binarySearchLatter(middle, end, entry); }
    }

    std::vector<double>::iterator MathUtils::binarySearchFormer(std::vector<double>::iterator begin,
                                                                std::vector<double>::iterator end, double entry) {

        double end_val = *end;
        double begin_val = *begin;
        std::size_t size = end - begin + 1;
        if (entry > end_val) { return end; }

        if (entry < begin_val) {
            std::ostringstream os;
            os << "Entry not in range for binary search! Entry: " << entry << ", first element: " << begin_val <<
               ", last element: " << end_val << std::endl;
            throw std::runtime_error(os.str());
        }

        if (size == 2) { return begin; }
        std::vector<double>::iterator middle = begin + (int) std::floor((double) size / 2.0);
        double middle_val = *middle;

        if (entry == middle_val) { return middle; }
        if (entry < middle_val) { return binarySearchFormer(begin, middle, entry); }
        else { return binarySearchFormer(middle, end, entry); }
    }

    double MathUtils::binomial(double n, double k) {
        if (n < k) { return 0; }
        return boost::math::binomial_coefficient<double>(n, k);
    }


    EiMatrix MathUtils::computeCovariance(EiMatrix &data) {
        EiVector mu_data = data.colwise().sum() * (1.0 / (double) data.rows());

        EiMatrix sigma_data(data.cols(), data.cols());
        sigma_data.setZero();

        for (int i = 0; i < data.rows(); i++) {
            EiVector a = (data.row(i).transpose() - mu_data);
            EiMatrix test = a * a.transpose();
            sigma_data = sigma_data
                         + ((data.row(i).transpose() - mu_data)
                            * ((data.row(i).transpose() - mu_data).transpose()));
        }


        sigma_data *= 1.0 / ((double) data.rows());
        return sigma_data;
    }

    double MathUtils::diGamma(int x) { return boost::math::digamma<int>(x); }

    double MathUtils::sumOfLog(double log_1, double log_2) {
        double max_log = std::max<double>(log_1, log_2);
        return std::log(std::exp(log_1 - max_log) + std::exp(log_2 - max_log)) + max_log;
    }


    double MathUtils::diffOfLog(double log_1, double log_2) {

        double max_log = std::max<double>(log_1, log_2);
        return std::log(std::exp(log_1 - max_log) - std::exp(log_2 - max_log)) + max_log;
    }

    double MathUtils::sumOfLog(const std::vector<double> &logs) {
        double max_log = *std::max_element(logs.begin(), logs.end());
        return sumOfLog(logs, max_log);
    }


    double MathUtils::sumOfLog(const std::vector<double> &logs, double max_log) {
        double sum = 0;
        std::vector<double>::const_iterator it = logs.begin();
        for (it; it != logs.end(); ++it) {
            sum += std::exp(*it - max_log);
        }
        return std::log(sum) + max_log;
    }
}



