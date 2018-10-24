//
// Created by jan on 12/10/18.
//

#include <stdexcept>
#include <sstream>
#include "ParserData.h"
#include "../io/ParserReader.h"

namespace models {
    ParserData::ParserData(std::vector<std::string>
                           parameter_names_,
                           std::vector<std::string> species_names_) : BaseData(parameter_names_, species_names_),
                                                                      _normal_random_nbr(), _uniform_random_nbr(),
                                                                      _uniform_int_random_nbr(),
                                                                      _normal_random_params(),
                                                                      _uniform_random_params(),
                                                                      _uniform_int_random_params() {}

    ParserData::ParserData(std::string file_name) : BaseData(),
                                                    _normal_random_nbr(), _uniform_random_nbr(),
                                                    _uniform_int_random_nbr(),
                                                    _normal_random_params(),
                                                    _uniform_random_params(), _uniform_int_random_params() {
        try {
            io::ParserReader reader(file_name);
            parameter_names = reader.readParameterNames();
            species_names = reader.readSpeciesNames();
            _normal_random_nbr = reader.readNormalRdn();
            _uniform_random_nbr = reader.readUniformRdn();
            _uniform_int_random_nbr = reader.readUniformIntRdn();
            _normal_random_params = reader.readNormalParams();
            _uniform_random_params = reader.readUniformParams();
            _uniform_int_random_params = reader.readUniformIntParams();
        } catch (const std::exception &e) {
            std::stringstream os;
            os << "Failed to crate Parser Data: " << "\n\t" << e.what() << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    ParserData::~ParserData() {}

    const std::vector<std::string> &ParserData::getNormalRandomNbrsName() const {
        return _normal_random_nbr;
    }

    const std::vector<std::string> &ParserData::getUniformRandomNbrsName() const {
        return _uniform_random_nbr;
    }

    const std::vector<std::string> &ParserData::getUniformIntRandomNbrsName() const {
        return _uniform_int_random_nbr;
    }

    const std::vector<std::pair<double, double>> &ParserData::getNormalRandomParams() const {
        return _normal_random_params;
    }

    const std::vector<std::pair<double, double>> &ParserData::getUniformRandomParams() const {
        return _uniform_random_params;
    }

    const std::vector<std::pair<int, int>> &ParserData::getUniformIntRandomParams() const {
        return _uniform_int_random_params;
    }

    std::size_t ParserData::getNumNormalNumbers() { return _normal_random_nbr.size(); }

    std::size_t ParserData::getNumUniformNumbers() { return _uniform_random_nbr.size(); }

    std::size_t ParserData::getNumUniformIntNumbers() { return _uniform_int_random_nbr.size(); }

}