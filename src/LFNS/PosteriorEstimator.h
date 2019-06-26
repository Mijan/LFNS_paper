//
// Created by jan on 05/10/18.
//

#ifndef LFNS_POSTERIORESTIMATOR_H
#define LFNS_POSTERIORESTIMATOR_H


#include <string>
#include "LiveParticleSet.h"
#include "DeadParticleSet.h"

namespace lfns {

    struct PosteriorQuantitites {
        double log_zd;
        double log_zl;
        double log_ztot;
        double log_zd_var;
        double log_zl_var;
        double log_ztot_var;
        double log_final_vol;
        double log_Ex2;
        double log_live_average;
        double log_live_variance;
        double log_max_live;
        double log_max_std_improvement;
        double log_min_var;

        std::vector<double> log_delta;
    };

    class PosteriorEstimator {

    public:
        PosteriorEstimator(int N, int r);

        PosteriorQuantitites estimatePosteriorQuantities(LiveParticleSet &live_points, DeadParticleSet &dead_points);


        void writeToFile(std::string output_file_name, LiveParticleSet &live_points, DeadParticleSet &dead_points,
                         PosteriorQuantitites &post_quant);

    private:
        int _N;
        int _r;
        std::vector<double> _E_t;
        std::vector<double> _E_t2;

        std::vector<std::vector<double> > _e_normalized;
        std::vector<double> _e_log_scale;

        std::vector<std::vector<double> > _epsilons_normalized;
        std::vector<double> _epsilons_log_scale;

        double E_t(int j);

        double E_t2(int j);

        std::vector<double> getDeltas(int m);

        double _computeLogZdVar(DeadParticleSet &dead_points, double log_zd);

        double _computeLogBi(int i);

        void _computeE(DeadParticleSet &dead_points);

        double _computeLogZlVar(LiveParticleSet &live_points, int m);

        double _computeLogZtotVar(double log_zd, double log_zd_var, double log_zl, double log_zl_var,
                                  LiveParticleSet &live_points, int m);

        double _computeLogZdVar_2(DeadParticleSet &dead_points, double zd);

        void _computeLogZdVarPartFirst_2(DeadParticleSet &dead_points, int l, double *log_result, int *sing_result);

        void _computeLogZdVarPartSecond_2(DeadParticleSet &dead_points, int l, double *log_result, int *sing_result);

        double _computeLogZdVar_3(DeadParticleSet &dead_points, double zd);

        double _cov_t(int i, int j);

        double _cov_diff_t(int i, int j);


        double _varX(int i, int j);

        void
        _computeCovTerm(DeadParticleSet &dead_points, LiveParticleSet &live_points, double *log_cov, int *sign_cov);

    };

    typedef std::shared_ptr<PosteriorEstimator> PosteriorEstimator_ptr;
};


#endif //LFNS_POSTERIORESTIMATOR_H
