//
// Created by jan on 08/10/18.
//

#ifndef LFNS_EIGENMATRICES_H
#define LFNS_EIGENMATRICES_H

#include <Eigen/Dense>
#include <memory>

namespace base {
    typedef Eigen::VectorXd EiVector;
    typedef Eigen::VectorXcd EiVectorC;
    typedef Eigen::MatrixXd EiMatrix;
    typedef Eigen::MatrixXcd EiMatrixC;
    typedef Eigen::RowVectorXd EiRowVector;
    typedef Eigen::Block<Eigen::MatrixXd, 1, Eigen::Dynamic> EiVectorRow;
    typedef Eigen::Block<Eigen::MatrixXd, Eigen::Dynamic, 1> EiVectorCol;
    typedef Eigen::Block<const Eigen::MatrixXd, 1, Eigen::Dynamic> EiConstSubVectorRow;
    typedef Eigen::Block<const Eigen::MatrixXd, Eigen::Dynamic, 1> EiConstSubVectorCol;

    typedef Eigen::Block <Eigen::MatrixXd> EiSubMatrix;
    typedef Eigen::Ref <Eigen::MatrixXd> EiMatrixRef;
    typedef Eigen::Ref<const Eigen::MatrixXd> EiConstMatrixRef;
    typedef Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<> > EiVectorRef;
    typedef Eigen::Ref<const Eigen::VectorXd, 0, Eigen::InnerStride<> > EiConstVectorRef;
    typedef Eigen::PartialPivLU <Eigen::MatrixXd> EiLU;
    typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic> EiDiag;

    typedef Eigen::LLT <Eigen::MatrixXd> EiCholesky;
}
#endif //LFNS_EIGENMATRICES_H

