/*
 * MixtureComponent.cpp
 *
 *  Created on: Sep 2, 2013
 *      Author: jan
 */

#include "EstimationMixtureComponent.h"

#include <cmath>
#include <iostream>

#include "WishartDistribution.h"
#include "MultivariateNormalDistribution.h"

namespace DP_GMM {

    EstimationMixtureComponent::EstimationMixtureComponent(const Data &data, int data_point_index,
                                           const EiMatrix &W, double beta, const EiVector &xi, double rho,
                                           RngPtr r) :
            _data(data), _r(r), _num_data_points(data.rows()), _mean(data.cols()), _covariance(data.cols(),
                                                                data.cols()), _precision(data.cols(), data.cols()),
            _empirical_component_mean(data.cols()), _sum_weighted_data_points(data.cols()),
            _empirical_component_mean_matrix(data.cols(), data.cols()),
            _sum_weighted_data_matrix(data.cols(), data.cols()), _inv_W_star(data.cols(), data.cols()),
            _regulization_matrix(Eigen::MatrixXd::Identity(data.cols(), data.cols())), _dist_wishart() {

        EiLU W_star_lu(W * beta);
        _inv_W_star = W_star_lu.inverse();
        _log_det_W_star = log(W_star_lu.determinant());

        _regulization_matrix *= 1e-4;
        addDataPoint(data_point_index, xi, rho, W, beta);
    }

    EstimationMixtureComponent::~EstimationMixtureComponent() { }

    void EstimationMixtureComponent::addDataPoint(int index, const EiVector &xi, double rho,
                                          const EiMatrix &W, double beta) {

        _data_points.insert(index);
        if (_data_points.size() > 10) {
            EiConstSubVectorRow data_point = _data.row(index);

            //update _W_star matrix
            double update_factor = 0.0;
            EiVector update_vector(_data.cols());

            computeUpdateVectorForWStar(rho, xi, data_point, &update_factor,
                                        &update_vector);
            invertRankOneUpdate(update_vector, update_factor, &_inv_W_star);

            //update _empirical_component_mean
            _sum_weighted_data_points += data_point.transpose();
            _empirical_component_mean = _sum_weighted_data_points
                                        * (1.0 / ((double) _data_points.size()));

            //update _empirical_component_mean_matrix. Note that data_point is a row vector!
            _sum_weighted_data_matrix += (data_point.transpose() * data_point);

            _empirical_component_mean_matrix = _sum_weighted_data_matrix
                                               * (1.0 / ((double) _data_points.size()));

            // update rest
            _component_weight += 1;
            _log_det_W_star = log(1.0 / _inv_W_star.lu().determinant());
            if (std::isnan(_log_det_W_star) || std::isinf(_log_det_W_star)) {
                std::cout << "+";
                updateWStar(xi, rho, W, beta);
            }
        } else {
            updateWStar(xi, rho, W, beta);
        }
    }

    bool EstimationMixtureComponent::removeDataPoint(int index, const EiVector &xi,
                                             double rho, const EiMatrix &W, double beta) {
        //returns true if the component is empty after this operation

        _data_points.erase(index);
        if (_data_points.size() > 10) {

            EiConstSubVectorRow data_point = _data.row(index);

            //update _empirical_component_mean
            _sum_weighted_data_points -= data_point.transpose();
            _empirical_component_mean = _sum_weighted_data_points
                                        * (1.0 / ((double) _data_points.size()));

            //update _empirical_component_mean_matrix. Note that data_point is a row vector!
            _sum_weighted_data_matrix -= (data_point.transpose() * data_point);

            _empirical_component_mean_matrix = _sum_weighted_data_matrix
                                               * (1.0 / ((double) _data_points.size()));

            //update _W_star matrix
            double update_factor = 0.0;
            EiVector update_vector(_data.cols());

            computeUpdateVectorForWStar(rho, xi, data_point, &update_factor,
                                        &update_vector);
            invertRankOneUpdate(update_vector, -update_factor, &_inv_W_star);

            // update rest
            _component_weight -= 1 / ((double) _data.cols());
            _log_det_W_star = log(1.0 / _inv_W_star.lu().determinant());
            if (std::isnan(_log_det_W_star) || std::isinf(_log_det_W_star)) {
                std::cout << "+";
                updateWStar(xi, rho, W, beta);
            }
        } else if (_data_points.size() > 0) {
            updateWStar(xi, rho, W, beta);
        }

        if (_data_points.empty()) {
            return true;
        } else {
            return false;
        }

    }

    void EstimationMixtureComponent::computeUpdateVectorForWStar(double rho,
                                                         const EiVector &xi, EiConstSubVectorRow data_point,
                                                         double *update_factor, EiVector *update_vector) {

        std::size_t n = _data_points.size();
        *update_factor = (rho + n) / (rho + n + 1);
        *update_vector = ((_empirical_component_mean * n) + (xi * rho))
                         * (1.0 / (rho + n)) - data_point.transpose();
    }

    void EstimationMixtureComponent::updateMixtureParameters(const EiVector &xi, double rho,
                                                     const EiMatrix &W, double beta) {
        updateWStar(xi, rho, W, beta);
        _dist_wishart.wishRnd(_inv_W_star, _mean.size(), std::max<double>(beta, 1) + _data_points.size(), _r,
                              &_precision);

        EiLU _precision_lu(_precision);
        _precision_det = _precision_lu.determinant();
        _covariance = _precision_lu.inverse();
        sampleMeanFromPosteriorWithPrecision(xi, rho, W, beta);
    }

    void EstimationMixtureComponent::updateWStar(const EiVector &xi, double rho,
                                         const EiMatrix &W, double beta) {
        recomputeEmpiricalMeanAndMatrix();
        double num_data_points = _data_points.size();

        int n = _mean.size();

        EiMatrix tmp_matrix_2(n, n);

        if (_data_points.size() > 1) {
            tmp_matrix_2 = (_empirical_component_mean_matrix
                            - (_empirical_component_mean
                               * _empirical_component_mean.transpose()))
                           * num_data_points;
        } else {
            tmp_matrix_2.setZero();
        }

        EiMatrix tmp_matrix_3 = ((_empirical_component_mean - xi)
                                 * (_empirical_component_mean - xi).transpose())
                                * ((rho * num_data_points) / (rho + num_data_points));

        EiLU W_star_lu(beta * W + tmp_matrix_2 + tmp_matrix_3);

        _inv_W_star = W_star_lu.inverse();

        _log_det_W_star = log(W_star_lu.determinant());
        if (std::isnan(_log_det_W_star) || std::isinf(_log_det_W_star)) {
            //if the determinant is still negative then the matrices must be really bad conditioned... in this case the  we just choose a value for log_det_w_star
            std::cout << "*";
            EiLU W_star_lu_regulized(
                    beta * W + tmp_matrix_2 + tmp_matrix_3 + _regulization_matrix);

            _inv_W_star = W_star_lu_regulized.inverse();

            _log_det_W_star = log(W_star_lu_regulized.determinant());
        }
    }

    void EstimationMixtureComponent::sampleMeanFromPosteriorWithPrecision(
            const EiVector &xi, double rho, const EiMatrix &W, double beta) {
        int n = _mean.size();
        double num_data_points = _data_points.size();

        EiVector mu_mean = ((_empirical_component_mean * num_data_points)
                            + (rho * xi)) * (1.0 / (rho + num_data_points));
        EiMatrix cov_mean = _covariance * (1.0 / (rho + num_data_points));

        MultivariateNormalDistribution::mvnormRnd(_r, n, mu_mean, cov_mean, &_mean);
        if (_mean(0) != _mean(0)) {
            std::cout << "mixture component mean is nan " << _mean(0) << std::endl;
        }
    }

    void EstimationMixtureComponent::recomputeEmpiricalMeanAndMatrix() {
        _empirical_component_mean.setZero();
        _sum_weighted_data_points.setZero();
        _empirical_component_mean_matrix.setZero();
        _sum_weighted_data_matrix.setZero();
        _component_weight = 0.0;

        for (std::unordered_set<int>::iterator it = _data_points.begin();
             it != _data_points.end(); it++) {

            int data_point_index = *it;

            EiConstSubVectorRow data_point = _data.row(data_point_index);

            _sum_weighted_data_points += data_point.transpose();

            //update _empirical_component_mean_matrix. Note that data_point is a row vector!
            _sum_weighted_data_matrix += (data_point.transpose() * data_point);
        }
        _component_weight = ((double) _data_points.size())
                            / ((double) _data.rows());

        _empirical_component_mean = _sum_weighted_data_points
                                    * (1.0 / _data_points.size());
        _empirical_component_mean_matrix = _sum_weighted_data_matrix
                                           * (1.0 / _data_points.size());
    }

    void EstimationMixtureComponent::invertRankOneUpdate(const EiVector &u, double scal,
                                                 EiMatrix *inv_A) {

        EiVector inv_A_u = (*inv_A) * u;
        double g = u.transpose() * inv_A_u;
        g = 1.0 / (1 + scal * g);

        *inv_A = (*inv_A) - (inv_A_u * inv_A_u.transpose()) * (g * scal);
    }

    GaussMixtureComponent EstimationMixtureComponent::getGaussComponent() {
        return GaussMixtureComponent(_mean, _covariance, _precision, _precision_det,
                                     _component_weight);
    }

} /* namespace DP_GMM2 */
