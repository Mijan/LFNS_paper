//
// Created by jan on 09/10/18.
//

#include "DensityEstimation.h"
#include <stdio.h>
#include <iostream>


namespace sampler {
    DensityEstimation::DensityEstimation(base::RngPtr rng, SamplerData data) : Sampler(rng, data),
                                                                               _trans_sample(data.size()),
                                                                               _mean(data.size()),
                                                                               _evs(data.size(), data.size()),
                                                                               _evals(data.size()) {}

    void DensityEstimation::updateDensitySamples(base::EiMatrix &samples) {
        base::EiMatrix centered = samples.rowwise() - samples.colwise().mean();
        base::EiMatrix cov = (centered.adjoint() * centered) / double(samples.rows() - 1);

        Eigen::ComplexEigenSolver<base::EiMatrix> solver(cov);
        _evs = solver.eigenvectors();
        _evals = solver.eigenvalues();
        for (std::size_t col = 0; col < _evs.cols(); col++) {
            _evs.col(col) = _evs.col(col) * _evals(col);
        }

        _mean = samples.colwise().mean();


        base::EiMatrixC work_inverse = _evs.lu().inverse();

        base::EiMatrixC transformed = work_inverse.real() * centered.transpose();
        transformed.transposeInPlace();

        base::EiMatrix transformed_real = transformed.real();

        updateTransformedDensitySamples(transformed_real);
    }


    void DensityEstimation::setMean(base::EiVector mean) { _mean = mean; }

    void DensityEstimation::setEV(base::EiMatrixC evs, base::EiVectorC evals) {
        _evs = evs;
        _evals = evals;
    }

    std::vector<double> &DensityEstimation::sample() {
        do {
            sampleTransformed(_trans_sample);
            _trans_sample = (_evs * _trans_sample.cast<std::complex<double> >()).real();
            _trans_sample += _mean;

        } while (!isSampleFeasible(_trans_sample));
        memcpy(_sample.data(), _trans_sample.real().data(), sizeof(double) * _trans_sample.size());
        return _sample;
    }

    double DensityEstimation::getLogLikelihood(const std::vector<double> &sample) {
        sampleTransformed(_trans_sample);
        _trans_sample = (_evs * _trans_sample.cast<std::complex<double> >()).real();
        _trans_sample += _mean;
        return getTransformedLogLikelihood(_trans_sample);
    }
}
