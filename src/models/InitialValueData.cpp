//
// Created by jan on 12/10/18.
//

#include <iostream>
#include <sstream>
#include <algorithm>
#include "InitialValueData.h"
#include "../io/InitialValueReader.h"

namespace models {
    InitialValueData::InitialValueData(std::string initial_value_file) : ParserData(initial_value_file),
                                                                         _initial_values_by_species(),
                                                                         _initial_states(), _initial_values() {

        if (!species_names.empty()) {
            std::cerr
                    << "Initial values are assumed to be independent of the current state, thus states provided after 'Species' keyword will be ignored!"
                    << std::endl;
            species_names.clear();

        }
        io::InitialValueReader reader(initial_value_file);
        _initial_values = reader.readInitialEquations();
        _initial_states = reader.readInitialStates();


        if (std::find(_initial_states.begin(), _initial_states.end(), TIME_NAME) == _initial_states.end()) {
            std::cerr << "Warning: No initial time (with '_t') set! Assume _t = 0" << std::endl;
            _initial_states.push_back("_t");
            _initial_values.push_back("0");
        }

        for (std::size_t i = 0; i < _initial_states.size(); i++) {
            std::string species = _initial_states[i];
            _initial_values_by_species[species] = _initial_values[i];
        }
    }


    InitialValueData::~InitialValueData() {};

    void InitialValueData::setInitialValues(std::string species, std::string initial_value) {
        _initial_values_by_species[species] = initial_value;
        int index = std::find(_initial_states.begin(), _initial_states.end(), species) - _initial_states.begin();
        if (index >= _initial_states.size()) {
            _initial_states.push_back(species);
            _initial_values.push_back(initial_value);
        } else {
            _initial_values[index] = initial_value;
        }
    }

    std::string InitialValueData::getInitialValueForSpecies(std::string species) {
        if (_initial_values_by_species.count(species) == 0) {
            std::stringstream os;
            os << "Tried to get initial value for " << species << ", but no initial value provided!" << std::endl;
            throw std::runtime_error(os.str());
        }
        const std::string init_val = _initial_values_by_species[species];
        return init_val;
    }

    const std::vector<std::string> &InitialValueData::getInitialStates() const {
        return _initial_states;
    }

    const std::vector<std::string> &InitialValueData::getInitialValues() const {
        return _initial_values;
    }

    std::size_t InitialValueData::getNumInitialValues() const {
        return _initial_values.size();
    }
}
