/*
 * HyperParameters.cpp
 *
 *  Created on: Jul 17, 2013
 *      Author: jan
 */

#include "HyperParameters.h"

#include <fstream>

#include "PostLogAlpha.h"
#include "PostLogBeta.h"

namespace DP_GMM {

    HyperParameters::HyperParameters(const Data &data, RngPtr r) :
            _r(r), _D(data.cols()), _xi(_D), _W(_D, _D), _sigma_data(_D, _D), _mu_data(
            _D), _inv_sigma_data(_D, _D), _sampler(r), _dist_wishart() {

        _mu_data = data.colwise().sum() * (1.0 / (double) data.rows());

        _sigma_data.setZero();

        for (int i = 0; i < data.rows(); i++) {
            EiVector a = (data.row(i).transpose() - _mu_data);
            EiMatrix test = a * a.transpose();
            _sigma_data = _sigma_data
                          + ((data.row(i).transpose() - _mu_data)
                             * ((data.row(i).transpose() - _mu_data).transpose()));
        }

        _sigma_data *= 1.0 / ((double) data.rows());

        GammaDistribution gamma_alpha(0.5, 2);
        _alpha = gamma_alpha(*r);
        _alpha = 1.0 / _alpha;

        GammaDistribution gamma_beta(0.5, 2.0 / (double) _D);
        _beta = gamma_beta(*r);
        _beta = (1.0 / (double) _beta) + _D - 1;

        MultivariateNormalDistribution::mvnormRnd(_r, _D, _mu_data, _sigma_data,
                                                  &_xi);

        WishartDistribution wish;
        wish.wishRnd(_sigma_data * (1.0 / (double) _D), _D, _D, _r,
                     &_W);

        GammaDistribution gamma_rho(0.25, 2);
        _rho = gamma_rho(*r);

        _inv_sigma_data = _sigma_data.lu().inverse();
    }

    HyperParameters::HyperParameters(const EiMatrix &data, RngPtr r, std::string hyper_params_file_name) : _r(r),
                                                                                                           _D(data.cols()),
                                                                                                           _xi(_D),
                                                                                                           _W(_D, _D),
                                                                                                           _sigma_data(
                                                                                                                   _D,
                                                                                                                   _D),
                                                                                                           _mu_data(_D),
                                                                                                           _inv_sigma_data(
                                                                                                                   _D,
                                                                                                                   _D),
                                                                                                           _sampler(r) {
        std::ifstream hyper_params_file(hyper_params_file_name.c_str());

        if (!hyper_params_file.is_open()) {
            std::cout << "Hyper parameters file " << hyper_params_file_name
                      << " could not be read! Using default hyper parameters."
                      << std::endl;
        }

        int num_dims = data.cols();
        double alpha;
        double beta;
        EiVector xi(num_dims);
        EiMatrix W(num_dims, num_dims);
        double rho;

        hyper_params_file >> alpha;
        hyper_params_file >> beta;
        for (int i = 0; i < num_dims; i++) {
            hyper_params_file >> xi(i);
        }

        for (int i = 0; i < num_dims; i++) {
            for (int j = 0; j < num_dims; j++) {
                hyper_params_file >> W(i, j);
            }
        }
        hyper_params_file >> rho;

        hyper_params_file.close();

        _xi = xi;
        _W = W;
        _alpha = alpha;
        _beta = beta;
        _rho = rho;


        _mu_data = data.colwise().sum() * (1.0 / (double) data.rows());

        _sigma_data.setZero();

        for (int i = 0; i < data.rows(); i++) {
            EiVector a = (data.row(i).transpose() - _mu_data);
            EiMatrix test = a * a.transpose();
            _sigma_data = _sigma_data
                          + ((data.row(i).transpose() - _mu_data)
                             * ((data.row(i).transpose() - _mu_data).transpose()));
        }

        _sigma_data *= 1.0 / ((double) data.rows());
        _inv_sigma_data = _sigma_data.lu().inverse();
    }

    HyperParameters::HyperParameters(const Data &data, RngPtr r, double alpha,
                                     double beta, EiVector xi, EiMatrix W, double rho) :
            _r(r), _D(data.cols()), _xi(xi), _W(W), _sigma_data(_D, _D), _mu_data(
            _D), _inv_sigma_data(_D, _D), _sampler(r), _alpha(alpha), _beta(
            beta), _rho(rho) {

        _mu_data = data.colwise().sum() * (1.0 / (double) data.rows());

        _sigma_data.setZero();

        for (int i = 0; i < data.rows(); i++) {
            EiVector a = (data.row(i).transpose() - _mu_data);
            EiMatrix test = a * a.transpose();
            _sigma_data = _sigma_data
                          + ((data.row(i).transpose() - _mu_data)
                             * ((data.row(i).transpose() - _mu_data).transpose()));
        }

        _sigma_data *= 1.0 / ((double) data.rows());
        _inv_sigma_data = _sigma_data.lu().inverse();

    }

    HyperParameters::~HyperParameters() {}

    double HyperParameters::getAlpha() const { return _alpha; }

    double HyperParameters::getBeta() const { return _beta; }

    const EiVector &HyperParameters::getXi() const { return _xi; }

    const EiMatrix &HyperParameters::getW() const { return _W; }

    double HyperParameters::getRho() const { return _rho; }

    void HyperParameters::updateAlpha(int num_samples, int num_components) {

        try {
            PostLogAlpha post_log_alpha(num_samples, num_components, log(_alpha));
            _alpha = _sampleLogConcave(post_log_alpha);
        } catch (std::exception &e) {
            std::cerr << "Could not resample alpha : ";
            std::cerr << e.what();
            std::cerr << "Assume previous value for alpha " << _alpha << std::endl;
        }
    }

    void HyperParameters::updateBeta(const EstimationMixtureComponentSet &components) {

        try {
            PostLogBeta post_log_beta(components, _W, _D, log(_beta));
            _beta = _sampleLogConcave(post_log_beta);
        } catch (std::exception &e) {
            std::cerr << "Could not resample beta : ";
            std::cerr << e.what();
            std::cerr << "Assume previous value for beta " << _beta << std::endl;
        }
    }

    void HyperParameters::updateXi(const EstimationMixtureComponentSet &components) {

        EiMatrix sum_precision(_D, _D);
        sum_precision.setZero();

        EiVector sum_prec_mu(_D);
        sum_prec_mu.setZero();

        for (EstimationMixtureComponentSet::const_iterator it = components.begin();
             it != components.end(); it++) {
            EstimationMixtureComponentPtr comp = *it;
            sum_precision += comp->getPrecision();
            sum_prec_mu += comp->getPrecision() * comp->getMean();
        }

        sum_precision *= _rho;
        sum_precision += _inv_sigma_data;
        EiMatrix cov_xi = sum_precision.lu().inverse();

        sum_prec_mu *= _rho;

        EiVector mu_xi = cov_xi * (_inv_sigma_data * _mu_data + sum_prec_mu);

        MultivariateNormalDistribution::mvnormRnd(_r, _D, mu_xi, cov_xi, &_xi);

    }

    void HyperParameters::updateW(const EstimationMixtureComponentSet &components) {
        int K = components.size();

        EiMatrix sum_precision(_D, _D);
        sum_precision.setZero();

        for (EstimationMixtureComponentSet::const_iterator it = components.begin();
             it != components.end(); it++) {
            EstimationMixtureComponentPtr comp = *it;
            sum_precision += comp->getPrecision();
        }

        sum_precision *= _beta;
        sum_precision += _inv_sigma_data * _D;

        _dist_wishart.wishRnd(sum_precision.lu().inverse(), _D,
                              K * _beta + _D, _r, &_W);
        while (_W.determinant() <= 0) {
            EiMatrix _regulization_matrix = Eigen::MatrixXd::Identity(_W.cols(), _W.cols());
            _regulization_matrix *= 1e-4;
            _W += _regulization_matrix;

        }
    }

    void HyperParameters::updateRho(const EstimationMixtureComponentSet &components) {
        int K = components.size();

        double tmp_1 = 0.0;

        for (EstimationMixtureComponentSet::const_iterator it = components.begin();
             it != components.end(); it++) {
            EstimationMixtureComponentPtr comp = *it;

            tmp_1 += (comp->getMean() - _xi).transpose() * comp->getPrecision()
                     * (comp->getMean() - _xi);
        }

        if (tmp_1 != tmp_1) {
            for (EstimationMixtureComponentSet::const_iterator it = components.begin();
                 it != components.end(); it++) {
                EstimationMixtureComponentPtr comp = *it;
                std::cout << " the means are : " << comp->getMean() << std::endl;
            }

        }
        double aRho = (2.0 * K * _D + 1) / 4.0;
        double bRho = 2.0 / (double) (tmp_1 + 1);

        GammaDistribution gamma_rho(aRho, bRho);
        _rho = gamma_rho(*_r);
    }

    void HyperParameters::printHyperParams() {
        std::cout << "The hyper parameters are: alpha " << _alpha << ", beta "
                  << _beta << " rho " << _rho << std::endl;
        std::cout << "W matrix ";
        std::cout << _W << std::endl;
        std::cout << "xi vector ";
        std::cout << _xi << std::endl;
    }

    double HyperParameters::_sampleLogConcave(LogConcaveFunction &fct) {
        double sample = _sampler.sample(fct);
        return exp(sample);
    }

} /* namespace DP_GMM */
