//
// Created by jan on 26/10/18.
//

#include "InputPulse.h"
#include "../base/MathUtils.h"
#include <algorithm>

namespace models {
    InputPulse::InputPulse(InputData input_data)
            : pulse_beginnings(), pulse_ends(), input_name(input_data.pulse_inpt_name),
              _input_strength(input_data.pulse_strenght),
              parameter_index(-1) {
        int effective_num_pulses = input_data.num_pulses;
//        std::min(input_data.num_pulses,
//                                            (int) ((final_time - input_data.starting_time) / (input_data.pulse_period) +
//                                                   1));
        pulse_beginnings = std::vector<double>(effective_num_pulses, 0.0);
        pulse_ends = std::vector<double>(effective_num_pulses, 0.0);
        pulse_beginnings[0] = input_data.starting_time;
        pulse_ends[0] = input_data.starting_time + input_data.pulse_duration;

        for (int i = 1; i < effective_num_pulses; i++) {
            pulse_beginnings[i] = pulse_beginnings[i - 1] + input_data.pulse_period;
            pulse_ends[i] = pulse_beginnings[i] + input_data.pulse_duration;
        }
    }

    InputPulse::~InputPulse() {}

    std::vector<double> InputPulse::getDisContTimes() {
        std::vector<double> dis_cont_times(2 * pulse_beginnings.size(), 0.0);
        int j = 0;
        for (int i = 0; i < pulse_beginnings.size(); i++) {
            dis_cont_times[j++] = pulse_beginnings[i];
            dis_cont_times[j++] = pulse_ends[i];
        }
        return dis_cont_times;
    }

    bool InputPulse::pulseActive(double t) {
        if (t < pulse_beginnings.front()) { return false; }
        int pulse_index = base::MathUtils::findIndexSmallerThan(pulse_beginnings, t);
        if (t < pulse_ends[pulse_index]) { return true; } else { return false; }
    }
}