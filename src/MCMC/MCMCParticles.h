//
// Created by jan on 25/02/19.
//

#ifndef LFNS_MCMCPARTICLES_H
#define LFNS_MCMCPARTICLES_H

#include <vector>
#include <string>

namespace mcmc {
    class MCMCParticle {
    public:
        MCMCParticle(std::vector<double> &particle, double log_likelihood) : particle(particle),
                                                                             log_likelihood(log_likelihood) {}

        ~MCMCParticle() {}

        std::vector<double> particle;
        double log_likelihood;
    };

    class MCMCParticles {
    public:
        MCMCParticles(int num_particles = 0);

        virtual ~MCMCParticles();

        void reserve(std::size_t num_particles);

        void push_back(MCMCParticle particle);

        std::size_t size();

        MCMCParticle &back();


        void writeToFile(std::string output_file_name, std::string suffix = "");

        void readFromFile(std::string input_file_name);

    private:
        std::vector<MCMCParticle> _particles;

    };
}


#endif //LFNS_MCMCPARTICLES_H
