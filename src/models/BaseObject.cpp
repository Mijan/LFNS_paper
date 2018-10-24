//
// Created by jan on 12/10/18.
//

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "BaseObject.h"

namespace models {
    BaseObject::BaseObject(BaseData data) : _base_data(data),
                                            _parameter_ptrs(data.getNumParamters(), nullptr),
                                            _state_ptrs(data.getNumSpecies(), nullptr), _time_ptr(nullptr) {}

    BaseObject::BaseObject(std::vector<std::string> parameter_names, std::vector<std::string> species_names)
            : _base_data(parameter_names, species_names), _parameter_ptrs(0, nullptr),
              _state_ptrs(0, nullptr), _time_ptr(nullptr) {
        _parameter_ptrs.resize(_base_data.getNumParamters());
        _state_ptrs.resize(_base_data.getNumSpecies());
    }

    void BaseObject::setPointer(double *ptr, std::string name) {
        if (_base_data.isSpeciesName(name)) {
            _state_ptrs[_base_data.getSpeciesIndex(name)] = ptr;
        } else if (_base_data.isParamName(name)) {
            _parameter_ptrs[_base_data.getParamIndex(name)] = ptr;
        } else if (_base_data.isTimeName(name)) {
            _time_ptr = ptr;
        }
    }


    void BaseObject::setPointers(std::vector<double *> ptrs, std::vector<std::string> names) {
        if (ptrs.size() != names.size()) {
            std::stringstream os;
            os << "Failed to set pointers! " << ptrs.size() << " pointers provided, but " << names.size()
               << " names provieded!" << std::endl;
            throw std::runtime_error(os.str());
        }
        for (std::size_t i = 0; i < ptrs.size(); i++) {
            setPointer(ptrs[i], names[i]);
        }
    }

    bool BaseObject::_allPointerSet() const {
        if (_parameterPointerSet() && _statePointerSet() && _timePointerSet()) {
            return true;
        } else {
            return false;
        }
    }

    bool BaseObject::_parameterPointerSet() const {
        for (double *ptr : _parameter_ptrs) {
            if (!ptr) { return false; }
        }
        return true;
    }

    bool BaseObject::_statePointerSet() const {
        for (double *ptr : _state_ptrs) {
            if (!ptr) { return false; }
        }
        return true;
    }


    bool BaseObject::_timePointerSet() const {
        if (!_time_ptr) {
            return false;
        }
        return true;
    }

    void BaseObject::printPointer(std::ostream &os) const {
        for (std::size_t parameter_index = 0; parameter_index < _base_data.getNumParamters(); parameter_index++) {
            os << _base_data.getParameterNames()[parameter_index] << ":\t" << _parameter_ptrs[parameter_index]
               << std::endl;
        }

        os << std::endl;
        for (std::size_t species_index = 0; species_index < _base_data.getNumSpecies(); species_index++) {
            os << _base_data.getSpeciesNames()[species_index] << ":\t" << _state_ptrs[species_index] << std::endl;
        }

        os << _base_data.getTimeName() << ":\t" << _time_ptr << std::endl;
        std::cout << std::endl;
    }


    double *BaseObject::parameterPtr(std::size_t i) { return _parameter_ptrs[i]; }

    double *BaseObject::statePtr(std::size_t i) { return _state_ptrs[i]; }

    double *BaseObject::timePtr() { return _time_ptr; }

    const std::vector<std::string> &BaseObject::getSpeciesNames() const { return _base_data.getSpeciesNames(); }

    const int BaseObject::getNumSpecies() const { return _base_data.getNumSpecies(); }

    const std::vector<std::string> &BaseObject::getParameterNames() const { return _base_data.getParameterNames(); }

    const int BaseObject::getNumParameters() const { return _base_data.getNumParamters(); }

    void BaseObject::printInfo(std::ostream &os) const {
        if (!_base_data.getParameterNames().empty()) {
            os << "Parameters: ";
            for (const std::string &param_name : _base_data.getParameterNames()) {
                os << param_name << " ";
            }
        }

        if (!_base_data.getSpeciesNames().empty()) {
            os << "\nSpecies: ";
            for (const std::string &param_name : _base_data.getSpeciesNames()) {
                os << param_name << " ";
            }
        }
    }

    BaseData &BaseObject::getBaseData() { return _base_data; }

}