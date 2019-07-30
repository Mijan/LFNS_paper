//
// Created by jan on 12/10/18.
//

#include <iomanip>
#include <set>
#include "ChemicalReactionNetwork.h"
#include "InputPulse.h"

namespace models {

    using namespace std::placeholders;

    ChemicalReactionNetwork::ChemicalReactionNetwork(ModelReactionData model_data) : ParserBaseObject(model_data),
                                                                                     _initialized(false),
                                                                                     _propensity_parsers(
                                                                                             model_data.getNumReactions()),
                                                                                     _rhs_parsers(
                                                                                             model_data.getNumSpecies()),
                                                                                     _model_data(model_data) {}

    ChemicalReactionNetwork::~ChemicalReactionNetwork() {}

    void
    ChemicalReactionNetwork::updatePropensities(std::vector<double> &propensities, std::vector<double> &state,
                                                double t) {

        if (!_initialized) {
            std::stringstream os;
            os << "ChemicalReactionNetwork must be initialized before measurement can be computed!"
               << std::endl;
            if (!_allPointerSet()) { printPointer(os); }
            throw std::runtime_error(os.str());
        }
        if (propensities.size() != _propensity_parsers.size()) {
            std::stringstream os;
            os << "Tried to update propensty object with " << propensities.size() << " propensities, but "
               << _propensity_parsers.size() << " required!" << std::endl;
            throw std::runtime_error(os.str());
        }

        if (_perturbation_fct) { (*_perturbation_fct)(state.data(), t); }
        _updateState(state.data());
        for (std::size_t prop_nbr = 0; prop_nbr < propensities.size(); prop_nbr++) {
            try {
                if (_active_reactions_hybrid_model.empty() || _active_reactions_hybrid_model[prop_nbr]) {
                    propensities[prop_nbr] = _propensity_parsers[prop_nbr].Eval();
                } else {
                    propensities[prop_nbr] = 0;
                }
            } catch (mu::ParserError &e) {
                std::ostringstream os;
                os << "Parser error for propensity computation in reaction  "
                   << prop_nbr << " and expression : " << e.GetExpr()
                   << std::endl;
                os << "Message:  " << e.GetMsg() << "\n";
                os << "Formula:  " << e.GetExpr() << "\n";
                os << "Token:    " << e.GetToken() << "\n";
                os << "Position: " << e.GetPos() << "\n";
                os << "Errc:     " << e.GetCode() << "\n";
                throw std::runtime_error(os.str());
            }
        }
    }

    PropensityFct_ptr ChemicalReactionNetwork::getPropensityFct() {
        return std::make_shared<PropensityFct>(
                std::bind(&ChemicalReactionNetwork::updatePropensities, this, _1, _2, _3));
    }

    void ChemicalReactionNetwork::fireReaction(std::vector<double> &state, int reaction_index) {

        if (!_initialized) {
            std::stringstream os;
            os << "InitialValueProvider must be initialized before measurement can be computed!"
               << std::endl;
            if (!_allPointerSet()) { printPointer(os); }
            throw std::runtime_error(os.str());
        }
        const std::map<std::size_t, int> &stoichiometry_map = _model_data.stoichiometry_for_reaction[reaction_index];
        std::map<std::size_t, int>::const_iterator it;
        for (it = stoichiometry_map.begin(); it != stoichiometry_map.end(); it++) {
            state[it->first] += it->second;
        }
    }

    ReactionFct_ptr ChemicalReactionNetwork::getReactionFct() {
        return std::make_shared<ReactionFct>(std::bind(&ChemicalReactionNetwork::fireReaction, this, _1, _2));
    }


    void ChemicalReactionNetwork::rhs(double *dx, const double *state, double t) {


        if (!_initialized) {
            std::stringstream os;
            os << "ChemicalReactionNetwork must be initialized before measurement can be computed!"
               << std::endl;
            if (!_allPointerSet()) { printPointer(os); }
            throw std::runtime_error(os.str());
        }

        if (_perturbation_fct) { (*_perturbation_fct)(state, t); }
        _updateState(state);
        std::size_t species_index = 0;
        for (mu::Parser &p : _rhs_parsers) {
            if (_det_index_hybrid_model.empty() || _det_index_hybrid_model[species_index]) {
                dx[species_index] = p.Eval();
            } else {
                dx[species_index] = 0;
            }
            species_index++;

        }
    }

    RhsFct_ptr ChemicalReactionNetwork::getRhsFct() {
        return std::make_shared<RhsFct>(std::bind(&ChemicalReactionNetwork::rhs, this, _1, _2, _3));
    }


    std::size_t ChemicalReactionNetwork::getNumReactions() { return _model_data.getNumReactions(); }


    void ChemicalReactionNetwork::setDetStatesForHybridModel(std::vector<std::string> det_state_names) {
        _det_index_hybrid_model.resize(_state.size());
        std::vector<std::string> species_names = getSpeciesNames();
        for (int i = 0; i < species_names.size(); i++) {
            std::string species_name = species_names[i];
            std::vector<std::string>::iterator it = std::find(det_state_names.begin(), det_state_names.end(),
                                                              species_name);
            if (it == det_state_names.end()) {
                _det_index_hybrid_model[i] = false;
            } else {
                _det_index_hybrid_model[i] = true;
            }
        }

        _active_reactions_hybrid_model.resize(getNumReactions());
        std::fill(_active_reactions_hybrid_model.begin(), _active_reactions_hybrid_model.end(), true);
        std::map<size_t, std::vector<size_t> > reaction_by_species = _getReactionBySpecies();
        for (std::string &species_name : det_state_names) {
            if (!_model_data.isSpeciesName(species_name)) {
                std::stringstream ss;
                ss << "Tried to set deterministic species " << species_name
                   << " for Hybrid model, but no such species seems to be defined!" << std::endl;
                throw std::runtime_error(ss.str());
            }
            std::vector<std::size_t> reactions = reaction_by_species[_model_data.getSpeciesIndex(species_name)];
            for (std::size_t reaction_index : reactions) {
                _active_reactions_hybrid_model[reaction_index] = false;
            }
        }
    }


    void ChemicalReactionNetwork::setStochStatesForHybridModel(std::vector<std::string> stoch_state_names) {
        std::vector<std::string> det_states = getSpeciesNames();
        for (std::string &stoch_state : stoch_state_names) {
            if (!_model_data.isSpeciesName(stoch_state)) {
                std::stringstream ss;
                ss << "Tried to set stochastic species " << stoch_state
                   << " for Hybrid model, but no such state defined!" << std::endl;
                throw std::runtime_error(ss.str());
            }
            std::vector<std::string>::iterator it = std::find(det_states.begin(), det_states.end(), stoch_state);

            if (it == det_states.end()) {
                std::cerr << "Tried to set stochastic species " << stoch_state
                          << " twice, ignoring the second entry"
                          << std::endl;
            }
            det_states.erase(it);
        }
        setDetStatesForHybridModel(det_states);
    }


    std::vector<int> ChemicalReactionNetwork::getStochIndices() {
        std::vector<int> stoch_indices;
        for (int i = 0; i < _det_index_hybrid_model.size(); i++) {
            if (!_det_index_hybrid_model[i]) { stoch_indices.push_back(i); }
        }
        return stoch_indices;
    }

    std::vector<int> ChemicalReactionNetwork::getDetSpeciesIndices() {

        std::vector<int> det_indices;
        std::map<size_t, std::vector<size_t> > reactions = _getReactionBySpecies();
        for (int i = 0; i < _det_index_hybrid_model.size(); i++) {
            if (_det_index_hybrid_model[i]) {
                std::vector<std::size_t> reaction_nbrs = reactions[i];
                for (std::size_t reaction_nbr : reaction_nbrs) {
                    std::vector<std::string> production_species = _model_data.production_species[reaction_nbr];
                    for (std::string species : production_species) {
                        base::Utils::addOnlyNew(det_indices, {_model_data.getSpeciesIndex(species)});
                    }
                }
            }
        }
        return det_indices;
    }

    void ChemicalReactionNetwork::printInfo(std::ostream &os) const {
        os << "\n\n-----------   Model   -----------" << std::endl;
        ParserBaseObject::printInfo(os);
        os << std::endl << std::endl;
        for (std::size_t i = 0; i < _propensity_parsers.size(); i++) {
            os << "Reaction " << std::setw(2) << i << ":";

            std::size_t max_prop_length = 0;
            for (const mu::Parser &p : _propensity_parsers) {
                if (max_prop_length < p.GetExpr().size()) { max_prop_length = p.GetExpr().size(); }
            }
            os << "\tPropencity: " << std::setw(max_prop_length + 1) << _propensity_parsers[i].GetExpr();

            os << "\t\tStoichiometry: ";
            const std::map<std::size_t, int> &stoich = _model_data.stoichiometry_for_reaction[i];
            for (std::size_t species_nbr = 0; species_nbr < _model_data.getNumSpecies(); species_nbr++) {
                if (stoich.count(species_nbr) > 0) {
                    bool print_stoich = stoich.at(species_nbr) != 0;
                    if (print_stoich) {
                        os << _model_data.getSpeciesNames()[species_nbr] << " ";
                        if (stoich.at(species_nbr) > 0) { os << "+"; }
                        os << stoich.at(species_nbr) << ", ";
                    }
                }
            }

            os << std::endl;
        }
        os << "\n\nOde equations:" << std::endl;
        std::size_t max_derr_length = 0;
        for (const std::string &name : _model_data.getSpeciesNames()) {
            if (max_derr_length < name.size()) { max_derr_length = name.size(); }
        }

        for (std::size_t i = 0; i < _rhs_parsers.size(); i++) {
            std::string derriv_str = "(d " + _model_data.getSpeciesNames()[i] + ")/dt";
            os << std::setw(max_derr_length + 7) << derriv_str << " = " << _rhs_parsers[i].GetExpr()
               << std::endl;
        }
        os << std::endl;
    }

    void ChemicalReactionNetwork::_initialize() {
        try {
            if (!_allPointerSet()) {
                std::stringstream os;
                os
                        << "Tried to _initialize ChemicalReactionNetwork before all pointers have been set. The pointers are:"
                        << std::endl;
                printPointer(os);
                throw std::runtime_error(os.str());
            }
            _createPropensityParsers();
            _createRhsParsers();

            _initialized = true;
        } catch (const std::exception &e) {
            std::stringstream os;
            os << "Failed to _initialize ChemicalReactionNetwork:" << std::endl;
            os << "\t" << e.what() << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    void ChemicalReactionNetwork::_createPropensityParsers() {
        for (std::size_t reaction_nbr = 0;
             reaction_nbr < _model_data.getNumReactions(); reaction_nbr++) {
            mu::Parser p;
            _initializeParser(p);

            if (_model_data.isPropMassAction(reaction_nbr)) {
                std::string propensity_str = _getStochMassActionPropensity(reaction_nbr);
                p.SetExpr(propensity_str);
            } else {
                p.SetExpr(_model_data.propensities[reaction_nbr]);
            }
            _propensity_parsers[reaction_nbr] = p;
        }
    }


    void ChemicalReactionNetwork::_createRhsParsers() {
        _rhs_parsers = std::vector<mu::Parser>(_model_data.getNumSpecies());
        std::map<size_t, std::vector<size_t> > reaction_nbr_by_species = _getReactionBySpecies();

        for (std::size_t species_nbr = 0; species_nbr < _model_data.getNumSpecies(); species_nbr++) {
            mu::Parser p;
            _initializeParser(p);

            std::string propensity_string = _getOdeEquationForSpecies(species_nbr, reaction_nbr_by_species);

            p.SetExpr(propensity_string.size() > 0 ? propensity_string : "0");
            _rhs_parsers[species_nbr] = p;
        }
    }

    std::string ChemicalReactionNetwork::_getStochMassActionPropensity(std::size_t reaction_nbr) {
        std::vector<std::string> production_species = _model_data.production_species[reaction_nbr];

        std::map<std::string, std::size_t> unique_species;
        std::vector<std::string> propensity_str_vector = _model_data.reaction_variables[reaction_nbr];
        if (propensity_str_vector.size() != 1) {
            std::ostringstream os;
            os
                    << "For mass action kinetics, exactly 1 parameter needs to be provided! For mass action reaction "
                    << reaction_nbr << " are " << propensity_str_vector.size()
                    << "parameters provided!" << std::endl;
            throw std::runtime_error(os.str());
        }
        std::string propensity_str = propensity_str_vector[0];
        for (std::size_t i = 0; i < production_species.size(); i++) {
            std::string species = production_species[i];
            if (unique_species.count(species) == 0) {
                unique_species[species] = 1;
            } else {
                unique_species[species] += 1;
            }
        }

        std::map<std::string, std::size_t>::iterator it;
        for (it = unique_species.begin(); it != unique_species.end(); it++) {
            if (it->second > 1) {
                std::ostringstream out;
                out << it->second;
                propensity_str = propensity_str + "*binom(" + it->first + "," + out.str() + ")";
            } else {
                propensity_str = propensity_str + "*" + it->first;
            }
        }
        return propensity_str;
    }

    std::map<size_t, std::vector<size_t> > ChemicalReactionNetwork::_getReactionBySpecies() const {
        std::map<std::size_t, std::vector<std::size_t> > _reaction_nbr_by_species;
        for (std::size_t species_nbr = 0; species_nbr < _model_data.getNumSpecies(); species_nbr++) {
            _reaction_nbr_by_species[species_nbr] = std::vector<std::size_t>();
            for (std::size_t reaction_nbr = 0; reaction_nbr < _model_data.getNumReactions(); reaction_nbr++) {
                const std::map<std::size_t, int> &stoichiometry =
                        _model_data.stoichiometry_for_reaction[reaction_nbr];
                if ((stoichiometry.find(species_nbr) != stoichiometry.end()) &&
                    stoichiometry.at(species_nbr) != 0) {
                    _reaction_nbr_by_species[species_nbr].push_back(reaction_nbr);
                }
            }
        }
        return _reaction_nbr_by_species;
    }

    std::string ChemicalReactionNetwork::_getOdeEquationForSpecies(size_t species_nbr,
                                                                   const std::map<size_t, std::vector<size_t> > &_reaction_nbr_by_species) {
        const std::vector<std::size_t> &involved_reactions =
                _reaction_nbr_by_species.at(species_nbr);
        std::string propensity_string = "";

        for (std::size_t reaction_nbr = 0;
             reaction_nbr < involved_reactions.size(); reaction_nbr++) {
            std::size_t reaction_index = involved_reactions[reaction_nbr];

            const std::map<std::size_t, int> &stoichiometry =
                    _model_data.stoichiometry_for_reaction[reaction_index];

            if (stoichiometry.at(species_nbr) != 0) {
                int stoichiometry_sgn = (stoichiometry.at(species_nbr) > 0)
                                        - (stoichiometry.at(species_nbr) < 0);
                int stoichiometry_value = stoichiometry.at(species_nbr)
                                          * stoichiometry_sgn;

                std::string reaction_prop_str;
                if (_model_data.isPropMassAction(reaction_index)) {
                    reaction_prop_str = _getDetMassActionKinetics(reaction_index);
                } else {
                    reaction_prop_str = _model_data.propensities[reaction_index];
                }

                if (reaction_nbr > 0 || stoichiometry_sgn < 0) {
                    std::string sign_string = "";
                    if (stoichiometry_sgn > 0) {
                        sign_string = "+";
                    } else if (stoichiometry_sgn < 0) {
                        sign_string = "-";
                    }
                    propensity_string += sign_string;
                }
                if (stoichiometry_value != 1) {
                    std::ostringstream stoichiometry_value_str;
                    stoichiometry_value_str << stoichiometry_value;
                    propensity_string += stoichiometry_value_str.str() + "*";
                }
                propensity_string += "(" + reaction_prop_str + ")";
            }
        }
        return propensity_string;
    }

    std::string ChemicalReactionNetwork::_getDetMassActionKinetics(std::size_t reaction_nbr) {
        std::vector<std::string> production_species = _model_data.production_species[reaction_nbr];
        std::set<std::string> unique_species;
        std::vector<std::string> reaction_variables = _model_data.reaction_variables[reaction_nbr];
        if (reaction_variables.size() != 1) {
            std::ostringstream os;
            os << "For mass action kinetics, exactly 1 parameter needs to be provided! For mass action reaction "
               << reaction_nbr << " parameters ";
            for (std::string var : reaction_variables) {
                os << "'" << var << "' ";
            }
            os << "' are provided!" << std::endl;
            throw std::runtime_error(os.str());
        }
        std::string propensity_str = reaction_variables[0];
        for (std::size_t i = 0; i < production_species.size(); i++) {
            std::string species = production_species[i];
            if (unique_species.count(species) == 0) {
                unique_species.insert(species);
            }
            propensity_str = propensity_str + " * " + species;
        }
        return propensity_str;
    }


}