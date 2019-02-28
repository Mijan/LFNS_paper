//
// Created by jan on 23/10/18.
//

#ifndef LFNS_MPITAGS_H
#define LFNS_MPITAGS_H

#include <boost/mpi.hpp>

namespace bmpi = boost::mpi;

namespace lfns {
    namespace mpi {

        enum MPI_INSTRUCTION {
            DIETAG,
            LIKELIHOOD_RECOMPU,
            UPDATE_EPSILON,
            UPDATE_SAMPLER,
            SAMPLER_SIZE,
            SAMPLE_PRIOR,
            SAMPLE_CONSTR_PRIOR,
            SAMPLER,
            EPSILON,
            PARTICLE,
            CLOCKS_SAMPLING,
            INSTRUCTION,
            PARTICLE_ACCEPTED,
            PARTICLE_REJECTED,
            STOP_SIMULATION,
            PREPARE_STOPPING,
            PARTICLE_SECONDS
        };

    } /* namespace lfns */
} /* namespace mpi */
#endif //LFNS_MPITAGS_H
