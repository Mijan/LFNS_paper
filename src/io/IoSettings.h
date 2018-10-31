//
// Created by jan on 31/10/18.
//

#ifndef LFNS_IOSETTINGS_H
#define LFNS_IOSETTINGS_H

#include <string>
#include <iostream>

namespace io {
    class IoSettings {
    public:
        std::string config_file = "";
        std::string output_file = "";

        void print(std::ostream &stream) {
            stream << "Config file: " << config_file << std::endl;
            stream << "Output file: " << output_file << std::endl << std::endl;
        }
    };
}


#endif //LFNS_IOSETTINGS_H
