//
// Created by jan on 09/10/18.
//

#include "DpGmmSampler.h"
#include "../base/MultivariateNormal.h"

namespace sampler {
    DpGmmSampler::DpGmmSampler(base::RngPtr rng, DpGmmSamplerData data) : DensityEstimation(rng, data),
                                                                          _mixture_components(),
                                                                          _dist(0, 1), _sample_ei_vector(),
                                                                          _hyper_params_set(false),
                                                                          _estimator(rng, data.num_dp_iterations),
                                                                          _hyper_parameters(
                                                                                  base::EiMatrix(1, 1), rng),
                                                                          _num_dp_gmm_iteration(
                                                                                  data.num_dp_iterations) {}


    DpGmmSampler::DpGmmSampler(base::RngPtr rng, DpGmmSamplerData data,
                               DP_GMM::GaussMixtureComponentSet mixture_components) : DensityEstimation(rng, data),
                                                                                      _mixture_components(
                                                                                              mixture_components),
                                                                                      _dist(0, 1), _sample_ei_vector(),
                                                                                      _hyper_params_set(true),
                                                                                      _estimator(rng,
                                                                                                 data.num_dp_iterations),
                                                                                      _hyper_parameters(
                                                                                              base::EiMatrix(1, 1),
                                                                                              rng),
                                                                                      _num_dp_gmm_iteration(
                                                                                              data.num_dp_iterations) {}

    DpGmmSampler::~DpGmmSampler() {}

    void DpGmmSampler::writeToStream(std::ostream &stream) {
        stream << "DPGMM sampler with " << _mixture_components.size() << " components." << std::endl;
    }

    void DpGmmSampler::updateTransformedDensitySamples(base::EiMatrix transformed_samples) {
        DP_GMM::EstimationMixtureComponentSet mixtures(0);
        if (_hyper_params_set) {
            _estimator.estimate(transformed_samples, _hyper_parameters, &mixtures);
        } else {
            _hyper_parameters = _estimator.estimate(transformed_samples, &mixtures);
            _hyper_params_set = true;
        }

        _mixture_components.clear();
        DP_GMM::EstimationMixtureComponentSet::iterator it;
        for (it = mixtures.begin(); it != mixtures.end(); it++) {
            DP_GMM::EstimationMixtureComponentPtr comp = *it;
            _mixture_components.insert(std::make_shared<DP_GMM::GaussMixtureComponent>(comp->getGaussComponent()));
        }
    }

    void DpGmmSampler::sampleTransformed(base::EiVector &trans_sample) {
        DP_GMM::GaussMixtureComponentPtr comp;
        DP_GMM::GaussMixtureComponentSet::iterator it = _mixture_components.begin();
        double u = _dist(*_rng);
        double w = 0.0;
        while (w < u) {
            comp = *it;
            w += comp->comp_weight;
            it++;
        }

        const base::EiVector &mean = comp->mean;
        const base::EiMatrix &decomp_cov = comp->decomposed_cov;

        base::MultivariateNormal::mvnormRndWithDecomposedVar(_rng, _sample.size(), mean, decomp_cov, &trans_sample);
    }

    double DpGmmSampler::getTransformedLogLikelihood(const base::EiVector &trans_sample) {
        double likelihood = 0.0;

        std::unordered_set<DP_GMM::GaussMixtureComponentPtr>::const_iterator it;
        for (it = _mixture_components.begin(); it != _mixture_components.end(); it++) {
            const DP_GMM::GaussMixtureComponentPtr comp = *it;
            likelihood += comp->comp_weight
                          * base::MultivariateNormal::mvnormPrepared(
                    trans_sample.size(), trans_sample, comp->mean, comp->precision,
                    1.0 / comp->precision_det);
        }
        return log(likelihood);
    }

}