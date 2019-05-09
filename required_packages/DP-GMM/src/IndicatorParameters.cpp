/*
 * IndicatorSampler.cpp
 *
 *  Created on: Jul 17, 2013
 *      Author: jan
 */

#include "IndicatorParameters.h"

namespace DP_GMM {

    IndicatorParameters::IndicatorParameters(const EiMatrix &data,
                                             EstimationMixtureComponentSet *mixture_components, RngPtr r) :
            _data(data), _num_components(0), _mixture_components(
            mixture_components), _component_by_data_point(
            new EstimationMixtureComponentPtr[data.rows()]), _D(data.cols()), _alpha(
            0.0), _beta(0.0), _W(_D, _D), _rho(0.0), _xi(_D), _zero_vector(
            EiVector::Zero(_D)), _inv_beta_W(_D, _D), _log_det_beta_W(0.0), dist_uniform(0, 1), _r(r) {
    }

    IndicatorParameters::~IndicatorParameters() { delete[] _component_by_data_point; }

    void IndicatorParameters::sampleIndicators(
            const HyperParameters &hyper_parameters) {

        _setHyperParams(hyper_parameters);
        _updateMixtures();

        for (int i = 0; i < _data.rows(); i++) {
            EstimationMixtureComponentPtr current_mixture = _component_by_data_point[i];
            _removeDataPointFromComponent(i, current_mixture);

            double prob_new_comp;
            double sum_probs;

            _computeProbs(&sum_probs, &prob_new_comp, i);
            _sampleComponent(sum_probs, prob_new_comp, i);
        }
    }

    void IndicatorParameters::_setHyperParams(
            const HyperParameters &hyper_parameters) {
        _alpha = hyper_parameters.getAlpha();
        _beta = hyper_parameters.getBeta();
        _W = hyper_parameters.getW();
        _rho = hyper_parameters.getRho();
        _xi = hyper_parameters.getXi();

        EiLU _beta_W_lu(_beta * _W);

        _inv_beta_W = _beta_W_lu.inverse();
        _log_det_beta_W = log(_beta_W_lu.determinant());
    }

    void IndicatorParameters::_updateMixtures() {
        for (EstimationMixtureComponentSet::iterator it = _mixture_components->begin();
             it != _mixture_components->end(); it++) {
            EstimationMixtureComponentPtr comp = (*it);
            comp->updateWStar(_xi, _rho, _W, _beta);
        }

    }

    void IndicatorParameters::_removeDataPointFromComponent(int i,
                                                            EstimationMixtureComponentPtr current_mixture) {
        _component_by_data_point[i] = EstimationMixtureComponentPtr(nullptr);
        if (current_mixture) {
            bool delete_component = current_mixture->removeDataPoint(i, _xi, _rho,
                                                                     _W, _beta);
            if (delete_component) {
                _num_components--;
                int num_erased_comps = _mixture_components->erase(current_mixture);

#ifdef DEBUG
                if (_num_components != _mixture_components->size() || num_erased_comps != 1) {
                    std::cerr << "number of components inconsistant!" << std::endl;
                }
#endif
            }
        }
    }

    void IndicatorParameters::_computeProbs(double *sum_probs, double *prob_new_comp,
                                            int i) {
        *sum_probs = 0.0;
        double log_gamma_val = 0.0;

        _probs_by_components.clear();

        if (_mixture_components->empty()) {
            *sum_probs = 1.0;
            *prob_new_comp = 1.0;
            return;
        }
        for (EstimationMixtureComponentSet::iterator it = _mixture_components->begin();
             it != _mixture_components->end(); it++) {
            EstimationMixtureComponentPtr comp = *it;
            log_gamma_val = _multivariateGammaLog(_beta + comp->getNumDataPoints(),
                                                  1, _D);
            *prob_new_comp = (comp->getNumDataPoints()) / (_data.rows() - 1 + _alpha)
                             * exp(
                    _logLikeIndicators(i, comp->getNumDataPoints(),
                                       comp->getEmpiricalComponentMean(),
                                       comp->getInvWStar(), comp->getLogDetWStar(),
                                       log_gamma_val));
#ifdef DEBUG
            if (std::isnan(*prob_new_comp) || std::isinf(*prob_new_comp)) {
                std::cerr
                        << "error with the computation of the component probability occured..."
                        << std::endl;
                (comp->getNumDataPoints()) / (_data.rows() - 1 + _alpha)
                * exp(
                        _logLikeIndicators(i, comp->getNumDataPoints(),
                                           comp->getEmpiricalComponentMean(),
                                           comp->getInvWStar(), comp->getLogDetWStar(),
                                           log_gamma_val));
            }
#endif

            *sum_probs += *prob_new_comp;
            _probs_by_components.insert(
                    std::pair<EstimationMixtureComponentPtr, double>(comp, (double) *prob_new_comp));
        }
        log_gamma_val = _multivariateGammaLog(_beta, 1, _D);
        *prob_new_comp = (_alpha / (_data.rows() - 1 + _alpha)
                          * exp(
                _logLikeIndicators(i, 0, _zero_vector, _inv_beta_W,
                                   _log_det_beta_W, log_gamma_val)));
#ifdef DEBUG
        if (std::isnan(*prob_new_comp) || std::isinf(*prob_new_comp)) {
            std::cerr
                    << "error with the computation of the component probability occured..."
                    << std::endl;
            (_alpha / (_data.rows() - 1 + _alpha)
             * exp(
                    _logLikeIndicators(i, 0, _zero_vector, _inv_beta_W,
                                       _log_det_beta_W, log_gamma_val)));
        }
#endif
        *sum_probs += *prob_new_comp;
    }

    double IndicatorParameters::_logLikeIndicators(int data_point_index,
                                                   double num_data_points_in_component,
                                                   const EiVector &emp_component_mean,
                                                   const EiMatrix &inv_W_star, double log_det_W_star,
                                                   double log_gamma_val) {

        EiConstSubVectorRow x = _data.row(data_point_index);

        double tmp_val =
                (((num_data_points_in_component * emp_component_mean) + _rho * _xi)
                 * (1.0 / (_rho + num_data_points_in_component))
                 - x.transpose()).transpose() * inv_W_star
                * (((num_data_points_in_component * emp_component_mean)
                    + _rho * _xi)
                   * (1.0 / (_rho + num_data_points_in_component))
                   - x.transpose());
        tmp_val = ((_rho + num_data_points_in_component)
                   / (_rho + num_data_points_in_component + 1)) * tmp_val;

#ifdef DEBUG
        if (tmp_val <= -1) {
            std::cout << "something went wrong: " << std::endl;
            EiVector vec_tmp = (((num_data_points_in_component * emp_component_mean) + _rho * _xi)
                                * (1.0 / (_rho + num_data_points_in_component))
                                - x.transpose());
            std::cout << "vec_tmp " << vec_tmp << std::endl;
            std::cout << "determinant of det w star " << inv_W_star.determinant() << std::endl;
            std::cout << "and factor is " << ((_rho + num_data_points_in_component)
                                              / (_rho + num_data_points_in_component + 1)) << std::endl;
        }
#endif
        double like = 0.0;
        like +=
                (_D / 2.0)
                * log(
                        (_rho + num_data_points_in_component)
                        / ((double) (_rho
                                     + num_data_points_in_component + 1)));
        like = like - (1 * _D / 2.0) * log(M_PI) + log_gamma_val;
        like -= ((_beta + num_data_points_in_component + 1) / 2.0)
                * log(1 + tmp_val);
        like -= ((1) / 2.0) * log_det_W_star;

        return like;
    }

    void IndicatorParameters::_sampleComponent(double sum_probs,
                                               double prob_new_comp, int data_point_index) {
        double u = dist_uniform(*_r);
        double a = sum_probs * u;
        EstimationMixtureComponentPtr component;

        if (a <= prob_new_comp) {

            component = std::make_shared<EstimationMixtureComponent>(_data, data_point_index,
                                                             _W, _beta, _xi, _rho, std::ref(_r));
            _mixture_components->insert(component);
            _num_components++;
#ifdef DEBUG
            if(_num_components != _mixture_components->size()) {
                std::cerr<< "number of components inconsistant!" << std::endl;
            }
#endif
        } else {
            a -= prob_new_comp;
            for (EstimationMixtureComponentSet::iterator it = _mixture_components->begin();
                 it != _mixture_components->end(); it++) {
                component = *it;
                if (a < _probs_by_components.at(component)) {
                    component->addDataPoint(data_point_index, _xi, _rho, _W, _beta);
                    break;
                } else {
                    a -= _probs_by_components.at(component);
                }
            }
        }

        _component_by_data_point[data_point_index] = component;
    }

    double IndicatorParameters::_multivariateGammaLog(double x, double y,
                                                      size_t D) {
        double result = 0.0;
        if (y == 1) {
            result = std::lgamma((x + y) / 2.0)
                     - std::lgamma((x + y - D) / 2.0);
        } else {
            for (size_t i = 0; i < D; i++) {
                result += std::lgamma((x + y - i) / 2.0)
                          - std::lgamma((x - i) / 2.0);
            }
        }
        return result;
    }

} /* namespace DP_GMM2 */
