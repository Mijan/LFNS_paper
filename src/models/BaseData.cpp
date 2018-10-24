//
// Created by jan on 12/10/18.
//

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include "BaseData.h"

namespace models {
    BaseData::BaseData(std::vector<std::string> parameter_names_, std::vector<std::string> species_names_)
            : parameter_names(parameter_names_), species_names(species_names_) {}


    BaseData::BaseData() : parameter_names(), species_names() {}

    BaseData::~BaseData() {}

    std::size_t BaseData::getNumParamters() const { return parameter_names.size(); }

    std::size_t BaseData::getNumSpecies() const { return species_names.size(); }

    bool BaseData::isSpeciesName(std::string name) const {
        return std::find(species_names.cbegin(), species_names.cend(), name.c_str()) != species_names.cend();
    }

    bool BaseData::isParamName(std::string name) const {
        return std::find(parameter_names.cbegin(), parameter_names.cend(), name.c_str()) != parameter_names.cend();
    }

    bool BaseData::isTimeName(std::string name) const { return name.compare(TIME_NAME) == 0; }

    std::size_t BaseData::getSpeciesIndex(std::string species_name) const {
        std::size_t param_index =
                std::find(species_names.cbegin(), species_names.cend(), species_name) - species_names.cbegin();
        if (param_index < species_names.size()) {
            return param_index;
        } else {
            std::ostringstream os;
            os << "Species name '" << species_name << "' not found!" << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    std::size_t BaseData::getParamIndex(std::string param_name) const {
        std::size_t param_index =
                std::find(parameter_names.cbegin(), parameter_names.cend(), param_name) - parameter_names.cbegin();
        if (param_index < parameter_names.size()) {
            return param_index;
        } else {
            std::ostringstream os;
            os << "Parameter name '" << param_name << "' not found!" << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    const std::vector<std::string> &BaseData::getParameterNames() const { return parameter_names; }

    const std::vector<std::string> &BaseData::getSpeciesNames() const { return species_names; }

    std::string BaseData::getTimeName() const { return TIME_NAME; }
}