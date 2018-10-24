//
// Created by jan on 15.12.16.
//

#ifndef LNSV4_INITIALVALUEPROVIDERREADIN_H
#define LNSV4_INITIALVALUEPROVIDERREADIN_H

#include <algorithm>
#include <functional>
#include "InitialValueProvider.h"
#include "ParserBaseObject.h"
#include "InitialValueData.h"

namespace models {
    typedef std::function<void(std::vector<double> *state_0, double *t_0,
                               const std::vector<double> &theta)> InitialStateFct;
    typedef std::shared_ptr<InitialStateFct> InitialStateFct_ptr;

    class InitialValueProvider : public ParserBaseObject {
    public:
        InitialValueProvider(base::RngPtr rng, InitialValueData data);

        InitialValueProvider(InitialValueData data);

        virtual ~InitialValueProvider();

        virtual void computeInitialState(std::vector<double> *state_0, double *t_0,
                                             const std::vector<double> &theta);

        InitialStateFct_ptr getInitialStateFct();

        virtual void printInfo(std::ostream &os);

        void setOutputStateMapping(std::vector<std::string> state_order);

    protected:

    private:
        bool _initialized;
        std::vector<double> _parameter;

        std::vector<mu::Parser> _initial_state_equations;

        mu::Parser _initial_time;

        InitialValueData _initial_data;

        std::vector<int> _output_state_ordering;
        std::map<int, int> _output_state_mapping;

        virtual void _initialize();
    };

    typedef std::shared_ptr<InitialValueProvider> InitialValueProvider_ptr;
}


#endif //LNSV4_INITIALVALUEPROVIDERREADIN_H
