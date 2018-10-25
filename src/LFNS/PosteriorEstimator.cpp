//
// Created by jan on 05/10/18.
//

#include <cmath>
#include <cfloat>
#include "PosteriorEstimator.h"
#include "../base/MathUtils.h"
#include "../base/IoUtils.h"

namespace lfns {


    PosteriorEstimator::PosteriorEstimator(int N, int r) : _N(N), _r(r), _E_t(), _E_t2(), _f_normalized(),
                                                           _f_log_scale() {}

    PosteriorQuantitites
    PosteriorEstimator::estimatePosteriorQuantities(LiveParticleSet &live_points, DeadParticleSet &dead_points) {

        _E_t = std::vector<double>(_r + 1, 1.0);
        _E_t2 = std::vector<double>(_r + 1, 1.0);
        for (int j = 1; j <= _r; j++) {
            _E_t[j] = E_t(j);
            _E_t2[j] = E_t2(j);
        }

        _computeF(dead_points);

        PosteriorQuantitites post_quant;
        int m = dead_points.size() / _r;

        std::vector<double> log_delta = getDeltas(m);
        post_quant.log_delta = log_delta;
        double log_final_vol = m * std::log(_E_t[_r]);
        post_quant.log_final_vol = log_final_vol;

        std::vector<double> log_zds = std::vector<double>(m * _r, 0.0);
        double max_log = -DBL_MAX;
        for (int i = 0; i < m * _r; i++) {
            log_zds[i] = dead_points[i].getLogLikelihood() + log_delta[i];
            max_log = max_log > log_zds[i] ? max_log : log_zds[i];
        }
        double log_zd = base::MathUtils::sumOfLog(log_zds, max_log);

        double log_zl = live_points.getLogAverageL() + log_final_vol;
        post_quant.log_zd = log_zd;
        post_quant.log_zl = log_zl;

        post_quant.log_ztot = base::MathUtils::sumOfLog(log_zd, log_zl);


        double log_zd_var = _computeLogZdVar(dead_points, log_zd);
        post_quant.log_zd_var = log_zd_var;

        double log_zl_var = _computeLogZlVar(live_points, m);
        post_quant.log_zl_var = log_zl_var;

        double log_ztot_var = _computeLogZtotVar(log_zd, log_zd_var, log_zl, log_zl_var, live_points, m);
        post_quant.log_ztot_var = log_ztot_var;

        post_quant.log_live_average = live_points.getLogAverageL();
        post_quant.log_live_variance = live_points.getLogVariance();
        post_quant.log_Ex2 = m * std::log(_E_t2[_r]);

        double log_1 = 0.5 * post_quant.log_ztot_var;
        double log_2 = live_points.getLogVariance() - std::log(live_points.numberParticles()) + m * std::log(_E_t2[_r]);
        double log_3 = 0.5 * base::MathUtils::diffOfLog(post_quant.log_ztot_var, log_2);
        post_quant.log_max_std_improvement = base::MathUtils::diffOfLog(log_1, log_3) - post_quant.log_ztot;


        post_quant.log_max_live = live_points.getHighestLogLikelihood();
        return post_quant;

    }

    std::vector<double> PosteriorEstimator::getDeltas(int m) {
        std::vector<double> log_x = std::vector<double>(_r * m, 0.0);

        double log_tr = log(E_t(_r));

        for (int j = 0; j < _r; j++) {
            log_x[j] = log(E_t(j + 1));
            for (int k = 1; k < m; k++) {
                log_x[k * _r + j] = log_x[j] + k * log_tr;
            }
        }

        std::vector<double> log_delta = std::vector<double>(_r * m, 0.0);
        log_delta[0] = log(1 - exp(log_x[0]));
        for (int j = 1; j < log_delta.size(); j++) {
            log_delta[j] = log(exp(log_x[j - 1]) - exp(log_x[j]));
        }
        return log_delta;
    }

    double PosteriorEstimator::E_t(int j) { return (_N - j + 1) / (double) (_N + 1); }

    double PosteriorEstimator::E_t2(int j) { return (_N - j + 1) * (_N - j + 2) / (double) ((_N + 1) * (_N + 2)); }

    double PosteriorEstimator::_computeLogZdVar(DeadParticleSet &dead_points, double log_zd) {
        double E_zd_2 = 0;
        int m = dead_points.size() / _r;

        std::vector<double> log_B = std::vector<double>(m, 0.0);
        double max_log_B = -DBL_MAX;
        for (int i = 0; i < m; i++) {
            log_B[i] = _computeLogBi(i + 1);
            max_log_B = max_log_B > log_B[i] ? max_log_B : log_B[i];
        }
        E_zd_2 = base::MathUtils::sumOfLog(log_B, max_log_B);

        return base::MathUtils::diffOfLog(E_zd_2, 2 * log_zd);
    }

    double PosteriorEstimator::_computeLogBi(int i) {
        double log_sum_2 = 0;
        double log_sum_2_1 = 0;

        for (int j = 0; j <= _r; j++) {
            log_sum_2_1 += std::pow(_f_normalized[i - 1][j], 2) * _E_t2[j];
        }
        log_sum_2_1 = std::log(log_sum_2_1) + 2 * _f_log_scale[i - 1];

        double max_tmp_2 = -DBL_MAX;
        std::vector<double> log_sums_2_2 = std::vector<double>(_r, -DBL_MAX);
        for (int j = 1; j <= _r; j++) {
            double log_fac;
            if (j < _r) {
                log_fac = std::log(2) + std::log(_f_normalized[i - 1][j] * _E_t[j]) + _f_log_scale[i - 1];
            } else {
                log_fac = std::log(2) + std::log(-_f_normalized[i - 1][j] * _E_t[j]) + _f_log_scale[i - 1];
            }

            double log_sum_2_2_1 = 0;
            for (int k = 0; k < j; k++) {
                log_sum_2_2_1 += _f_normalized[i - 1][k] * _E_t[k];
            }
            log_sum_2_2_1 = std::log(log_sum_2_2_1) + _f_log_scale[i - 1];

            log_sums_2_2[j-1] = log_fac + log_sum_2_2_1;
            max_tmp_2 = max_tmp_2 > log_sums_2_2[j-1] ? max_tmp_2 : log_sums_2_2[j-1];
        }
        double log_sum_2_2 = 0;
        for (int j = 0; j < _r-1; j++) {
            log_sum_2_2 += exp(log_sums_2_2[j] - max_tmp_2);
        }

        log_sum_2_2 = log_sum_2_2 - exp(log_sums_2_2[_r-1] - max_tmp_2);
        int sgn = log_sum_2_2 < 0 ? -1 : 1;

        log_sum_2_2 = std::log(sgn * log_sum_2_2) + max_tmp_2;

        double max_log = std::max<double>(log_sum_2_2, log_sum_2_1);
        log_sum_2 = std::log(sgn * exp(log_sum_2_2 - max_log) + exp(log_sum_2_1 - max_log)) + max_log;
        log_sum_2 = log_sum_2 + (i - 1) * std::log(_E_t2[_r]);


        double log_bm;
        if (i > 1) {
            double log_fac_1 = 0;
            for (int j = 0; j <= _r; j++) {
                log_fac_1 += _f_normalized[i - 1][j] * _E_t[j];
            }
            log_fac_1 = std::log(log_fac_1) + _f_log_scale[i - 1];


            std::vector<double> log_facs_2 = std::vector<double>(i - 1, 0.0);
            double max_tmp = -DBL_MAX;
            for (int l = 0; l < i - 1; l++) {
                double log_fac_2_1 = l * std::log(_E_t2[_r]);
                double log_fac_2_2 = 0;
                double tmp_fac = std::pow(_E_t[_r], (i - 1) - l);
                for (int j = 0; j < _r; j++) {
                    log_fac_2_2 += _f_normalized[l][j] * _E_t[j] * tmp_fac;
                }
                log_fac_2_2 += _f_normalized[l][_r] * _E_t2[_r] * std::pow(_E_t[_r], (i - 1) - l - 1);
                log_fac_2_2 = std::log(log_fac_2_2) + _f_log_scale[l];
                log_facs_2[l] = log_fac_2_1 + log_fac_2_2;
                max_tmp = max_tmp > log_facs_2[l] ? max_tmp : log_facs_2[l];
            }
            double log_fac_2 = base::MathUtils::sumOfLog(log_facs_2, max_tmp);

            log_bm = log(2) + log_fac_1 + log_fac_2;


            log_bm = base::MathUtils::sumOfLog(log_bm, log_sum_2);
        } else {
            log_bm = log_sum_2;
        }
        return log_bm;
    }


    void PosteriorEstimator::_computeF(DeadParticleSet &dead_points) {

        int m = dead_points.size() / _r;

        _f_normalized.resize(m);
        _f_log_scale.resize(m);

        for (int i = 0; i < m; i++) {
            std::vector<double> f(_r + 1, 0.0);
            double max_log = dead_points[i * _r + (_r - 1)].getLogLikelihood();
            _f_log_scale[i] = max_log;
            for (int j = 0; j <= _r; j++) {
                if (j == 0) { f[j] = std::exp(dead_points[i * _r].getLogLikelihood() - max_log); }
                if (j > 0 && j < _r) {
                    f[j] = std::exp(dead_points[i * _r + j].getLogLikelihood() - max_log) -
                           std::exp(dead_points[i * _r + j - 1].getLogLikelihood() - max_log);
                }
                if (j == _r) { f[j] = -std::exp(dead_points[i * _r + (_r - 1)].getLogLikelihood() - max_log); }

            }
            _f_normalized[i] = f;
        }
    }

    double PosteriorEstimator::_computeLogZlVar(LiveParticleSet &live_points, int m) {
        double log_xm = m * std::log(_E_t[_r]);
        double log_xm2 = m * std::log(_E_t2[_r]);
        double log_average_l = live_points.getLogAverageL();

        double lvar = live_points.getLogVariance(log_average_l);
        double n = std::log(live_points.numberParticles());
//        double log_var_l =
//                2 * log_xm + live_points.getLogVariance(log_average_l) - std::log(live_points.numberParticles());
        double log_var_l = live_points.getLogVariance(log_average_l) - std::log(live_points.numberParticles());
        double log_var_xm = std::log(std::exp(log_xm2) - exp(2 * log_xm));

        double sum_1 = log_var_xm + log_var_l;
        double sum_2 = log_var_xm + 2 * log_average_l;
        double sum_3 = 2 * log_xm + log_var_l;
        std::vector<double> sum_elements = {sum_1, sum_2, sum_3};
        return base::MathUtils::sumOfLog(sum_elements);

    }

    double PosteriorEstimator::_computeLogZtotVar(double log_zd, double log_zd_var, double log_zl, double log_zl_var,
                                                  LiveParticleSet &live_points, int m) {

        double sum_1 = log_zd_var;
        double sum_2 = log_zl_var;

        double e_tmp = 0;
        std::vector<double> log_facs = std::vector<double>(m, 0.0);
        for (int i = 1; i <= m; i++) {
            double log_fac_2_1 = (i - 1) * log(_E_t2[_r]);

            double log_fac_2_2 = 0;
            for (int j = 0; j < _r; j++) {
                log_fac_2_2 += _f_normalized[i - 1][j] * _E_t[j] * std::pow(_E_t[_r], m - i + 1);
            }
            log_fac_2_2 += _f_normalized[i - 1][_r] * _E_t2[_r] * std::pow(_E_t[_r], m - i);
            log_fac_2_2 = log(log_fac_2_2) + _f_log_scale[i - 1];
            log_facs[i - 1] = log_fac_2_1 + log_fac_2_2;
        }
        e_tmp = base::MathUtils::sumOfLog(log_facs);

        double sum_3_1 = live_points.getLogAverageL() + e_tmp;

        double sum_3_2 = log_zl + log_zd;
        double max_log = std::max<double>(sum_3_1, sum_3_2);

        int sign_3 = exp(sum_3_1 - max_log) - exp(sum_3_2 - max_log) < 0 ? -1 : 1;
        double sum_3 = log(2) + (log(abs(exp(sum_3_1 - max_log) - exp(sum_3_2 - max_log))) + max_log);

        std::vector<double> sums = {sum_1, sum_2, sum_3};
        double max_log_2 = *(std::max_element(sums.begin(), sums.end()));
        double log_ztot_var =
                std::log(exp(sum_1 - max_log_2) + exp(sum_2 - max_log_2) + sign_3 * exp(sum_3 - max_log_2)) + max_log_2;
        return log_ztot_var;
    }

    void PosteriorEstimator::writeToFile(std::string output_file_name, LiveParticleSet &live_points,
                                         DeadParticleSet &dead_points, PosteriorQuantitites &post_quant) {

        int m = dead_points.size() / _N;

        std::string posterior_file_name = base::IoUtils::appendToFileName(output_file_name, "posterior");

        std::ofstream posterior_file(posterior_file_name.c_str());
        if (!posterior_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << posterior_file_name.c_str()
               << " for writing posterior. Posterior could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::vector<LFNSParticle>::iterator it = dead_points.begin(); it != dead_points.end(); it++) {
            const std::vector<double> &theta = it->getTheta();
            for (int j = 0; j < theta.size(); j++) {
                double part = theta[j];
                posterior_file << part << " ";
            }
            posterior_file << std::endl;
        }

        for (std::set<LFNSParticle>::iterator it = live_points.begin(); it != live_points.end(); it++) {
            const std::vector<double> &theta = it->getTheta();
            for (int j = 0; j < theta.size(); j++) {
                double part = theta[j];
                posterior_file << part << " ";
            }
            posterior_file << std::endl;
        }
        posterior_file.close();

        std::string likelihood_file_name = base::IoUtils::appendToFileName(output_file_name,
                                                                           "posterior_log_likelihoods");
        std::ofstream likelihood_file(likelihood_file_name.c_str());
        if (!likelihood_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << likelihood_file_name.c_str()
               << " for writing posterior log likelihoods. Posterior log likelihoods could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        for (std::vector<LFNSParticle>::iterator it = dead_points.begin(); it != dead_points.end(); it++) {
            double log_like = it->getLogLikelihood();
            likelihood_file << log_like << std::endl;
        }

        for (std::set<LFNSParticle>::iterator it = live_points.begin(); it != live_points.end(); it++) {
            double log_like = it->getLogLikelihood();
            likelihood_file << log_like << std::endl;
        }
        likelihood_file.close();

        std::string weight_file_name = base::IoUtils::appendToFileName(output_file_name, "posterior_log_weights");
        std::ofstream weight_file(weight_file_name.c_str());
        if (!weight_file.is_open()) {
            std::stringstream os;
            os << "error opening file " << weight_file_name.c_str()
               << " for writing posterior weights. Posterior weights could not be saved!!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        std::vector<double> log_deltas = post_quant.log_delta;
        int i = 0;
        for (std::vector<LFNSParticle>::iterator it = dead_points.begin(); it != dead_points.end(); it++) {
            double log_weight = it->getLogLikelihood() + log_deltas[i++];
            weight_file << log_weight << std::endl;
        }


        double log_weight_factor_live_points = post_quant.log_final_vol - std::log(_N);
        for (std::set<LFNSParticle>::iterator it = live_points.begin(); it != live_points.end(); it++) {
            double log_weight = it->getLogLikelihood() + log_weight_factor_live_points;
            weight_file << log_weight << std::endl;
        }
        weight_file.close();


        std::cout << "Posterior written into " << posterior_file_name << std::endl;
    }
}
