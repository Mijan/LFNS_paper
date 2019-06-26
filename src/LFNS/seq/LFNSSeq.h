//
// Created by jan on 23/10/18.
//

#ifndef LFNS_LFNSSEQ_H
#define LFNS_LFNSSEQ_H

#include "../LFNS.h"

namespace lfns {
    namespace seq {

        class LFNSSeq : public lfns::LFNS {
        public:
            LFNSSeq(LFNSSettings &settings, LogLikelihodEvalFct_ptr log_likelihood_evaluation);

            void runLFNS() override;

        protected:
            LogLikelihodEvalFct_ptr _log_likelihood_evaluation;
        };
    }
}


#endif //LFNS_LFNSSEQ_H
