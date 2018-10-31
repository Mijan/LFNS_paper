//
// Created by jan on 31/10/18.
//

#ifndef LFNS_PARTICLEFILTERSETTINGS_H
#define LFNS_PARTICLEFILTERSETTINGS_H


#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace particle_filter {
    class ParticleFilterSettings {
    public:
        int num_used_trajectories = 1e5;
        std::map<std::string, std::string> data_files;
        std::map<std::string, std::string> time_files;
        std::vector<std::string> experiments_for_particle_filter;

        bool use_premature_cancelation = false;
        int H = 200;

        void print(std::ostream &stream) {


            stream << "H:\t" << H << std::endl;
            stream << "Data used:" << std::endl;
            for (std::string experiment: experiments_for_particle_filter) {
                stream << "Experiment:\t" << experiment << "\t\tData file: " << data_files[experiment]
                       << "\ttimes file: "
                       << time_files[experiment] << std::endl;
            }
            stream << "Nnumber of trajectories used for each experiment: " << num_used_trajectories << std::endl;
            stream << "\n\n" << std::endl;
        }

    };
}


#endif //LFNS_PARTICLEFILTERSETTINGS_H
