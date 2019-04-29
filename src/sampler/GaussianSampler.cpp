//
// Created by jan on 09/10/18.
//

#include <cfloat>
#include "GaussianSampler.h"
#include "../base/MultivariateNormal.h"

namespace sampler {

    GaussianSampler::GaussianSampler(base::RngPtr rng, NormalSamplerData data) : Sampler(rng, data),
                                                                                 _mean(data.mean),
                                                                                 _inverse_cov_matrix(
                                                                                         data.cov.lu().inverse()),
                                                                                 _decomposed_var(data.bounds.size(),
                                                                                                 data.bounds.size()),
                                                                                 _cov(data.cov), _det_var(0.0),
                                                                                 _n(data.bounds.size()) {

        _inverse_cov_matrix = data.cov.inverse();

        base::MultivariateNormal::decomposeVar(data.cov, &_decomposed_var);
        _det_var = data.cov.determinant();
    }

    GaussianSampler::~GaussianSampler() {}

    std::vector<double> &GaussianSampler::sample() {
        do {
            base::MultivariateNormal::mvnormRndWithDecomposedVar(_rng, _n,
                                                                 _mean, _decomposed_var,
                                                                 &_sample);
        } while (!isSampleFeasible(_sample));
        return _sample;
    }

    double GaussianSampler::getLogLikelihood(const std::vector<double> &sample) {
        if (!isSampleFeasible(sample)) {
            return -DBL_MAX;
        }
        return log(base::MultivariateNormal::mvnormPrepared(_n, sample, _mean,
                                                            _inverse_cov_matrix,
                                                            _det_var));
    }

    void GaussianSampler::setMean(const base::EiVectorRef &mean) { _mean = mean; }

    void GaussianSampler::setVar(const base::EiMatrixRef &var) {
        _inverse_cov_matrix = var.inverse();

        base::MultivariateNormal::decomposeVar(var, &_decomposed_var);
        _det_var = var.lu().determinant();
        if ((var.rows() != _n) || var.cols() != _n) {
            std::stringstream os;
            os << "Tried to set covariance matrix with " << var.rows() << " rows and " << var.cols() << " columns, "
               << ", but dimensionality is " << _n
               << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    void GaussianSampler::setBounds(std::vector<std::pair<double, double> > _bounds) {
        Sampler::setBounds(_bounds);
        _n = _bounds.size();
    }


    void GaussianSampler::updateKernel(const base::EiMatrix &transformed_samples) {
        _cov = (transformed_samples.adjoint() * transformed_samples) / double(transformed_samples.rows() - 1);
        _inverse_cov_matrix = _cov.inverse();

        base::MultivariateNormal::decomposeVar(_cov, &_decomposed_var);
        _det_var = _cov.determinant();
    }


    std::vector<double> &GaussianSampler::sample(const std::vector<double> &kernel_center) {
        base::EiConstVectorRef new_mean = Eigen::Map<const Eigen::VectorXd>(kernel_center.data(), _n);

        base::MultivariateNormal::mvnormRndWithDecomposedVar(_rng, _n, new_mean, _decomposed_var, &_sample);
        return _sample;
    }


    void GaussianSampler::sample(base::EiVector &sample, const std::vector<double> &kernel_center) {
        base::EiConstVectorRef new_mean = Eigen::Map<const Eigen::VectorXd>(kernel_center.data(), _n);

        base::MultivariateNormal::mvnormRndWithDecomposedVar(_rng, _n, new_mean, _decomposed_var, &sample);
    }

    double GaussianSampler::getLogLikelihood(const std::vector<double> &sample,
                                             const std::vector<double> &kernel_center) {


        base::EiConstVectorRef new_mean = Eigen::Map<const Eigen::VectorXd>(kernel_center.data(), _n);
        return std::log(
                base::MultivariateNormal::mvnormPrepared(_n, sample, new_mean, _inverse_cov_matrix, _det_var));
    }

    double GaussianSampler::getLogLikelihood(const base::EiVector &sample, const std::vector<double> &kernel_center) {
        base::EiConstVectorRef new_mean = Eigen::Map<const Eigen::VectorXd>(kernel_center.data(), _n);
        return std::log(
                base::MultivariateNormal::mvnormPrepared(_n, sample, new_mean, _inverse_cov_matrix, _det_var));
    }

    void GaussianSampler::writeToStream(std::ostream &stream) {
        stream << "Normal Sampler" << std::endl;
        stream << "mean: ";
        stream << _mean;
        stream << "\ninverse covariance: ";
        stream << _inverse_cov_matrix;
        stream << std::endl;

        Sampler::writeToStream(stream);
    }
}
