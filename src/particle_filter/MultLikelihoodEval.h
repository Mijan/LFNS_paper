//
// Created by jan on 31/10/18.
//

#ifndef LFNS_MULTLIKELIHOODEVAL_H
#define LFNS_MULTLIKELIHOODEVAL_H

#include <memory>
#include <vector>
#include <functional>
#include <float.h>

namespace particle_filter {
    using namespace std::placeholders;

    typedef std::function<double(const std::vector<double> &theta)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class MultLikelihoodEval {
    public:
        MultLikelihoodEval() : _log_like_fun(), _threshold_ptr(nullptr), _verbose(false), _stream(nullptr) {}

        ~MultLikelihoodEval() {}

        double compute_log_like(const std::vector<double> &theta) {
            double log_like = 0;
            int num_traj = 1;
            for (LogLikelihodEvalFct_ptr fun : _log_like_fun) {
                double log_like_single_traj = (*fun)(theta);
                if (_verbose) {
                    std::cout << " " << log_like_single_traj;
                }
                if (_stream) {
                    (*_stream) << " " << log_like_single_traj;
                }
                log_like += log_like_single_traj;
                if (_threshold_ptr && log_like < *_threshold_ptr) { return -DBL_MAX; }
                num_traj++;
            }
            if (_verbose) {
                std::cout << std::endl;
            }
            if (_stream) {
                (*_stream)<< std::endl;
            }
            return log_like;
        }

        void addLogLikeFun(LogLikelihodEvalFct_ptr log_like_fun) { _log_like_fun.push_back(log_like_fun); }

        LogLikelihodEvalFct_ptr getLogLikeFun() {
            return std::make_shared<LogLikelihodEvalFct>(
                    std::bind(&MultLikelihoodEval::compute_log_like, this, std::placeholders::_1));
        }

        void setThresholdPtr(double *threshold_ptr) { _threshold_ptr = threshold_ptr; }

        void setVerbose(bool verbose) { _verbose = verbose; }

        void setStream(std::ostream * stream) { _stream = stream; }

    private:
        std::vector<LogLikelihodEvalFct_ptr> _log_like_fun;

        double *_threshold_ptr;
        bool _verbose;
        std::ostream * _stream;
    };
}


#endif //LFNS_MULTLIKELIHOODEVAL_H
