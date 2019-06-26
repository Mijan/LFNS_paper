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
                                                                               _inv_evs(data.size(), data.size()),
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

        _inv_evs = _evs.inverse();

        base::EiMatrixC transformed = _inv_evs.real() * centered.transpose();
        transformed.transposeInPlace();

        base::EiMatrix transformed_real = transformed.real();

        updateTransformedDensitySamples(transformed_real);
    }

    std::vector<double> &DensityEstimation::sample() {
        do {
            sampleTransformed(_trans_sample);
            retransformSample(_trans_sample);

        } while (!isSampleFeasible(_trans_sample));
        memcpy(_sample.data(), _trans_sample.real().data(), sizeof(double) * _trans_sample.size());
        return _sample;
    }

    double DensityEstimation::getLogLikelihood(const std::vector<double> &sample) {
        _trans_sample = Eigen::Map<const base::EiVector>(sample.data(), sample.size());
        transformSample(_trans_sample);
        return getTransformedLogLikelihood(_trans_sample);
    }


    void DensityEstimation::transformSample(base::EiVector &sample) {
        sample -= _mean;
        sample = _inv_evs.real() * sample;
    }

    void DensityEstimation::retransformSample(base::EiVector &sample) {
        sample = (_evs * sample.cast<std::complex<double> >()).real();
        sample += _mean;
    }
}
