/*
 * MatrixTypesDPGMM.h
 *
 *  Created on: Jan 24, 2014
 *      Author: jan
 */

#ifndef DP_GMM2_MATRIXTYPESDPGMM_H_
#define DP_GMM2_MATRIXTYPESDPGMM_H_

#include <Eigen/Dense>
#include <Eigen/Cholesky>

namespace DP_GMM {

typedef Eigen::VectorXd EiVector;
typedef Eigen::MatrixXd EiMatrix;
typedef Eigen::Block<Eigen::MatrixXd, 1, Eigen::Dynamic> EiVectorRow;
typedef Eigen::Block<Eigen::MatrixXd, Eigen::Dynamic, 1> EiVectorCol;
typedef Eigen::Block<const Eigen::MatrixXd, 1, Eigen::Dynamic> EiConstSubVectorRow;
typedef Eigen::Block<const Eigen::MatrixXd, Eigen::Dynamic, 1> EiConstSubVectorCol;

typedef Eigen::Block<Eigen::MatrixXd> EiSubMatrix;
typedef Eigen::Ref<Eigen::MatrixXd> EiMatrixRef;
typedef Eigen::Ref<const Eigen::MatrixXd> EiConstMatrixRef;
typedef Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<> > EiVectorRef;
typedef Eigen::Ref<const Eigen::VectorXd, 0, Eigen::InnerStride<> > EiConstVectorRef;

typedef Eigen::PartialPivLU<Eigen::MatrixXd> EiLU;
typedef Eigen::LLT<Eigen::MatrixXd> EiCholesky;

} /* namespace DP_GMM2 */

#endif /* DP_GMM2_MATRIXTYPESDPGMM_H_ */
