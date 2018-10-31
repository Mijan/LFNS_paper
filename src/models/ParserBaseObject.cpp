//
// Created by jan on 12/10/18.
///

#include <set>
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
                                                                                   _state(parser_data.getNumSpecies()),
                                                                                   _time(0),
                                                                                   _input_state_ext_by_int(),
                                                                                   _num_input_param(
                                                                                           parser_data.getNumParamters()),
                                                                                   _perturbation_fct(nullptr) {

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
                                                                 _state(parser_data.getNumSpecies()),
                                                                 _time(0),
                                                                 _input_state_ext_by_int(),
                                                                 _num_input_param(
                                                                         parser_data.getNumParamters()),
                                                                 _perturbation_fct(nullptr) {

        if (parser_data.getNumNormalNumbers() > 0 || parser_data.getNumUniformNumbers() > 0 ||
            parser_data.getNumUniformIntNumbers() > 0) {
            throw std::runtime_error(
                    "Tried to _initialize ParserBaseObject without Random number generator but with random numbers!");
        }


        for (int i = 0; i < _base_data.getNumSpecies(); i++) {
            _state_ptrs[i] = &_state[i];
            _input_state_ext_by_int[i] = i;
        }
        _time_ptr = &_time;
    }


    ParserBaseObject::~ParserBaseObject() {}

    void ParserBaseObject::printInfo(std::ostream &os) const {
        BaseObject::printInfo(os);

        std::vector<std::string> normal_names = _parser_data.getNormalRandomNbrsName();
        std::vector<std::pair<double, double> > normal_params = _parser_data.getNormalRandomParams();
        if (!normal_names.empty()) {
            os << "\nNormal numbers:" << std::endl;
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

    void ParserBaseObject::setPointer(double *ptr, std::string name) {
        BaseObject::setPointer(ptr, name);
        if (_allPointerSet()) { _initialize(); }
    }

    void ParserBaseObject::setStateOrder(std::vector<std::string> state_order) {
        _input_state_ext_by_int.clear();
        for (int i = 0; i < state_order.size(); i++) {
            std::string state_name = state_order[i];
            bool is_state = _base_data.isSpeciesName(state_name);
            if (is_state) {
                int state_index = _base_data.getSpeciesIndex(state_name);
                _input_state_ext_by_int[state_index] = i;
            }
        }
    }


    bool ParserBaseObject::isParameter(std::string param_name) {
        return _base_data.isParamName(param_name);
    }

    bool ParserBaseObject::isSpecies(std::string species_name) {
        return _base_data.isSpeciesName(species_name);
    }

    void ParserBaseObject::setPerturbationFct(PerturbationFct_ptr perturb_fct) {
        _perturbation_fct = perturb_fct;
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

    void ParserBaseObject::_updateState(const double *state) {
        std::map<int, int>::iterator it;
        for (it = _input_state_ext_by_int.begin(); it != _input_state_ext_by_int.end(); it++) {
            *_state_ptrs[it->first] = state[it->second];
        }
    }
}