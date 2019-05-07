/*
 * LogConcaveFunction.h
 *
 *  Created on: Jul 30, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_LOGCONCAVEFUNCTION_H_
#define DP_GMM2_LOGCONCAVEFUNCTION_H_

#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace DP_GMM {

class LogConcaveFunction {
public:
	LogConcaveFunction() :
			_scale(0), _min_log_range(0), _max_log_range(0), _min_bound_val(
					MIN_BOUND_VAL), _max_bound_val(MAX_BOUND_VAL), _max_computed_log_val(), _max_computed_log_pt() {
	}
	virtual ~LogConcaveFunction() {
	}

	virtual double operator()(double x) = 0;
	double getMinRange() {
		return LEFT_TRANFORMED_BOUND;
	}
	double getMaxRange() {
		return RIGHT_TRANFORMED_BOUND;
	}
	double getMinRangeVal() {
		return _logEvaluate(_min_log_range);
	}
	double getMaxRangeVal() {
		return _logEvaluate(_max_log_range);
	}
	double _transformInput(double input) {

		return LEFT_TRANFORMED_BOUND
				+ ((input - _min_log_range) / (_max_log_range - _min_log_range))
						* (RIGHT_TRANFORMED_BOUND - LEFT_TRANFORMED_BOUND);
	}

	double _inverseTransformInput(double input) {

		return _min_log_range
				+ ((input - LEFT_TRANFORMED_BOUND)
						/ (RIGHT_TRANFORMED_BOUND - LEFT_TRANFORMED_BOUND))
						* (_max_log_range - _min_log_range);
	}
	bool isPointMass() {
		return (exp(_max_log_range) - exp(_min_log_range))
				/ (0.5 * (exp(_max_log_range) + exp(_min_log_range))) <= TOL;
	}
protected:

	virtual double _logEvaluate(double x) const = 0;
	void _setRange(double feasible_pt, double min_range_tmp,
			double max_range_tmp) {
		_max_computed_log_pt = feasible_pt;
		_min_log_range = min_range_tmp;
		_max_log_range = max_range_tmp;
		do {
			_scale = 0.0;
			_setBounds(_max_computed_log_pt, _min_log_range, _max_log_range);
			_setScale();
		} while (!_range_properly_set());
	}
	bool _range_properly_set() {
		return _min_bound_val >= _max_computed_log_val + MIN_BOUND_VAL;
	}

	void _setBounds(double feasible_pt, double min_range_tmp,
			double max_range_tmp) {
		double feasible_val = _logEvaluate(feasible_pt);
		if (feasible_val <= -DBL_MAX || feasible_val >= DBL_MAX
				|| feasible_val != feasible_val) {
			double min_val = _logEvaluate(min_range_tmp);
			double max_val = _logEvaluate(max_range_tmp);
			std::ostringstream os;
			os
					<< "Could not sample from log concave function, since at feasible point  "
					<< feasible_pt << " it is  " << feasible_val << std::endl;
			throw std::runtime_error(os.str());
		}
		_max_computed_log_val = feasible_val;
		_max_computed_log_pt = feasible_pt;

		double min_val = _logEvaluate(min_range_tmp);
		_updateMaxPt(min_val, min_range_tmp);
		double max_val = _logEvaluate(max_range_tmp);
		_updateMaxPt(max_val, max_range_tmp);

		_min_bound_val = feasible_val + MIN_BOUND_VAL;
		_max_bound_val = feasible_val + MAX_BOUND_VAL;

		if (min_val < _min_bound_val) {
			_min_log_range = _findSmallesFeasiblePoint(min_range_tmp,
					feasible_pt);
		} else {
			_min_log_range = min_range_tmp;
		}

		if (max_val < _min_bound_val) {
			_max_log_range = _findSmallesFeasiblePoint(max_range_tmp,
					feasible_pt);
		} else {
			_max_log_range = max_range_tmp;
		}

	}

	double _findSmallesFeasiblePoint(double infeasible_pt, double feasible_pt) {

		double m_pt = (infeasible_pt + feasible_pt) / 2.0;
		double m_val = _logEvaluate(m_pt);
		_updateMaxPt(m_val, m_pt);

		if (m_val > _min_bound_val) {
			if (m_val <= _max_bound_val) {
				return m_pt;
			} else {
				return _findSmallesFeasiblePoint(infeasible_pt, m_pt);
			}
		} else {
			return _findSmallesFeasiblePoint(m_pt, feasible_pt);
		}
	}

	void _setScale() {
		_scale = 0.0;
		double scale = -DBL_MAX;
		double new_scale = 0.0;
		double x = _min_log_range;
		while (x <= _max_log_range) {
			new_scale = _logEvaluate(x);
			if (new_scale > scale) {
				scale = new_scale;
				_updateMaxPt(scale, x);
			}
			x += (_max_log_range - _min_log_range) / 100;
		}
		_scale = scale;
	}

	double _scale;
private:
	void _updateMaxPt(double new_val, double new_pt) {
		if (new_val > _max_computed_log_val) {
			_max_computed_log_val = new_val;
			_max_computed_log_pt = new_pt;
		}
	}

	double _min_log_range;
	double _max_log_range;
	double _min_bound_val;
	double _max_bound_val;

	double _max_computed_log_val;
	double _max_computed_log_pt;
	static constexpr double TOL = 0.1;

	static constexpr double MIN_BOUND_VAL = -7;
	static constexpr double MAX_BOUND_VAL = -5;

	static constexpr double LEFT_TRANFORMED_BOUND = 0;
	static constexpr double RIGHT_TRANFORMED_BOUND = 100;
};

} /* namespace DP_GMM2 */

#endif /* DP_GMM2_LOGCONCAVEFUNCTION_H_ */
