//
// Created by jan on 12/10/18.
//

#ifndef LFNS_PARSERDATA_H
#define LFNS_PARSERDATA_H


#include "BaseData.h"

namespace models {

    class ParserData : public BaseData {
    public:
        ParserData(std::vector<std::string> parameter_names_,
                   std::vector<std::string> species_names_);

        ParserData(std::string file_name);

        virtual ~ParserData();

        const std::vector<std::string> &getNormalRandomNbrsName() const;

        const std::vector<std::string> &getUniformRandomNbrsName() const;

        const std::vector<std::string> &getUniformIntRandomNbrsName() const;

        const std::vector<std::pair<double, double>> &getNormalRandomParams() const;

        const std::vector<std::pair<double, double>> &getUniformRandomParams() const;

        const std::vector<std::pair<int, int>> &getUniformIntRandomParams() const;

        std::size_t getNumNormalNumbers();

        std::size_t getNumUniformNumbers();

        std::size_t getNumUniformIntNumbers();

    protected:
        std::vector<std::string> _normal_random_nbr;
        std::vector<std::string> _uniform_random_nbr;
        std::vector<std::string> _uniform_int_random_nbr;
        std::vector<std::pair<double, double> > _normal_random_params;
        std::vector<std::pair<double, double> > _uniform_random_params;
        std::vector<std::pair<int, int> > _uniform_int_random_params;
    };
}


#endif //LFNS_PARSERDATA_H
