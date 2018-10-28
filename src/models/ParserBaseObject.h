//
// Created by jan on 12/10/18.
//

#ifndef LFNS_PARSERBASE_H
#define LFNS_PARSERBASE_H

#include <muParser.h>
#include "BaseObject.h"
#include "../base/RandomDistributions.h"
#include "ParserData.h"
#include "InputPulse.h"
#include <functional>

namespace models {

    typedef std::function<void(const double* state, double t)> PerturbationFct;
    typedef std::shared_ptr<PerturbationFct> PerturbationFct_ptr;

    class ParserBaseObject : public BaseObject {
    public:
        ParserBaseObject(base::RngPtr rng, ParserData parser_data);

        ParserBaseObject(ParserData parser_data);

        virtual ~ParserBaseObject();

        virtual void setPointer(double *ptr, std::string name);

        virtual void printInfo(std::ostream &os) const;

        void setStateOrder(std::vector<std::string> state_order);

        bool isParameter(std::string param_name);

        bool isSpecies(std::string species_name);

        void setPerturbationFct(PerturbationFct_ptr perturb_fct);

    protected:
        base::RngPtr _rng;
        ParserData _parser_data;
        std::vector<double> _normal_numbers;
        std::vector<double> _uniform_numbers;
        std::vector<double> _uniform_int_numbers;
        std::vector<base::NormalDistribution> _normal_dists;
        std::vector<base::UniformRealDistribution> _uniform_dists;
        std::vector<base::UniformIntDistribution> _uniform_int_dists;


        std::vector<double> _state;
        double _time;

//        std::map<int, int> _input_parameter_ext_by_int;
        std::map<int, int> _input_state_ext_by_int;
        int _num_input_param;
//        bool _all_inputs_defined;

        void _initializeParser(mu::Parser &p);

        void _createRandomNumbers();

//        void _evaluateInput(const double *state, double t, const std::vector<double> &theta);

        void _updateState(const double *state);

        PerturbationFct_ptr _perturbation_fct;

//        virtual bool _allInputsDefined();

//        virtual void _printInputs();

        virtual void _initialize() = 0;
    };

    typedef std::shared_ptr<ParserBaseObject> ParserBaseObject_ptr;
}


#endif //LFNS_PARSERBASE_H
