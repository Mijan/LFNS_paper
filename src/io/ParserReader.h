//
// Created by jan on 12/10/18.
//

#ifndef LFNS_PARSERREADER_H
#define LFNS_PARSERREADER_H

#include "TxtFileReader.h"

namespace io {
    const std::string PARAMETER_CAPTION = "Parameters:";
    const std::string SPECIES_CAPTION = "Species:";
    const std::string RANDOM_NUMBER_CAPTION = "Random numbers:";
    const std::string NORMAL_CAPTION = "Normal";
    const std::string UNIFORM_CAPTION = "Uniform";
    const std::string UNIFORM_INT_CAPTION = "Uniform_Int";


    class ParserReader : public TxtFileReader {
    public:
        ParserReader(std::string parser_file_name);

        virtual ~ParserReader();

        std::vector<std::string> readParameterNames();

        std::vector<std::string> readSpeciesNames();

        std::vector<std::string> readNormalRdn();

        std::vector<std::string> readUniformRdn();

        std::vector<std::string> readUniformIntRdn();

        std::vector<std::pair<double, double>> readNormalParams();

        std::vector<std::pair<double, double>> readUniformParams();

        std::vector<std::pair<int, int>> readUniformIntParams();

    };
}


#endif //LFNS_PARSERREADER_H
