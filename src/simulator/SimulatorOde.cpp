//
// Created by jan on 10/10/18.
//

#include <bits/types/FILE.h>
#include <cvode/cvode_direct.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include "SimulatorOde.h"

namespace simulator {
    using namespace std::placeholders;

    SimulatorOde::SimulatorOde(OdeSettings settings, RhsFct_ptr rhs_fct, int num_states)
            : Simulator(), _cvode_mem(nullptr), _rhs_data(), _settings(settings),
              _current_state_n_vector(N_VMake_Serial(0, nullptr)), _A(SUNDenseMatrix(1, 1)),
              _LS(SUNDenseLinearSolver(_current_state_n_vector, _A)), _sim_state(nullptr), _t(nullptr) {

        _cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);

        _rhs_data.rhs_fct = rhs_fct.get();
        _rhs_data.num_states = num_states;
    }

    SimulatorOde::~SimulatorOde() {
        CVodeFree(&_cvode_mem);
        N_VDestroy_Serial(_current_state_n_vector);
        SUNMatDestroy(_A);
        SUNLinSolFree(_LS);
    }


    void SimulatorOde::initialize(std::vector<double> &state, double &t) {
        _rhs_data.num_states = state.size();
        _sim_state = &state;
        _t = &t;

        N_VDestroy_Serial(_current_state_n_vector);
        _current_state_n_vector = N_VMake_Serial(_rhs_data.num_states, state.data());
        if (CVodeInit(_cvode_mem, CVRhsFn, 0, _current_state_n_vector) != CV_SUCCESS) {
            throw std::runtime_error("Initializing ODE solver went wrong!");
        }


        if (CVodeSetUserData(_cvode_mem, &_rhs_data) != CV_SUCCESS) {
            throw std::runtime_error("User data could not be set!");
        }

        SUNMatDestroy(_A);
        _A = SUNDenseMatrix(_rhs_data.num_states, _rhs_data.num_states);

        SUNLinSolFree(_LS);
        _LS = SUNDenseLinearSolver(_current_state_n_vector, _A);

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

    void SimulatorOde::setRootFunction(RootFct_ptr root_fct) {
        Simulator::setRootFunction(root_fct);
        if (CVodeRootInit(_cvode_mem, 1, CVRootFn) != CV_SUCCESS) {
            throw std::runtime_error("Failed to _initialize root function!");
        }
    }

    void SimulatorOde::reset(std::vector<double> &state, double &t) {

        if (&state != _sim_state || &t != _t) {
            N_VSetArrayPointer(state.data(), _current_state_n_vector);
            _sim_state = &state;
            _t = &t;
        }
        if (CVodeReInit(_cvode_mem, t, _current_state_n_vector) != CV_SUCCESS) {
            throw std::runtime_error("Reinitializing ODE solver went wrong!");
        }
    }

    void
    SimulatorOde::simulate(std::vector<double> &state, double &t, double final_t,
                           const std::vector<double> &theta) {
        reset(state, t);
        continueSimulate(final_t, theta);
    }

    void SimulatorOde::continueSimulate(double final_time, const std::vector<double> &theta) {
        _rhs_data.theta = &theta;
        _runSolver(*_sim_state, *_t, final_time);
    }


    SimulationFct_ptr SimulatorOde::getSimulationFct() {
        return std::make_shared<SimulationFct>(std::bind(&SimulatorOde::simulate, this, _1, _2, _3, _4));
    }

    void SimulatorOde::_runSolver(std::vector<double> &state, double &t, double final_time) {

        bool run_next_step = final_time > t + _settings.min_step_size;
        while (run_next_step) {
            _runSingleStep(state, t, final_time);
            if (_root_found) { t += _settings.min_step_size; }
            if (_stopping_criterions.processStopped()) { return; }

            run_next_step = final_time > t + _settings.min_step_size;
        }
    }


    void SimulatorOde::_runSingleStep(std::vector<double> &state, double &t, double final_time) {
        if (_root_found) { CVodeReInit(_cvode_mem, t, _current_state_n_vector); }
        int flag = CVode(_cvode_mem, final_time, _current_state_n_vector, &t, CV_NORMAL);
        if (flag == CV_ROOT_RETURN) {
            _root_found = true;
        } else {
            _root_found = false;
        }
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

            throw std::runtime_error(os.str());
        }
    }


    int SimulatorOde::cvOdeRhs_static(realtype t, N_Vector y, N_Vector ydot, void *user_data) {
        RhsData *rhs_data = (RhsData *) user_data;
        const std::vector<double> *theta_ptr = rhs_data->theta;
        (*rhs_data->rhs_fct)(NV_DATA_S(ydot), NV_DATA_S(y), t, *theta_ptr);

        return 0;
    }

    int SimulatorOde::cvOdeRoot_static(realtype t, N_Vector y, realtype *gout, void *user_data) {
        RhsData *rhs_data = (RhsData *) user_data;

        if (rhs_data->root_fct_ptr) {
            std::vector<double> state(NV_DATA_S(y), NV_DATA_S(y) + rhs_data->num_states);
            *gout = (*rhs_data->root_fct_ptr)(state, t);
        }

        return 0;
    }

    void
    SimulatorOde::cvOdeErrorHandling_static(int error_code, const char *module, const char *function, char *msg,
                                            void *eh_data) {}

}