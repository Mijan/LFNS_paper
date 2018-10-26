//
// Created by jan on 12/10/18.
//

#ifndef LFNS_INITIALVALUEDATA_H
#define LFNS_INITIALVALUEDATA_H

#include <map>
#include "ParserData.h"

namespace models {
    class InitialValueData : public ParserData {
    public:

        InitialValueData(std::string initial_value_file);

        InitialValueData(const InitialValueData& rhs);

        virtual ~InitialValueData();

        void setInitialValues(std::string species, std::string initial_value);

        std::string getInitialValueForSpecies(std::string species);

        const std::vector<std::string> &getInitialStates() const;

        const std::vector<std::string> &getInitialValues() const;

        std::size_t getNumInitialValues() const;


    private:
        std::map<std::string, std::string> _initial_values_by_species;
        std::vector<std::string> _initial_states;
        std::vector<std::string> _initial_values;

    };

}


#endif //LFNS_INITIALVALUEDATA_H
