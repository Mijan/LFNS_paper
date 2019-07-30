//
// Created by jan on 09/07/19.
//

#ifndef LFNS_SIMULATOREXCEPTIONS_H
#define LFNS_SIMULATOREXCEPTIONS_H

namespace simulator {
    class SimulationAborted : public std::exception {
    public:
        SimulationAborted(std::string error_message, double termination_time) : _error_message(error_message),
                                                                                _termination_time(termination_time) {}

        const char *what() const throw() { return _error_message.c_str(); }

        double terminationTime() const { return _termination_time; }

    private:
        std::string _error_message;
        double _termination_time;
    };
}
#endif //LFNS_SIMULATOREXCEPTIONS_H
