//
// Created by jan on 05/10/18.
//

#ifndef LFNS_DEADPARTICLESET_H
#define LFNS_DEADPARTICLESET_H

#include "LFNSParticle.h"
#include <string>

namespace lfns {
    class DeadParticleSet {

    public:
        DeadParticleSet();

        void push_back(const LFNSParticle &particle);

        std::size_t size();

        const LFNSParticle &operator[](std::size_t i) const;

        std::vector<LFNSParticle>::iterator begin();

        std::vector<LFNSParticle>::iterator end();

        void writeToFile(std::string output_file_name, std::string suffix = "");

        void readFromFile(std::string input_file_name);

    private:
        std::vector<LFNSParticle> _dead_points;

    };
}


#endif //LFNS_DEADPARTICLESET_H
