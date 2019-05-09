/*
 * EstimationMixtureComponent.h
 *
 *  Created on: Sep 2, 2013
 *      Author: jan
 */

#ifndef DP_GMM2_DPMIXTURECOMPONENT_H_
#define DP_GMM2_DPMIXTURECOMPONENT_H_

#include <cmath>
#include <iostream>

#include <unordered_set>

#include "BoostDistributions.h"
#include "GaussMixtureComponent.h"
#include "MatrixTypes.h"
#include "WishartDistribution.h"

namespace DP_GMM {
    typedef EiMatrix Data;

    class EstimationMixtureComponent {
    public:
        EstimationMixtureComponent(const EiMatrix &data, int data_point_index,
                           const EiMatrix &W, double beta, const EiVector &xi, double rho,
                           RngPtr r);

        ~EstimationMixtureComponent();

        const EiVector &getMean() const { return _mean; }

        const EiMatrix &getCovariance() const { return _covariance; }

        const EiMatrix &getPrecision() const { return _precision; }

        double getPrecisionDet() const { return _precision_det; }

        double getComponentWeight() const { return ((double) getNumDataPoints()) / (double) _num_data_points; }

        const EiVector &getEmpiricalComponentMean() const { return _empirical_component_mean; }

        const EiMatrix &getInvWStar() const { return _inv_W_star; }

        double getLogDetWStar() const { return _log_det_W_star; }

        std::size_t getNumDataPoints() const { return _data_points.size(); }

        const std::unordered_set<int> &getDataPoints() { return _data_points; }

        void setDataPoints(std::unordered_set<int> data_points) { _data_points = data_points; }

        void setComponentWeight(double component_weight) { _component_weight = component_weight; }

        void addDataPoint(int index, const EiVector &xi, double rho, const EiMatrix &W, double beta);

        bool removeDataPoint(int index, const EiVector &xi, double rho, const EiMatrix &W, double beta);

        void updateWStar(const EiVector &xi, double rho, const EiMatrix &W, double beta);

        void updateMixtureParameters(const EiVector &xi, double rho, const EiMatrix &W, double beta);

        void scaleCompWeight(double scale) { _component_weight = _component_weight * scale; }

        GaussMixtureComponent getGaussComponent();

    private:
        const Data &_data;
        const RngPtr _r;
        const int _num_data_points;

        EiVector _mean;
        EiMatrix _covariance;
        EiMatrix _precision;

        EiVector _empirical_component_mean;
        EiVector _sum_weighted_data_points;

        EiMatrix _empirical_component_mean_matrix;
        EiMatrix _sum_weighted_data_matrix;

        EiMatrix _inv_W_star;
        EiMatrix _regulization_matrix;
        std::unordered_set<int> _data_points;

        double _precision_det;
        double _component_weight;
        double _log_det_W_star;

        WishartDistribution _dist_wishart;

        void sampleMeanFromPosteriorWithPrecision(const EiVector &xi, double rho,
                                                  const EiMatrix &W, double beta);

        void recomputeEmpiricalMeanAndMatrix();

        void invertRankOneUpdate(const EiVector &u, double scal, EiMatrix *inv_A);

        void computeUpdateVectorForWStar(double rho, const EiVector &xi,
                                         const EiConstSubVectorRow data_point, double *update_factor,
                                         EiVector *update_vector);
    };

    typedef std::shared_ptr<EstimationMixtureComponent> EstimationMixtureComponentPtr;
    typedef std::unordered_set<EstimationMixtureComponentPtr> EstimationMixtureComponentSet;
} /* namespace DP_GMM2 */

#endif /* DP_GMM_DPMIXTURECOMPONENT_H_ */
