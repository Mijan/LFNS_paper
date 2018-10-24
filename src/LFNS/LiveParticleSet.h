//
// Created by jan on 05/10/18.
//

#ifndef LFNS_LIVEPARTICLESET_H
#define LFNS_LIVEPARTICLESET_H

#include "LFNSParticle.h"
#include "../base/EigenMatrices.h"
#include <vector>
#include <set>
#include <string>

namespace lfns {

    class LiveParticleSet {

    public:
        LiveParticleSet();

        int numberParticles();

        void push_back(std::vector<double> vector, double log_likelihood);

        const LFNSParticle removeLowestPartcile();

        double getLowestLikelihood();

        double getHighestLogLikelihood();

        double getLogAverageL();

        double getLogVariance();

        double getLogVariance(double log_average);

        std::multiset<LFNSParticle>::iterator begin();

        std::multiset<LFNSParticle>::iterator end();

        void writeToFile(std::string output_file_name, std::string suffix = "");

        void readFromFile(std::string input_file_name);

        base::EiMatrix toMatrix();

    private:
        std::multiset<LFNSParticle, lfns_particle_comp> _live_points;
    };

}


#endif //LFNS_LIVEPARTICLESET_H
