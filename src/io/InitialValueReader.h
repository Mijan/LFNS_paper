//
// Created by jan on 12/10/18.
//

#ifndef LFNS_INITIALVALUEREADER_H
#define LFNS_INITIALVALUEREADER_H

#include "ParserReader.h"

namespace io {
    const std::string INITIAL_VALUES = "Initial Values:";

    class InitialValueReader : public ParserReader {
    public:
        InitialValueReader(std::string initial_value_file_name);

        virtual ~InitialValueReader();

        std::vector<std::string> readInitialStates();

        std::vector<std::string> readInitialEquations();

    };
}


#endif //LFNS_INITIALVALUEREADER_H
