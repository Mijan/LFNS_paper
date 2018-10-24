//
// Created by jan on 12/10/18.
//

#ifndef LFNS_MODELREADER_H
#define LFNS_MODELREADER_H


#include <stdexcept>
#include <sstream>
#include "ParserReader.h"

namespace io {


    const std::string REACTIONS_CAPTION = "Reactions:";
    const std::string VARIABLES_CAPTION = "Variables:";
    const std::string PROPENSITY_CAPTION = "Propensity:";
    const std::string NULL_STRING = "0";
    
    class ModelReactionReader : public ParserReader {
    public:
        ModelReactionReader(std::string model_file_name);

        virtual ~ModelReactionReader();

        std::vector<std::vector<std::string> > readProductionSpecies();

        std::vector<std::vector<std::string> > readProductSpecies();

        std::vector<std::vector<std::string> > readReactionVariables();

        std::vector<std::string> readPropensities();

    private:
        std::vector<std::string> _txt_file_lines;

        std::string _readReactionLineReaction(std::string line);

        std::vector<std::string> _readReactionLineVariables(std::string line);

        std::string _readReactionLinePropensity(std::string line);

        void _removeNULLSpecies(std::vector<std::string> &species_vector);
    };

    class ModelAnaReader : public ParserReader {
    public:
        ModelAnaReader(std::string model_file_name) : ParserReader(model_file_name) {}

        virtual ~ModelAnaReader() {};

        std::string readEquationForSpecies(std::string species_name) {
            std::vector<std::string> equation_lines = _readCaptionLines(EQUATIONS_CAPTION);

            for (std::string equation_line : equation_lines) {
                std::vector<std::string> equations = _splitInTwo(equation_line, ":");
                if (equations[0].compare(species_name) == 0) {
                    return equations[1];
                }
            }
            std::stringstream os;
            os << "No Equations line for species '" << species_name << "' found!" << std::endl;
            throw std::runtime_error(os.str());
        }

    private:
        const std::string EQUATIONS_CAPTION = "Equations:";
    };
}


#endif //LFNS_MODELREADER_H
