//
// Created by jan on 12/10/18.
//

#include <sstream>
#include "ModelReactionData.h"
#include "../io/ModelReactionReader.h"
#include "../base/Utils.h"

namespace models {
    ModelReactionData::ModelReactionData(std::vector<std::string> parameter_names_,
                                         std::vector<std::string> species_names_) : ParserData(parameter_names_,
                                                                                               species_names_),
                                                                                    production_species(),
                                                                                    product_species(),
                                                                                    reaction_variables(),
                                                                                    propensities(),
                                                                                    stoichiometry_for_reaction() {}

    ModelReactionData::ModelReactionData(std::string model_file_name) : ParserData(model_file_name),
                                                                        production_species(), product_species(),
                                                                        reaction_variables(), propensities(),
                                                                        stoichiometry_for_reaction() {
        try {
            io::ModelReactionReader reader(model_file_name);
            production_species = reader.readProductionSpecies();
            product_species = reader.readProductSpecies();
            reaction_variables = reader.readReactionVariables();
            propensities = reader.readPropensities();
            createStoichiometry();
        } catch (const std::exception &e) {
            std::stringstream ss;
            ss << "Failed to create ModelReactionData:\n\t" << e.what() << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    ModelReactionData::ModelReactionData(const ModelReactionData &rhs) : ParserData(rhs),
                                                                         production_species(rhs.production_species),
                                                                         product_species(rhs.product_species),
                                                                         reaction_variables(rhs.reaction_variables),
                                                                         propensities(rhs.propensities),
                                                                         stoichiometry_for_reaction(
                                                                                 rhs.stoichiometry_for_reaction) {}

    std::size_t ModelReactionData::getNumReactions() const { return propensities.size(); }

    bool ModelReactionData::isPropMassAction(std::size_t reaction_index) const {
        if (propensities.size() > reaction_index) {
            return propensities[reaction_index].compare(MASS_ACTION_NAME) == 0;
        } else {
            std::stringstream os;
            os << "Tried to check if reaction with index " << reaction_index << " has mass action propensity, but ";
            if (propensities.empty()) {
                os << " no propensities provided!" << std::endl;
            } else {
                os << " only " << propensities.size() << " propensities provided!" << std::endl;
            }
            throw std::runtime_error(os.str());
        }

    }

    void ModelReactionData::createStoichiometry() {

        for (std::size_t reaction_nbr = 0; reaction_nbr < getNumReactions(); reaction_nbr++) {
            std::vector<std::string> production_species_for_reaction = production_species[reaction_nbr];
            std::vector<std::string> product_species_for_reaction = product_species[reaction_nbr];
            std::map<std::size_t, int> stoichiometry;

            for (std::size_t species_nbr = 0; species_nbr < production_species_for_reaction.size(); species_nbr++) {
                std::size_t species_index = getSpeciesIndex(production_species_for_reaction[species_nbr]);
                stoichiometry[species_index] = stoichiometry[species_index] - 1;
            }

            for (std::size_t species_nbr = 0; species_nbr < product_species_for_reaction.size(); species_nbr++) {
                std::size_t species_index = getSpeciesIndex(product_species_for_reaction[species_nbr]);
                stoichiometry[species_index] = stoichiometry[species_index] + 1;
            }
            stoichiometry_for_reaction.push_back(stoichiometry);
        }
    }

}