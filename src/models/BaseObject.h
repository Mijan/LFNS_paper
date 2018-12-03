//
// Created by jan on 12/10/18.
//

#ifndef LFNS_BASEOBJECT_H
#define LFNS_BASEOBJECT_H

#include <memory>
#include "BaseData.h"

namespace models {
    class BaseObject {
    public:
        BaseObject(BaseData);

        BaseObject(std::vector<std::string> parameter_names, std::vector<std::string> species_names);

        virtual void setPointer(double *ptr, std::string name);

        virtual void setPointers(std::vector<double *> ptrs, std::vector<std::string> names);

        virtual void printPointer(std::ostream &os) const;

        virtual void printPointerValue(std::ostream &os) const;

        double *parameterPtr(std::size_t i);

        double *statePtr(std::size_t i);

        double *timePtr();

        const std::vector<std::string> &getSpeciesNames() const;

        const int getNumSpecies() const;

        const std::vector<std::string> &getParameterNames() const;

        const int getNumParameters() const;

        virtual void printInfo(std::ostream &os) const;

        BaseData &getBaseData();

    protected:

        std::vector<double *> _parameter_ptrs;
        std::vector<double *> _state_ptrs;
        double *_time_ptr;

        BaseData _base_data;

        virtual bool _allPointerSet() const;

        bool _parameterPointerSet() const;

        bool _statePointerSet() const;

        bool _timePointerSet() const;
    };

    typedef std::shared_ptr<BaseObject> BaseObject_ptr;
}


#endif //LFNS_BASEOBJECT_H
