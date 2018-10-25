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

        std::vector<std::vector<double> > _f_normalized;
        std::vector<double> _f_log_scale;

        double E_t(int j);

        double E_t2(int j);

        std::vector<double> getDeltas(int m);

        double _computeLogZdVar(DeadParticleSet &dead_points, double log_zd);

        double _computeLogBi(int i);

        void _computeF(DeadParticleSet &dead_points);

        double _computeLogZlVar(LiveParticleSet &live_points, int m);

        double _computeLogZtotVar(double log_zd, double log_zd_var, double log_zl, double log_zl_var,
                                  LiveParticleSet &live_points, int m);
    };
};


#endif //LFNS_POSTERIORESTIMATOR_H
