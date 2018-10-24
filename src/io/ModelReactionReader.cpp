//
// Created by jan on 12/10/18.
//

#include "ModelReactionReader.h"

#include <boost/algorithm/string.hpp>
#include <algorithm>

namespace io {

    ModelReactionReader::ModelReactionReader(std::string model_file_name) : ParserReader(model_file_name) {}

    ModelReactionReader::~ModelReactionReader() {}

    std::vector<std::vector<std::string> > ModelReactionReader::readProductionSpecies() {
        std::vector<std::string> reaction_lines = _readCaptionLines(REACTIONS_CAPTION);
        std::vector<std::vector<std::string> > production_species_vector;

        for (std::size_t line_nbr = 0; line_nbr < reaction_lines.size();
             line_nbr++) {
            std::string reaction = _readReactionLineReaction(reaction_lines[line_nbr]);
            std::vector<std::string> reaction_species = _splitInTwo(reaction, "-->");
            std::vector<std::string> production_species = _readStrippedDelimitterdLine(reaction_species[0], "+");

            _removeNULLSpecies(production_species);
            production_species_vector.push_back(production_species);
        }

        return production_species_vector;
    }

    std::vector<std::vector<std::string> > ModelReactionReader::readProductSpecies() {
        std::vector<std::string> reaction_lines = _readCaptionLines(REACTIONS_CAPTION);
        std::vector<std::vector<std::string> > production_species_vector;

        for (std::size_t line_nbr = 0; line_nbr < reaction_lines.size();
             line_nbr++) {
            std::string reaction = _readReactionLineReaction(reaction_lines[line_nbr]);
            std::vector<std::string> reaction_species = _splitInTwo(reaction, "-->");
            std::vector<std::string> product_species = _readStrippedDelimitterdLine(reaction_species[1], "+");

            _removeNULLSpecies(product_species);
            production_species_vector.push_back(product_species);
        }
        return production_species_vector;
    }

    std::vector<std::vector<std::string> > ModelReactionReader::readReactionVariables() {
        std::vector<std::string> reaction_lines = _readCaptionLines(REACTIONS_CAPTION);
        std::vector<std::vector<std::string> > variables_vector;

        for (std::size_t line_nbr = 0; line_nbr < reaction_lines.size();
             line_nbr++) {
            std::vector<std::string> variables = _readReactionLineVariables(reaction_lines[line_nbr]);
            variables_vector.push_back(variables);
        }
        return variables_vector;
    }

    std::vector<std::string> ModelReactionReader::readPropensities() {
        std::vector<std::string> reaction_lines = _readCaptionLines(REACTIONS_CAPTION);
        std::vector<std::string> propensities;

        for (std::size_t line_nbr = 0; line_nbr < reaction_lines.size();
             line_nbr++) {
            std::string propensity = _readReactionLinePropensity(reaction_lines[line_nbr]);
            propensities.push_back(boost::erase_all_copy(propensity, " "));
        }
        return propensities;
    }

    std::string ModelReactionReader::_readReactionLineReaction(std::string line) {
        std::vector<std::string> reaction_variables = _splitInTwo(line, VARIABLES_CAPTION);
        return boost::erase_all_copy(reaction_variables[0], " ");
    }

    std::vector<std::string> ModelReactionReader::_readReactionLineVariables(
            std::string line) {
        std::vector<std::string> reaction_variables = _splitInTwo(line, VARIABLES_CAPTION);
        std::vector<std::string> variables_proponsity = _splitInTwo(reaction_variables[1], PROPENSITY_CAPTION);
        return _readStrippedDelimitterdLine(variables_proponsity[0], ",");
    }

    std::string ModelReactionReader::_readReactionLinePropensity(std::string line) {
        std::vector<std::string> reaction_variables = _splitInTwo(line, VARIABLES_CAPTION);
        std::vector<std::string> variables_propensity = _splitInTwo(reaction_variables[1], PROPENSITY_CAPTION);
        return boost::erase_all_copy(variables_propensity[1], " ");;
    }

    void ModelReactionReader::_removeNULLSpecies(std::vector<std::string> &species_vector) {
        std::vector<std::string>::iterator it = std::find(species_vector.begin(), species_vector.end(), NULL_STRING);
        while (it != species_vector.end()) {
            species_vector.erase(it);
            it = std::find(species_vector.begin(), species_vector.end(), NULL_STRING);
        }
    }
}