//
// Created by jan on 26/10/18.
//

#ifndef LFNS_INPUTPULSE_H
#define LFNS_INPUTPULSE_H

#include <vector>
#include <string>
#include <set>
#include "../base/Utils.h"

namespace models {
    struct InputData {
        InputData(double pulse_period_, double pulse_strenght_, double pulse_duration_, int num_pulses_,
                  std::string pulse_inpt_name_, double starting_time_) : pulse_period(
                pulse_period_), pulse_strenght(pulse_strenght_), pulse_duration(pulse_duration_),
                                                                         num_pulses(num_pulses_),
                                                                         pulse_inpt_name(pulse_inpt_name_),
                                                                         starting_time(starting_time_) {}

        double pulse_period;
        double pulse_strenght;
        double pulse_duration;
        int num_pulses;
        std::string pulse_inpt_name;
        double starting_time;
        double final_time;
    };

    class InputPulse {
    public:
        InputPulse(InputData input_data, double final_time);

        virtual ~InputPulse();

        std::vector<double> getDisContTimes();

        bool pulseActive(double t);

        std::vector<double> pulse_beginnings;
        std::vector<double> pulse_ends;
        std::string input_name;
        double _input_strength;
        int parameter_index;
    };

    struct InputPulses {

        InputPulses() : input_pram_indices(), modified_parameter(), pulses() {}

        virtual ~InputPulses() {}

        std::vector<double> getDisContTime() {
            std::vector<double> times;
            for (InputPulse &pulse: pulses) { base::Utils::addOnlyNew(times, pulse.getDisContTimes()); }
            std::sort(times.begin(), times.end());
            return times;
        }

        std::vector<double> modified_parameter;
        std::set<int> input_pram_indices;
        std::vector<InputPulse> pulses;
    };
}


#endif //LFNS_INPUTPULSE_H
