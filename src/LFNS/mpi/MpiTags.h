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
            EPSILON,
            PARTICLE,
            INSTRUCTION,
            STOP_SIMULATION,
            PREPARE_STOPPING,
            PARTICLE_SECONDS
        };

    } /* namespace lfns */
} /* namespace mpi */
#endif //LFNS_MPITAGS_H
