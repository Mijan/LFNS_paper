//
// Created by jan on 15.12.16.
//

#include <iomanip>
#include "InitialValueProvider.h"
#include <algorithm>

namespace models {

    using namespace std::placeholders;

    InitialValueProvider::InitialValueProvider(base::RngPtr rng, InitialValueData data) : ParserBaseObject(rng, data),
                                                                                          _initialized(false),
                                                                                          _initial_state_equations(
                                                                                                  data.getNumInitialValues()),
                                                                                          _initial_time(),
                                                                                          _initial_data(data),
                                                                                          _output_state_mapping() {
        for (int i = 0; i < _initial_data.getNumInitialValues() - 1; i++) { _output_state_mapping[i] = i; }
    }

    InitialValueProvider::InitialValueProvider(InitialValueData data) : ParserBaseObject(data), _initialized(false),
                                                                        _initial_state_equations(
                                                                                data.getNumInitialValues()),
                                                                        _initial_time(), _initial_data(data),
                                                                        _output_state_mapping() {
        for (int i = 0; i < _initial_data.getNumInitialValues() - 1; i++) { _output_state_mapping[i] = i; }
    }

    InitialValueProvider::~InitialValueProvider() {}

    void InitialValueProvider::computeInitialState(std::vector<double> *state_0, double *t_0) {
        try {
            if (!_initialized) {
                std::stringstream os;
                os << "InitialValueProvider must be initialized before measurement can be computed!"
                   << std::endl;
                if (!_allPointerSet()) { printPointer(os); }
                throw std::runtime_error(os.str());
            }
            *t_0 = _initial_time.Eval();
            if (_perturbation_fct) {
                (*_perturbation_fct)(nullptr, *t_0);
            }
            _createRandomNumbers();
            for (std::size_t state_index = 0; state_index < state_0->size(); state_index++) {
                (*state_0)[_output_state_mapping[state_index]] = _initial_state_equations[state_index].Eval();
            }

        } catch (mu::Parser::exception_type &e) {
            std::ostringstream os;
            os << "Could not get initial states" << std::endl;
            os << "Message:  " << e.GetMsg() << "\n";
            os << "Formula:  " << e.GetExpr() << "\n";
            os << "Token:    " << e.GetToken() << "\n";
            os << "Position: " << e.GetPos() << "\n";
            os << "Errc:     " << e.GetCode() << "\n";
            throw std::runtime_error(os.str());
        }
    }


    InitialStateFct_ptr InitialValueProvider::getInitialStateFct() {
        return std::make_shared<InitialStateFct>(
                std::bind(&InitialValueProvider::computeInitialState, this, _1, _2));
    }


    void InitialValueProvider::printInfo(std::ostream &os) {
        os << std::endl;
        os << "\n----------   Initial Values   ---------- " << std::endl;
        ParserBaseObject::printInfo(os);
        try {
            std::size_t max_length = 0;
            for (const std::string &name : _initial_data.getInitialStates()) {
                if (max_length < name.size()) { max_length = name.size(); }
            }

            for (std::size_t i = 0; i < _initial_data.getNumInitialValues(); i++) {
                std::string state_str = "State " + _initial_data.getInitialStates()[i] + ":";
                os << std::setw(max_length + 7) << state_str << "\t" << _initial_state_equations[i].GetExpr()
                   << std::endl;
            }
            std::string time_str = "Time:";
            os << std::setw(max_length + 7) << time_str << "\t" << _initial_time.GetExpr() << std::endl;
            os << std::endl;
        } catch (mu::Parser::exception_type &e) {
            std::ostringstream os;
            os << "Could not define variable for Initial Value Provider" << std::endl;
            os << "Message:  " << e.GetMsg() << "\n";
            os << "Formula:  " << e.GetExpr() << "\n";
            os << "Token:    " << e.GetToken() << "\n";
            os << "Position: " << e.GetPos() << "\n";
            os << "Errc:     " << e.GetCode() << "\n";
            throw std::runtime_error(os.str());
        }
    }

    void InitialValueProvider::setOutputStateMapping(std::vector<std::string> state_order) {
        if (state_order.size() != _initial_data.getNumInitialValues()) {
            std::ostringstream os;
            os << "Failed to set the initial state order! For ordering, " << state_order.size()
               << " states provided, but "
               << _initial_state_equations.size() << " states needed!"
               << std::endl;
            throw std::runtime_error(os.str());
        }
        for (int i = 0; i < state_order.size(); i++) {
            const std::string &state = state_order[i];
            try {
                std::size_t index = _initial_data.getInitialStateIndex(state);
                _output_state_mapping[index] = i;
            } catch (const std::exception &e) {
                std::stringstream ss;
                ss << "Failed to set output state " << state << " for initial value provider. Provided ordering: ";
                for (std::string &state_name : state_order) { ss << state_name << ", "; }
                ss << std::endl;
                ss << "Internal state order: ";
                for (const std::string &state_name : _initial_data.getSpeciesNames()) { ss << state_name << ", "; }
                ss << std::endl;
                throw std::runtime_error(ss.str());
            }

        }
    }

    void InitialValueProvider::_initialize() {
        try {
            if (!_parameterPointerSet()) {
                std::stringstream os;
                os << "Tried to _initialize InitialValueProvider before all pointers have been set. The pointers are:"
                   << std::endl;
                printPointer(os);
                throw std::runtime_error(os.str());
            }

            for (int i = 0; i < _initial_data.getNumInitialValues(); i++) {
                std::string species_name = _initial_data.getInitialStates()[i];
                mu::Parser p;
                _initializeParser(p);
                p.SetExpr(_initial_data.getInitialValueForSpecies(species_name));
                _initial_state_equations[i] = p;
            }
            mu::Parser p;
            _initializeParser(p);
            p.SetExpr(_initial_data.getInitialTimeValue());
            _initial_time = p;

            _initialized = true;
        } catch (const std::exception &e) {
            std::stringstream os;
            os << "Failed to _initialize Initial Value Provider:" << std::endl;
            os << "\t" << e.what() << std::endl;
            throw std::runtime_error(os.str());
        }
    }


}