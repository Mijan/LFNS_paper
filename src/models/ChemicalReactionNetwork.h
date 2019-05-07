//
// Created by jan on 12/10/18.
//

#ifndef LFNS_CHEMICALREACTIONNETWORK_H
#define LFNS_CHEMICALREACTIONNETWORK_H

#include <ostream>
#include <functional>
#include "ParserBaseObject.h"
#include "ModelReactionData.h"

namespace models {
    typedef std::function<void(std::vector<double> &propensities, std::vector<double> &state, double t)> PropensityFct;
    typedef std::shared_ptr<PropensityFct> PropensityFct_ptr;

    typedef std::function<void(std::vector<double> &state, int reaction_index)> ReactionFct;
    typedef std::shared_ptr<ReactionFct> ReactionFct_ptr;

    typedef std::function<void(double *dx, const double *state, double t)> RhsFct;
    typedef std::shared_ptr<RhsFct> RhsFct_ptr;


    class ChemicalReactionNetwork : public ParserBaseObject {
    public:
        ChemicalReactionNetwork(ModelReactionData model_data);

        virtual ~ChemicalReactionNetwork();

        void updatePropensities(std::vector<double> &propensities, std::vector<double> &state, double t);

        PropensityFct_ptr getPropensityFct();

        void fireReaction(std::vector<double> &state, int reaction_index);

        ReactionFct_ptr getReactionFct();

        void rhs(double *dx, const double *state, double t);

        RhsFct_ptr getRhsFct();

        std::size_t getNumReactions();

        void _initialize();

        void setDetStatesForHybridModel(std::vector<std::string> det_state_names);

        void setStochStatesForHybridModel(std::vector<std::string> stoch_state_name);

        std::vector<int> getStochIndices();

        std::vector<int> getDetSpeciesIndices();

        void printInfo(std::ostream &os) const;

    private:
        bool _initialized;

        std::vector<mu::Parser> _propensity_parsers;
        std::vector<mu::Parser> _rhs_parsers;
        ModelReactionData _model_data;

        std::vector<bool> _det_index_hybrid_model;
        std::vector<bool> _active_reactions_hybrid_model;

        void _createPropensityParsers();

        void _createRhsParsers();

        std::string _getStochMassActionPropensity(std::size_t reaction_nbr);

        std::map<size_t, std::vector<size_t> > _getReactionBySpecies() const;

        std::string _getOdeEquationForSpecies(size_t species_nbr,
                                              const std::map<size_t, std::vector<size_t> > &_reaction_nbr_by_species);

        std::string _getDetMassActionKinetics(std::size_t reaction_nbr);
    };

    typedef std::shared_ptr<ChemicalReactionNetwork> ChemicalReactionNetwork_ptr;
}


#endif //LFNS_CHEMICALREACTIONNETWORK_H
