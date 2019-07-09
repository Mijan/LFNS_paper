//
// Created by jan on 10/10/18.
//

#include <memory>
#include <cvode/cvode_direct.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include "SimulatorOde.h"
#include "../base/MathUtils.h"
#include "SimulatorExceptions.h"

namespace simulator {
    using namespace std::placeholders;

    SimulatorOde::SimulatorOde(OdeSettings settings, RhsFct_ptr rhs_fct, int num_states)
            : Simulator(), _cvode_mem(nullptr), _rhs_data(), _states(num_states, 0.0), _t(0.0), _settings(settings),
              _current_state_n_vector(N_VMake_Serial(num_states, _states.data())),
              _A(SUNDenseMatrix(num_states, num_states)),
              _LS(SUNDenseLinearSolver(_current_state_n_vector, _A)) {

        _t_ptr = &_t;
        _states_ptr = &_states;
        _cvode_mem = CVodeCreate(CV_BDF);

        _rhs_data.rhs_fct = rhs_fct;
        _rhs_data.num_states = num_states;
        if (CVodeInit(_cvode_mem, CVRhsFn, 0, _current_state_n_vector) != CV_SUCCESS) {
            throw std::runtime_error("Initializing ODE solver went wrong!");
        }


        if (CVodeSetUserData(_cvode_mem, &_rhs_data) != CV_SUCCESS) {
            throw std::runtime_error("User data could not be set!");
        }

        if (CVDlsSetLinearSolver(_cvode_mem, _LS, _A) != CV_SUCCESS) {
            throw std::runtime_error("ODE Solver could not be set!");
        }

        if (CVodeSetErrHandlerFn(_cvode_mem, CVErrHandlerFn, nullptr) != CV_SUCCESS) {
            throw std::runtime_error("Error handling could not be set!");
        }
        if (CVodeSStolerances(_cvode_mem, _settings.rel_tol, _settings.abs_tol) != CV_SUCCESS) {
            throw std::runtime_error("Setting tolerances for ODE solver went wrong!");
        }

        if (CVodeSetMinStep(_cvode_mem, _settings.min_step_size) != CV_SUCCESS) {
            throw std::runtime_error("Stepsize could not be set");
        }
        if (CVodeSetMaxErrTestFails(_cvode_mem, _settings.max_error_fails) != CV_SUCCESS) {
            throw std::runtime_error("Maximum error test fails number could not be set!");
        }
        if (CVodeSetMaxNumSteps(_cvode_mem, _settings.max_num_steps) != CV_SUCCESS) {
            throw std::runtime_error("Maximum number steps could not be set!");
        }
    }


    SimulatorOde::SimulatorOde(const SimulatorOde &rhs)
            : Simulator(), _cvode_mem(nullptr), _rhs_data(), _states(rhs._states.size(), 0.0), _t(0.0),
              _settings(rhs._settings),
              _current_state_n_vector(N_VMake_Serial(rhs._states.size(), _states.data())),
              _A(SUNDenseMatrix(rhs._states.size(), rhs._states.size())),
              _LS(SUNDenseLinearSolver(_current_state_n_vector, _A)) {

        _cvode_mem = CVodeCreate(CV_BDF);
        _t_ptr = &_t;
        _states_ptr = &_states;
        _rhs_data.rhs_fct = rhs._rhs_data.rhs_fct;
        _rhs_data.num_states = rhs._rhs_data.num_states;
        if (CVodeInit(_cvode_mem, CVRhsFn, 0, _current_state_n_vector) != CV_SUCCESS) {
            throw std::runtime_error("Initializing ODE solver went wrong!");
        }


        if (CVodeSetUserData(_cvode_mem, &_rhs_data) != CV_SUCCESS) {
            throw std::runtime_error("User data could not be set!");
        }

        if (CVDlsSetLinearSolver(_cvode_mem, _LS, _A) != CV_SUCCESS) {
            throw std::runtime_error("ODE Solver could not be set!");
        }

        if (CVodeSetErrHandlerFn(_cvode_mem, CVErrHandlerFn, nullptr) != CV_SUCCESS) {
            throw std::runtime_error("Error handling could not be set!");
        }
        if (CVodeSStolerances(_cvode_mem, _settings.rel_tol, _settings.abs_tol) != CV_SUCCESS) {
            throw std::runtime_error("Setting tolerances for ODE solver went wrong!");
        }

        if (CVodeSetMinStep(_cvode_mem, _settings.min_step_size) != CV_SUCCESS) {
            throw std::runtime_error("Stepsize could not be set");
        }
        if (CVodeSetMaxErrTestFails(_cvode_mem, _settings.max_error_fails) != CV_SUCCESS) {
            throw std::runtime_error("Maximum error test fails number could not be set!");
        }
        if (CVodeSetMaxNumSteps(_cvode_mem, _settings.max_num_steps) != CV_SUCCESS) {
            throw std::runtime_error("Maximum number steps could not be set!");
        }
    }

    SimulatorOde::~SimulatorOde() {
        CVodeFree(&_cvode_mem);
        N_VDestroy_Serial(_current_state_n_vector);
        SUNMatDestroy(_A);
        SUNLinSolFree(_LS);
    }

    ResetFct_ptr SimulatorOde::getResetFct() {
        return std::make_shared<ResetFct>(std::bind(&SimulatorOde::reset, this, _1, _2));
    }

    void SimulatorOde::reset(std::vector<double> &state, double &t) {
        Simulator::reset(state, t);

        N_VSetArrayPointer(state.data(), _current_state_n_vector);

        if (CVodeReInit(_cvode_mem, t, _current_state_n_vector) != CV_SUCCESS) {
            throw std::runtime_error("Reinitializing ODE solver went wrong!");
        }
    }

    SimulationFct_ptr SimulatorOde::getSimulationFct() {
        return std::make_shared<SimulationFct>(std::bind(&SimulatorOde::simulate, this, _1));
    }

    void SimulatorOde::simulate(double final_time) { _runSolver(*_t_ptr, final_time); }

    void SimulatorOde::_runSolver(double &t, double final_time) {

        bool run_next_step = final_time > t + _settings.min_step_size;
        double T;
        while (run_next_step) {
            bool discon_found = false;
            if (_discont_it != _discont_times.end() && *_discont_it <= final_time) {
                T = *_discont_it++;
                T -= _settings.min_step_size;
                discon_found = true;
            } else { T = final_time; }

            _runSingleStep(t, T);
            if (discon_found) {
                t += 2* _settings.min_step_size;
                if (CVodeReInit(_cvode_mem, t, _current_state_n_vector) != CV_SUCCESS) {
                    throw std::runtime_error("Reinitializing ODE solver went wrong!");
                }
            }

            if (_stopping_criterions.processStopped()) { return; }
            run_next_step = final_time > t + _settings.min_step_size;
        }
    }


    void SimulatorOde::_runSingleStep(double &t, double final_time) {
        int flag = CVode(_cvode_mem, final_time, _current_state_n_vector, &t, CV_NORMAL);
        if (flag != CV_SUCCESS && flag != CV_ROOT_RETURN) {
            std::stringstream os;
            os << "Error while solving Ode; t = " << t << ", final time " << final_time << std::endl;
            os << "Number species: \t" << NV_LENGTH_S(_current_state_n_vector) << std::endl;
            os << "Current state : \t";
            for (std::size_t i = 0; i < NV_LENGTH_S(_current_state_n_vector); i++) {
                os << NV_Ith_S(_current_state_n_vector, i) << " ";
            }
            os << std::endl;
            os << "CVode Flag is :\t" << flag << std::endl;

            long n_steps, num_f_evals, n_lin_setups, net_fails;
            int q_last, q_cur;
            double h_in_used, h_last, h_curr, t_cur;

            CVodeGetIntegratorStats(_cvode_mem, &n_steps, &num_f_evals,
                                    &n_lin_setups, &net_fails, &q_last, &q_cur,
                                    &h_in_used, &h_last, &h_curr, &t_cur);

            os << "Number of internal steps:\t" << n_steps << std::endl;
            os << "Number of function evaluations: \t" << num_f_evals << std::endl;
            os << "Number of calls for the linear setup function :\t" << n_lin_setups << std::endl;
            os << "Number of failed test:\t" << net_fails << std::endl;
            os << "Method order used for the last internal step:\t" << q_last << std::endl;
            os << "Method order used for the current internal step:\t" << q_cur << std::endl;
            os << "Actual value of initial step size:\t" << h_in_used << std::endl;
            os << "Last step size:\t" << h_last << std::endl;
            os << "Current step size:\t" << h_curr << std::endl;
            os << "Current internal time:\t" << t_cur;

            SimulationAborted exception(os.str(), t);

            throw exception;
        }
    }


    int SimulatorOde::cvOdeRhs_static(realtype t, N_Vector y, N_Vector ydot, void *user_data) {
        RhsData *rhs_data = (RhsData *) user_data;
        (*rhs_data->rhs_fct)(NV_DATA_S(ydot), NV_DATA_S(y), t);
        return 0;
    }

    void
    SimulatorOde::cvOdeErrorHandling_static(int error_code, const char *module, const char *function, char *msg,
                                            void *eh_data) {}

}