/*
 * AdaptiveRejectionSampler.cpp
 *
 *  Created on: Jul 30, 2013
 *      Author: jan
 */

#include <cmath>
#include <float.h>
#include <stdlib.h>
#include <iostream>

#include "AdaptiveRejectionSampler.h"

namespace DP_GMM {

    AdaptiveRejectionSampler::AdaptiveRejectionSampler(RngPtr r) :
            _dist_uniform(0, 1), _initial_nbr_samples(3), _points(), _rng(r) { }

    AdaptiveRejectionSampler::~AdaptiveRejectionSampler() { }

    double AdaptiveRejectionSampler::sample(LogConcaveFunction &function) {
        if (function.isPointMass()) {
            return function._inverseTransformInput(
                    0.5 * (function.getMaxRange() + function.getMinRange()));
        }

        initialize(function);
        while (true) {
            double x_star = drawSampleS(function.getMinRange(),
                                        function.getMaxRange());
            double u_star = _dist_uniform(*_rng);

            double u_b = evalUpperEnv(x_star);
            double f, diff_f;

            if (u_star <= exp(evalLowerEnv(x_star) - u_b)) {
                return function._inverseTransformInput(x_star);
            } else {
                f = function(x_star);
                double t_plus = x_star + EPS;
                double f_plus = function(t_plus);
                diff_f = (f_plus - f) / EPS;

                if (u_star <= exp(log(f) - u_b)) {
                    return function._inverseTransformInput(x_star);;
                }
            }

            _points.insertPoint(PointInformation(x_star, f, diff_f));
            _points.updatePoints(function.getMinRange(), function.getMaxRange(),
                                 function.getMinRangeVal(), function.getMaxRangeVal());
        }
        return 0.0;
    }

    void AdaptiveRejectionSampler::initialize(LogConcaveFunction &function) {
        double min_range = function.getMinRange();
        double max_range = function.getMaxRange();
        _points = SequenceOfPointInformations();
        for (int j = 1; j <= _initial_nbr_samples; j++) {
            double x = min_range
                       + j * (max_range - min_range) / (_initial_nbr_samples + 1);
            _points.insertPoint(PointInformation(x, function));
        }
        _points.updatePoints(function.getMinRange(), function.getMaxRange(),
                             function.getMinRangeVal(), function.getMaxRangeVal());
    }

// TODO return vector position where the new sample would go...
    double AdaptiveRejectionSampler::drawSampleS(double min, double max) {
        double u = _dist_uniform(*_rng);
        double log_int_expU = IntegralU(min, max);
        int k = (int) _points.size();

        int j = 0;
        double zL = min;
        double zM = _points.getZ(0, 1);

        double suTotal = 0.0;
        double su = 0.0;

        su = integrateExpUOnInterval(zL, zM, _points.getPoint(0)) / log_int_expU;

        while (suTotal + su < u) {

            suTotal += su;
            if (j >= k) {
                std::cout << "Sampling of Log Concave function went wrong!" << std::endl;
                break;
            }
            j++;
            zL = _points.getZ(j - 1, j);
            if (j == k - 1) {
                zM = max;
            } else {
                zM = _points.getZ(j, j + 1);;
            }

            su = integrateExpUOnInterval(zL, zM, _points.getPoint(j))
                 / log_int_expU;
        }

        u -= suTotal;

        PointInformation point = _points.getPoint(j);
        double x_star = (log(
                u * point.dlogf()
                + exp(
                        point.logf() - point.x() * point.dlogf()
                        + point.dlogf() * zL)) - point.logf()
                         + point.x() * point.dlogf()) / point.dlogf();
        if (x_star > max || x_star < min || x_star != x_star) {
            std::ostringstream os;
            os << "log concave sampling went wrong! x_star is " << x_star
            << " but min and max are " << min << " and " << max
            << std::endl;
            throw std::runtime_error(os.str());
        }
        return x_star;
    }

    double AdaptiveRejectionSampler::IntegralU(double min, double max) {
        double integral = 0;
        int k = _points.size();

        double zL, zU;

        for (int i = 0; i < k; i++) {
            if (i == 0) {
                zL = min;
            } else {
                zL = _points.getZ(i - 1, i);
            }

            if (i == k - 1) {
                zU = max;
            } else {
                zU = _points.getZ(i, i + 1);
            }

            double integral_piece = integrateExpUOnInterval(zL, zU,
                                                            _points.getPoint(i));

            integral += integral_piece;
        }
        return integral;
    }

    double AdaptiveRejectionSampler::integrateExpUOnInterval(double l_bound,
                                                             double r_bound, PointInformation &point) {
        /* zL is the intersection of the tangents at xL and x{L-1}, hL is the
         * log-function value at xL and diff_hL is the derivative of the log_function value at xL */
        double integral_piece = (exp(
                r_bound * point.dlogf() - point.x() * point.dlogf() + point.logf())
                                 - exp(
                l_bound * point.dlogf() - point.x() * point.dlogf()
                + point.logf())) / point.dlogf();

        if (integral_piece <= 0 || std::isinf(integral_piece)
            || integral_piece != integral_piece) {
            std::ostringstream os;
            os << "Integral for ARS was " << integral_piece << " on interval "
            << l_bound << " to " << r_bound
            << ". The function value at center point " << point.x()
            << " is " << point.logf() << " and its derivative is "
            << point.dlogf() << std::endl;
            throw std::runtime_error(os.str());
        }
        return integral_piece;
    }

    double AdaptiveRejectionSampler::evalUpperEnv(double x) {
        std::size_t index = _points.getUpperEnvInterval(x);
        PointInformation &point = _points.getPoint(index);
        return point.logf() + (x - point.x()) * point.dlogf();
    }

    double AdaptiveRejectionSampler::evalLowerEnv(double x) {

        if (x <= _points.getPoint(0).x()
            || x >= _points.getPoint(_points.size() - 1).x()) {
            return -DBL_MAX;
        } else {
            size_t index = _points.getLowerEnvInterval(x);
            PointInformation &l_point = _points.getPoint(index);
            PointInformation &r_point = _points.getPoint(index + 1);
            double bound = ((r_point.x() - x) * l_point.logf()
                            + (x - l_point.x()) * r_point.logf())
                           / (r_point.x() - l_point.x());
            return bound;
        }
    }
} /* namespace DP_GMM2 */
