//
// Created by jan on 09/07/19.
//

#ifndef LFNS_MODELEXCEPTIONS_H
#define LFNS_MODELEXCEPTIONS_H

namespace models {
    class ModelException : public std::exception {
    public:
        ModelException(std::string error_message) : _error_message(error_message) {}

        const char *what() const throw() { return _error_message.c_str(); }

    private:
        std::string _error_message;
    };
}
#endif //LFNS_MODELEXCEPTIONS_H
