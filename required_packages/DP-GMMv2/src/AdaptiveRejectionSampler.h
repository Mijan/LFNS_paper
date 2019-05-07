/*
 * AdaptiveRejectionSampler.h
 *
 *  Created on: Jul 30, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_ADAPTIVEREJECTIONSAMPLER_H_
#define DP_GMM2_ADAPTIVEREJECTIONSAMPLER_H_

#include <algorithm>
#include <vector>

#include "LogConcaveFunction.h"
#include "BoostDistributions.h"

namespace DP_GMM {

    static double EPS = 0.00000001;

    class PointInformation {
    public:
        PointInformation(double x, LogConcaveFunction &fct) :
                _x(x), _logf_x(0), _dlogf_x(0) {
            double t_minus = x - EPS;
            double t_plus = x + EPS;
            double f_minus = fct(t_minus);
            double f_plus = fct(t_plus);
            double df_x = (f_plus - f_minus) / (2 * EPS);
            double f_x = fct(x);
            _logf_x = log(f_x);
            _dlogf_x = df_x / f_x;
        }

        PointInformation(double x, double f_x, double df_x) : _x(x), _logf_x(log(f_x)), _dlogf_x(df_x / f_x) { }

        ~PointInformation() { }

        double x() { return _x; }

        double logf() { return _logf_x; }

        double dlogf() { return _dlogf_x; }

        void setDLogf(double dlog_f_x) { _dlogf_x = dlog_f_x; }

        bool operator>(const PointInformation &rhs) const { return _x > rhs._x; }

        bool operator<(const PointInformation &rhs) const { return _x < rhs._x; }

        bool operator>=(const PointInformation &rhs) const { return _x >= rhs._x; }

        bool operator<=(const PointInformation &rhs) const { return _x <= rhs._x; }

    private:
        double _x;
        double _logf_x;
        double _dlogf_x;
    };

    class SequenceOfPointInformations {
    public:
        SequenceOfPointInformations() { }

        ~SequenceOfPointInformations() { }

        void insertPoint(PointInformation point) { _points.push_back(point); }

        void updatePoints(double min, double max, double f_min, double f_max) {
            std::sort(_points.begin(), _points.begin() + _points.size());
            _updateDerivatives(f_min, f_max, min, max);
            _updateZ();
        }

        PointInformation &getPoint(std::size_t i) { return _points[i]; }

        double getZ(std::size_t l_index, std::size_t r_index) { return _z[l_index]; }

        std::size_t size() { return _points.size(); }

        std::size_t getUpperEnvInterval(double x) {
            std::size_t index = 0;
            while (x > _z[index] && index < _z.size()) {
                index++;
            }
            return index;
        }

        std::size_t getLowerEnvInterval(double x) {
            std::size_t index = 0;
            while (x < _points[index + 1].x() && index < _points.size() - 1) {
                index++;
            }
            return index;
        }

    private:
        std::vector<PointInformation> _points;
        std::vector<double> _z;

        void _updateZ() {
            _z.clear();
            for (std::size_t i = 0; i < _points.size() - 1; i++) {
                double z = (_points[i + 1].logf() - _points[i].logf()
                            - _points[i + 1].x() * _points[i + 1].dlogf()
                            + _points[i].x() * _points[i].dlogf())
                           / (_points[i].dlogf() - _points[i + 1].dlogf());
                _z.push_back(z);
            }
        }

        void _updateDerivatives(double f_min, double f_max, double min,
                                double max) {
            double log_f_min = log(f_min);
            double log_f_max = log(f_max);
            double dlogf_l = (_points[0].logf() - log_f_min)
                             / (_points[0].x() - min);
            for (std::size_t i = 0; i < _points.size() - 1; i++) {
                double dlogf_r = (_points[i + 1].logf() - _points[i].logf())
                                 / (_points[i + 1].x() - _points[i].x());
                if (_points[i].dlogf() < dlogf_r || _points[i].dlogf() > dlogf_l) {
                    _points[i].setDLogf((dlogf_l + dlogf_r) / 2.0);
                }
            }
            double dlogf_r = (log_f_max - _points.back().logf())
                             / (max - _points.back().x());
            if (_points.back().dlogf() < dlogf_r
                || _points.back().dlogf() > dlogf_l) {
                _points.back().setDLogf((dlogf_l + dlogf_r) / 2.0);
            }
        }
    };

    class AdaptiveRejectionSampler {
    public:
        AdaptiveRejectionSampler(RngPtr r);

        virtual ~AdaptiveRejectionSampler();

        double sample(LogConcaveFunction &function);

    private:
        UniformRealDistribution _dist_uniform;
        int _initial_nbr_samples;
        SequenceOfPointInformations _points;
        RngPtr _rng;

        void initialize(LogConcaveFunction &function);

        double drawSampleS(double min, double max);

        double IntegralU(double min, double max);

        double integrateExpUOnInterval(double l_bound, double r_bound,
                                       PointInformation &point);

        double evalUpperEnv(double x);

        double evalLowerEnv(double x);
    };

} /* namespace DP_GMM2 */

#endif /* DP_GMM2_ADAPTIVEREJECTIONSAMPLER_H_ */
