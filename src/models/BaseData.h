//
// Created by jan on 12/10/18.
//

#ifndef LFNS_BASEDATA_H
#define LFNS_BASEDATA_H


#include <vector>
#include <string>
#include <memory>

namespace models {
    const static std::string TIME_NAME = "_t";

    class BaseData {
    public:
        BaseData(std::vector<std::string> parameter_names_,
                 std::vector<std::string> species_names_);

        BaseData();

        BaseData(const BaseData &rhs);

        virtual ~BaseData();

        std::size_t getNumParamters() const;

        std::size_t getNumSpecies() const;

        bool isSpeciesName(std::string name) const;

        bool isParamName(std::string name) const;

        bool isTimeName(std::string name) const;

        std::size_t getSpeciesIndex(std::string species_name) const;

        std::size_t getParamIndex(std::string param_name) const;

        const std::vector<std::string> &getParameterNames() const;

        const std::vector<std::string> &getSpeciesNames() const;

        std::string getTimeName() const;

    protected:
        std::vector<std::string> parameter_names;
        std::vector<std::string> species_names;
    };

    typedef std::shared_ptr<BaseData> BaseData_ptr;
}


#endif //LFNS_BASEDATA_H
