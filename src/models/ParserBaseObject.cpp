//
// Created by jan on 12/10/18.
///

#include "ParserBaseObject.h"
#include "../base/MathUtils.h"

namespace models {

    ParserBaseObject::ParserBaseObject(base::RngPtr rng, ParserData parser_data) : BaseObject(parser_data), _rng(rng),
                                                                                   _parser_data(parser_data),
                                                                                   _normal_numbers(
                                                                                           parser_data.getNumNormalNumbers()),
                                                                                   _uniform_numbers(
                                                                                           parser_data.getNumUniformNumbers()),
                                                                                   _uniform_int_numbers(
                                                                                           parser_data.getNumUniformIntNumbers()),
                                                                                   _normal_dists(), _uniform_dists(),
                                                                                   _uniform_int_dists(),
                                                                                   _parameter(
                                                                                           parser_data.getNumParamters()),
                                                                                   _state(parser_data.getNumSpecies()),
                                                                                   _time(0),
                                                                                   _input_parameter_ext_by_int(),
                                                                                   _input_state_ext_by_int(),
                                                                                   _num_input_param(
                                                                                           parser_data.getNumParamters()),
                                                                                   _all_inputs_defined(false),
                                                                                   _fixed_parameters() {

        const std::vector<std::string> &normal_random_num = _parser_data.getNormalRandomNbrsName();
        const std::vector<std::pair<double, double> > &normal_params = _parser_data.getNormalRandomParams();
        for (std::size_t normal_num = 0; normal_num < normal_random_num.size(); normal_num++) {
            std::pair<double, double> params = normal_params[normal_num];
            if (params.second < 0) {
                std::stringstream os;
                os << "Standard deviation for normal random number  " << normal_random_num[normal_num]
                   << " is infeasible: std "
                   << params.second << std::endl;
                throw std::runtime_error(os.str());
            }
            _normal_dists.push_back(base::NormalDistribution(params.first, params.second));
        }

        const std::vector<std::string> &uniform_random_num = _parser_data.getUniformRandomNbrsName();
        const std::vector<std::pair<double, double> > &uniform_params = _parser_data.getUniformRandomParams();
        for (std::size_t uniform_num = 0; uniform_num < uniform_random_num.size(); uniform_num++) {
            std::pair<double, double> params = uniform_params[uniform_num];
            if (params.first >= params.second) {
                std::stringstream os;
                os << "Bounds for uniform random number " << uniform_random_num[uniform_num] << " are infeasible: min "
                   << params.first << ", max " << params.second << std::endl;
                throw std::runtime_error(os.str());
            }
            _uniform_dists.push_back(base::UniformRealDistribution(params.first, params.second));
        }

        const std::vector<std::string> &uniform_int_random_num = _parser_data.getUniformIntRandomNbrsName();
        const std::vector<std::pair<int, int> > &uniform_int_params = _parser_data.getUniformIntRandomParams();
        for (std::size_t uniform_num = 0; uniform_num < uniform_int_random_num.size(); uniform_num++) {
            std::pair<double, double> params = uniform_int_params[uniform_num];
            if (params.first >= params.second) {
                std::stringstream os;
                os << "Bounds for uniform int random number " << uniform_int_random_num[uniform_num]
                   << " are infeasible: min "
                   << params.first << ", max " << params.second << std::endl;
                throw std::runtime_error(os.str());
            }
            _uniform_int_dists.push_back(base::UniformIntDistribution(params.first, params.second));
        }

        for (int i = 0; i < _base_data.getNumSpecies(); i++) {
            _state_ptrs[i] = &_state[i];
            _input_state_ext_by_int[i] = i;
        }
        for (int i = 0; i < _base_data.getNumParamters(); i++) {
            _parameter_ptrs[i] = &_parameter[i];
            _input_parameter_ext_by_int[i] = i;
        }
        _time_ptr = &_time;
    }

    ParserBaseObject::ParserBaseObject(ParserData parser_data) : BaseObject(parser_data), _rng(nullptr),
                                                                 _parser_data(parser_data),
                                                                 _normal_numbers(
                                                                         parser_data.getNumNormalNumbers()),
                                                                 _uniform_numbers(
                                                                         parser_data.getNumUniformNumbers()),
                                                                 _uniform_int_numbers(
                                                                         parser_data.getNumUniformIntNumbers()),
                                                                 _normal_dists(), _uniform_dists(),
                                                                 _uniform_int_dists(),
                                                                 _parameter(
                                                                         parser_data.getNumParamters()),
                                                                 _state(parser_data.getNumSpecies()),
                                                                 _time(0),
                                                                 _input_parameter_ext_by_int(),
                                                                 _input_state_ext_by_int(),
                                                                 _num_input_param(
                                                                         parser_data.getNumParamters()),
                                                                 _all_inputs_defined(false),
                                                                 _fixed_parameters() {

        if (parser_data.getNumNormalNumbers() > 0 || parser_data.getNumUniformNumbers() > 0 ||
            parser_data.getNumUniformIntNumbers() > 0) {
            throw std::runtime_error(
                    "Tried to _initialize ParserBaseObject without Random number generator but with random numbers!");
        }


        for (int i = 0; i < _base_data.getNumSpecies(); i++) {
            _state_ptrs[i] = &_state[i];
            _input_state_ext_by_int[i] = i;
        }
        for (int i = 0; i < _base_data.getNumParamters(); i++) {
            _parameter_ptrs[i] = &_parameter[i];
            _input_parameter_ext_by_int[i] = i;
        }
        _time_ptr = &_time;
    }


    void ParserBaseObject::printInfo(std::ostream &os) const {
        BaseObject::printInfo(os);

        std::vector<std::string> normal_names = _parser_data.getNormalRandomNbrsName();
        std::vector<std::pair<double, double> > normal_params = _parser_data.getNormalRandomParams();
        if (!normal_names.empty()) {
            os << "\n\nNormal numbers:" << std::endl;
            for (std::size_t i = 0; i < normal_names.size(); i++) {
                os << normal_names[i] << ":\tmean: " << normal_params[i].first << "\tstd: " << normal_params[i].second
                   << std::endl;
            }
        }

        std::vector<std::string> uniform_names = _parser_data.getUniformRandomNbrsName();
        std::vector<std::pair<double, double> > uniform_params = _parser_data.getUniformRandomParams();
        if (!uniform_names.empty()) {
            os << "\nUniform numbers:" << std::endl;
            for (std::size_t i = 0; i < uniform_names.size(); i++) {
                os << uniform_names[i] << ":\tmin: " << uniform_params[i].first << "\tmax: " << uniform_params[i].second
                   << std::endl;
            }
        }

        std::vector<std::string> uniform_int_names = _parser_data.getUniformIntRandomNbrsName();
        std::vector<std::pair<int, int> > uniform_int_params = _parser_data.getUniformIntRandomParams();
        if (!uniform_int_names.empty()) {
            os << "\nUniform int numbers:" << std::endl;
            for (std::size_t i = 0; i < uniform_int_names.size(); i++) {
                os << uniform_int_names[i] << ":\tmin: " << uniform_int_params[i].first << "\tmax: "
                   << uniform_int_params[i].second
                   << std::endl;
            }
        }

    }

    void ParserBaseObject::fixParameter(std::string param_name, double val) {
        bool param_found = _base_data.isParamName(param_name);
        if (param_found) {
            try {
                _parameter[_base_data.getParamIndex(param_name)] = val;
            } catch (const std::exception &e) {
                std::ostringstream os;
                os << "Failed to fix parameter " << param_name << ":" << std::endl;
                os << "\t" << e.what() << std::endl;
                throw std::runtime_error(os.str());
            }
            _fixed_parameters.push_back(param_name);
            _input_parameter_ext_by_int.erase(_base_data.getParamIndex(param_name));
        } else {
            std::cerr << "Tried to fix parameter " << param_name << ", but no such parameter defined." << std::endl;
        }
        _all_inputs_defined = _allInputsDefined();
    }

    std::vector<std::string> ParserBaseObject::getUnfixedParameterNames() const {
        std::vector<std::string> param_names = getParameterNames();
        std::vector<std::string>::iterator it;
        for (std::string fixed_param: _fixed_parameters) {
            it = std::find(param_names.begin(), param_names.end(), fixed_param);
            param_names.erase(it);
        }

        std::vector<std::string> param_names_;
        for (std::map<int, int>::const_iterator it = _input_parameter_ext_by_int.begin();
             it != _input_parameter_ext_by_int.end(); it++) {
            param_names_.push_back(_base_data.getParameterNames()[it->first]);
        }
        return param_names;

    }

    void ParserBaseObject::setPointer(double *ptr, std::string name) {
        BaseObject::setPointer(ptr, name);
        if (_allPointerSet()) { _initialize(); }
    }

    /* TODO this solution is not very elegant. ParserBaseObject should have no information about input parameter, since
     * this is something only the model files CRN and InputProvider and MeasurementModels have to deal with, nonetheless
     * we need these functions for all of the above. maybe find a common intemediate class?
    */
    void ParserBaseObject::setInputParameterOrder(const std::vector<std::string> parameter_order) {
        _input_parameter_ext_by_int.clear();
        _num_input_param = parameter_order.size();
        for (int i = 0; i < parameter_order.size(); i++) {
            std::string param_name = parameter_order[i];
            bool is_param = _base_data.isParamName(param_name);
            bool is_fixed = std::find(_fixed_parameters.begin(), _fixed_parameters.end(), param_name) !=
                            _fixed_parameters.end();
            if (is_param && !is_fixed) {
                int param_index = _base_data.getParamIndex(param_name);
                _input_parameter_ext_by_int[param_index] = i;
            }
        }

        _all_inputs_defined = _allInputsDefined();
    }

    void ParserBaseObject::setInputStateOrder(std::vector<std::string> state_order) {
        _input_state_ext_by_int.clear();
        for (int i = 0; i < state_order.size(); i++) {
            std::string state_name = state_order[i];
            bool is_state = _base_data.isSpeciesName(state_name);
            if (is_state) {
                int state_index = _base_data.getSpeciesIndex(state_name);
                _input_state_ext_by_int[state_index] = i;
            }
        }
        _all_inputs_defined = _allInputsDefined();
    }


    bool ParserBaseObject::isParameter(std::string param_name) {
        return _base_data.isParamName(param_name);
    }

    bool ParserBaseObject::isSpecies(std::string species_name) {
        return _base_data.isSpeciesName(species_name);
    }

    void ParserBaseObject::_initializeParser(mu::Parser &p) {
        /*
         * This function initializes the parser, it can only be called after all pointers to the states and parameters have been set
         */

        if (!_allPointerSet()) {
            std::stringstream os;
            os << "Tried to _initialize ParserBaseObject before all pointers have been set. The pointers are:"
               << std::endl;
            printPointer(os);
            throw std::runtime_error(os.str());
        }

        p.DefineFun("log", log);
        p.DefineFun("sqrt", sqrt);
        p.DefineFun("log10", log10);
        p.DefineFun("exp", exp);
        p.DefineConst("_pi", (double) M_PI);
        p.DefineFun("binom", base::MathUtils::binomial);
        p.DefineFun("ceil", ceil);
        p.DefineFun("floor", floor);

        try {
            for (std::size_t i = 0; i < _base_data.getNumSpecies(); i++) {
                p.DefineVar(_base_data.getSpeciesNames()[i], _state_ptrs[i]);
            }

            for (std::size_t i = 0; i < _base_data.getNumParamters(); i++) {
                p.DefineVar(_base_data.getParameterNames()[i], _parameter_ptrs[i]);
            }

            p.DefineVar(_base_data.getTimeName(), _time_ptr);

            std::vector<std::string> uniform_random_nbr_name = _parser_data.getUniformRandomNbrsName();
            for (int uniform_rnb = 0; uniform_rnb < uniform_random_nbr_name.size(); uniform_rnb++) {
                p.DefineVar(uniform_random_nbr_name[uniform_rnb], &(_uniform_numbers[uniform_rnb]));
            }

            std::vector<std::string> normal_random_nbr_name = _parser_data.getNormalRandomNbrsName();
            for (int normal_rnb = 0; normal_rnb < normal_random_nbr_name.size(); normal_rnb++) {
                p.DefineVar(normal_random_nbr_name[normal_rnb], &_normal_numbers[normal_rnb]);
            }

            std::vector<std::string> uniform_int_random_nbr_name = _parser_data.getUniformIntRandomNbrsName();
            for (int uniform_int_rnb = 0; uniform_int_rnb < uniform_int_random_nbr_name.size(); uniform_int_rnb++) {
                p.DefineVar(uniform_int_random_nbr_name[uniform_int_rnb], &_uniform_int_numbers[uniform_int_rnb]);
            }

        } catch (mu::Parser::exception_type &e) {
            std::ostringstream os;
            os << "Could not define variable for ModelBase" << std::endl;
            os << "Message:  " << e.GetMsg() << "\n";
            os << "Formula:  " << e.GetExpr() << "\n";
            os << "Token:    " << e.GetToken() << "\n";
            os << "Position: " << e.GetPos() << "\n";
            os << "Errc:     " << e.GetCode() << "\n";
            throw std::runtime_error(os.str());
        }
    }

    void ParserBaseObject::_createRandomNumbers() {
        for (std::size_t i = 0; i < _normal_dists.size(); i++) {
            _normal_numbers[i] = (_normal_dists[i])(*_rng);
        }
        for (std::size_t i = 0; i < _uniform_dists.size(); i++) {
            _uniform_numbers[i] = (_uniform_dists[i])(*_rng);
        }
        for (std::size_t i = 0; i < _uniform_int_dists.size(); i++) {
            _uniform_int_numbers[i] = (_uniform_int_dists[i])(*_rng);
        }
    }

    void ParserBaseObject::_updateTheta(const std::vector<double> &theta) {
        if (!_input_parameter_ext_by_int.empty() && *_parameter_ptrs[_input_parameter_ext_by_int[0]] != theta[0]) {
            if (theta.size() != _num_input_param) {
                std::stringstream ss;
                ss << "Expected " << _num_input_param << " parameters, but provided theta has "
                   << theta.size() << " entries!" << std::endl;
                throw std::runtime_error(ss.str());
            }
            std::map<int, int>::iterator it;
            for (it = _input_parameter_ext_by_int.begin(); it != _input_parameter_ext_by_int.end(); it++) {
                *_parameter_ptrs[it->first] = theta[it->second];
            }
        }
    }

    bool ParserBaseObject::_allInputsDefined() {
        for (int i = 0; i < _base_data.getNumParamters(); i++) {
            bool param_defined = false;
            if (_input_parameter_ext_by_int.count(i)) { param_defined = true; }
            std::string param_name = _base_data.getParameterNames()[i];
            if (std::find(_fixed_parameters.begin(), _fixed_parameters.end(), param_name) !=
                _fixed_parameters.end()) { param_defined = true; }
            if (!param_defined) { return false; }
        }

        for (int i = 0; i < _base_data.getNumSpecies(); i++) {
            if (!_input_state_ext_by_int.count(i)) { return false; }
        }
        return true;
    }


    void ParserBaseObject::_printInputs() {
        std::cout << "Parameters:" << std::endl;
        for (int i = 0; i < _base_data.getNumParamters(); i++) {
            std::string param_name = _base_data.getParameterNames()[i];
            std::cout << param_name << ":\t";
            if (_input_parameter_ext_by_int.count(i)) { std::cout << " input nbr " << _input_parameter_ext_by_int[i]; }
            if (std::find(_fixed_parameters.begin(), _fixed_parameters.end(), param_name) !=
                _fixed_parameters.end()) { std::cout << "fixed to " << *_parameter_ptrs[i]; }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Species:" << std::endl;
        for (int i = 0; i < _base_data.getNumSpecies(); i++) {
            std::string species_name = _base_data.getSpeciesNames()[i];
            std::cout << species_name << ":\t";
            if (!_input_state_ext_by_int.count(i)) { std::cout << " input nbr " << _input_state_ext_by_int[i]; }
            std::cout << std::endl;
        }
    }

    void ParserBaseObject::_updateState(const double *state) {
        std::map<int, int>::iterator it;
        for (it = _input_state_ext_by_int.begin(); it != _input_state_ext_by_int.end(); it++) {
            *_state_ptrs[it->first] = state[it->second];
        }
    }
}