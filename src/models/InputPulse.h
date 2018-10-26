//
// Created by jan on 26/10/18.
//

#ifndef LFNS_INPUTPULSE_H
#define LFNS_INPUTPULSE_H

#include <vector>
#include <string>

namespace models {

    class InputPulse {
    public:
        InputPulse(double pulse_period, double pulse_strenght, double pulse_duration, int num_pulses,
                   std::string pulse_inpt_name, double starting_time, double final_time);

        virtual ~InputPulse();

        std::vector<double> getDisContTimes();

        bool pulseActive(double t);

        std::vector<double> pulse_beginnings;
        std::vector<double> pulse_ends;
        std::string input_name;
        double _input_strength;
        int parameter_index;
    };
}


#endif //LFNS_INPUTPULSE_H
