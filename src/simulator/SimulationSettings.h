//
// Created by jan on 29/10/18.
//

#ifndef LFNS_SIMULATIONSETTINGS_H
#define LFNS_SIMULATIONSETTINGS_H

#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#include "../models/InputPulse.h"

namespace simulator {
    enum class MODEL_TYPE {
        ODE, STOCH
    };
    static std::map<std::string, MODEL_TYPE> MODEL_TYPE_NAME = {{"DET",   MODEL_TYPE::ODE},
                                                                {"STOCH", MODEL_TYPE::STOCH}};

    class SimulationSettings {
    public:
        simulator::MODEL_TYPE model_type = simulator::MODEL_TYPE::STOCH;
        std::vector<std::string> param_names;


        void print(std::ostream &stream) {
            if (model_type == simulator::MODEL_TYPE::STOCH) {
                stream << "A SSA simulator will be used" << std::endl;
            } else {
                stream << "An ODE simulator will be used" << std::endl;
            }
        }
    };
}


#endif //LFNS_SIMULATIONSETTINGS_H
