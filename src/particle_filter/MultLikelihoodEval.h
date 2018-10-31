//
// Created by jan on 31/10/18.
//

#ifndef LFNS_MULTLIKELIHOODEVAL_H
#define LFNS_MULTLIKELIHOODEVAL_H

#include <memory>
#include <vector>
#include <functional>

namespace particle_filter {
    using namespace std::placeholders;

    typedef std::function<double(const std::vector<double> &)> LogLikelihodEvalFct;
    typedef std::shared_ptr<LogLikelihodEvalFct> LogLikelihodEvalFct_ptr;

    class MultLikelihoodEval {
    public:
        MultLikelihoodEval() : _log_like_fun() {}

        ~MultLikelihoodEval() {}

        double compute_log_like(const std::vector<double> &theta) {
            double log_like = 0;
            for (LogLikelihodEvalFct_ptr fun : _log_like_fun) {
                log_like += (*fun)(theta);
            }
            return log_like;
        }

        void addLogLikeFun(LogLikelihodEvalFct_ptr log_like_fun) { _log_like_fun.push_back(log_like_fun); }

        LogLikelihodEvalFct_ptr getLogLikeFun() {
            return std::make_shared<LogLikelihodEvalFct>(std::bind(&MultLikelihoodEval::compute_log_like, this, _1));
        }

    private:
        std::vector<LogLikelihodEvalFct_ptr> _log_like_fun;
    };
}


#endif //LFNS_MULTLIKELIHOODEVAL_H
