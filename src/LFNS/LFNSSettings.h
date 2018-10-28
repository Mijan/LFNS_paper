//
// Created by jan on 04/10/18.
//

#ifndef LFNS_LFNSSETTINGS_H
#define LFNS_LFNSSETTINGS_H

#include <vector>
#include <string>
#include <map>

namespace lfns {

    enum DENSITY_ESTIMATOR {
        REJECT_DPGMM, KDE_GAUSS, KDE_UNIFORM, ELLIPS
    };
    enum class MODEL_TYPE {
        ODE, STOCH
    };
    static std::map<std::string, MODEL_TYPE> MODEL_TYPE_NAME = {{"DET",   MODEL_TYPE::ODE},
                                                                {"STOCH", MODEL_TYPE::STOCH}};
    enum class SAMPLER_TYPE {
        UNIFORM, NORMAL
    };
    static std::map<SAMPLER_TYPE, std::string> SAMPLER_TYPE_NAME = {{SAMPLER_TYPE::UNIFORM, "Uniform"},
                                                                    {SAMPLER_TYPE::NORMAL,  "Normal"}};


    struct ParameterSetting {
        ParameterSetting(std::string name_, std::pair<double, double> bounds_ = {1e-5, 1e2}, bool log_scale_ = true,
                         bool fixed_ = false, double fixed_val = -1) :
                name(name_), bounds(bounds_), log_scale(log_scale_), fixed(fixed_), fixed_value(fixed_val) {}

        std::string name;
        std::pair<double, double> bounds;
        bool log_scale;
        bool fixed;
        double fixed_value;
    };

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

    class LFNSSettings {
    public:

        void readSettings();

        std::vector<std::string> getUnfixedParameters() const;

        std::vector<std::pair<double, double> > getBounds(const std::vector<std::string> &param_names) const;

        std::vector<int> getLogParams(const std::vector<std::string> &param_names) const;

        void print(std::ostream &stream);

        std::vector<ParameterSetting> parameters;
        int N = 1000;
        int r = 100;
        bool uniform_prior = true;
        int num_used_trajectories = 1e5;

        std::map<std::string, std::vector<InputData> > input_datas;

        std::string model_file = "";
        std::string output_file = "";
        std::string initial_value_file = "";
        std::string measurement_file = "";
        std::map<std::string, std::string> data_files;
        std::map<std::string, std::string> time_files;
        std::vector<std::string> experiments_for_LFNS;
        std::string previous_log_file = "";
        DENSITY_ESTIMATOR estimator = REJECT_DPGMM;
        MODEL_TYPE model_type = MODEL_TYPE::STOCH;
        double log_termination = -4.6;
        bool use_premature_cancelation = false;
        int print_interval = 1;
        int acceptance_info_print_interval = 100;
        double rejection_quantile_for_density_estimation = 0.001;
        int H = 200;
    };
};

#endif //LFNS_LFNSSETTINGS_H
