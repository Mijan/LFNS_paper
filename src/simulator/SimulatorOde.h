//
// Created by jan on 10/10/18.
//

#ifndef LFNS_SIMULATORODE_H
#define LFNS_SIMULATORODE_H

#include <stdio.h>
#include <sundials/sundials_nvector.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <nvector/nvector_serial.h>
#include <cvode/cvode.h>
#include <memory>
#include <vector>
#include <functional>
#include "StoppingCriterion.h"
#include "Simulator.h"

namespace simulator {
    typedef std::function<void(double *dx, const double *state, double t)> RhsFct;
    typedef std::shared_ptr<RhsFct> RhsFct_ptr;

    struct RhsData {
        RhsFct_ptr rhs_fct;
        int num_states;
    };

    struct OdeSettings {
        double min_step_size = 1e-10;
        double rel_tol = 1e-10;
        double abs_tol = 1e-10;
        int max_num_steps = 10000;
        int max_error_fails = 20;
    };

    class SimulatorOde : public Simulator {
    public:
        SimulatorOde(OdeSettings settings, RhsFct_ptr rhs_fct, int num_states);

        SimulatorOde(const SimulatorOde &rhs);

        virtual ~SimulatorOde();

        virtual SimulationFct_ptr getSimulationFct();

        virtual void simulate(double final_time);

        virtual ResetFct_ptr getResetFct();

        void reset(std::vector<double> &state, double &t) override;


    private:
        void *_cvode_mem;
        RhsData _rhs_data;

        std::vector<double> _states;
        double _t;


        OdeSettings _settings;
        N_Vector _current_state_n_vector;
        SUNMatrix _A;
        SUNLinearSolver _LS;


        void _runSolver(double &t, double final_time);

        void _runSingleStep(double &t, double final_time);

        static int cvOdeRhs_static(realtype t, N_Vector y, N_Vector ydot, void *user_data);

        static void
        cvOdeErrorHandling_static(int error_code, const char *module, const char *function, char *msg, void *eh_data);

        static constexpr void (*CVErrHandlerFn)(int error_code, const char *module, const char *function, char *msg,
                                                void *eh_data) = &cvOdeErrorHandling_static;

        static constexpr int (*CVRhsFn)(realtype t, N_Vector y, N_Vector ydot, void *user_data) = &cvOdeRhs_static;
    };

    typedef std::shared_ptr<SimulatorOde> SimulatorOde_ptr;
}


#endif //LFNS_SIMULATORODE_H
