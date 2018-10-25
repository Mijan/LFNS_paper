//
// Created by jan on 12/10/18.
//

#ifndef LFNS_MODELDATA_H
#define LFNS_MODELDATA_H

#include <map>
#include "BaseData.h"
#include "ParserBaseObject.h"

namespace models {
    const std::string MASS_ACTION_NAME = "#ma";

    class ModelReactionData : public ParserData {
    public:
        ModelReactionData(std::vector<std::string> parameter_names_, std::vector<std::string> species_names_);

        ModelReactionData(std::string model_file_name);

        std::size_t getNumReactions() const;

        bool isPropMassAction(std::size_t reaction_index) const;

        void createStoichiometry();

        std::vector<std::vector<std::string> > production_species;
        std::vector<std::vector<std::string> > product_species;
        std::vector<std::vector<std::string> > reaction_variables;
        std::vector<std::string> propensities;

        std::vector<std::map<std::size_t, int> > stoichiometry_for_reaction;
    };

    typedef std::shared_ptr<ModelReactionData> ModelData_ptr;
}


#endif //LFNS_MODELDATA_H
