/*
   Rehuel: a simple C++ library for solving ODEs


   Copyright 2017, Stefan Paquay (stefanpaquay@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

============================================================================= */

/**
   \file brusselator.cpp

   \brief This file contains code to solve the Brusselator as example.

   Compile as g++ -O3 -I../ -fopenmp -march=native -lrehuel -larmadillo brusselator.cpp -o brusselator
*/

#include <iostream>

#include "rehuel.hpp"

/*
  ODEs are implemented through functors. They need to implement two functions:
  fun and jac.
  The first calculates the RHS of the ODE, the second the Jacobi matrix.
  It also needs a typedef, jac_type, to indicate whether or not the Jacobi
  matrix is sparse or not. Currently only non-sparse is supported.
*/
struct brusselator {
	typedef arma::mat jac_type;

	arma::vec fun( double t, const arma::vec &Y ) const
	{
		return { a + Y[0]*Y[0]*Y[1] - b*Y[0] - Y[0],
		         b*Y[0] - Y[0]*Y[0]*Y[1] };
	}

	arma::mat jac( double t, const arma::vec &Y ) const
	{
		return { {2*Y[0]*Y[1] - b - 1.0, Y[0]*Y[0]},
		         {b - 2*Y[0]*Y[1], -Y[0]*Y[0] } };
	}

	double a,b;
};



int solve_irk(const std::string &method_str, brusselator &brus,
              arma::vec Y0, double t0, double t1)
{
	newton::options newton_opts;
	irk::solver_options solver_opts = irk::default_solver_options();
	solver_opts.newton_opts = &newton_opts;
	int method = irk::RADAU_IIA_32;
	method = irk::name_to_method(method_str);
	irk::rk_output sol = irk::odeint( brus, t0, t1, Y0,
	                                  solver_opts, method );

	std::cerr << "Solved ODE with " << sol.t_vals.size() << " time steps in "
	          << sol.elapsed_time / 1000.0 << " seconds.\n";
	for( std::size_t i = 0; i < sol.t_vals.size(); ++i ){
		std::cout << sol.t_vals[i];
		for( std::size_t j = 0; j < sol.y_vals[i].size(); ++j ){
			std::cout << " " << sol.y_vals[i][j];
		}
		std::cout << "\n";
	}
	return 0;
}



int solve_erk(const std::string &method_str, brusselator &brus,
              arma::vec Y0, double t0, double t1)
{
	erk::solver_options solver_opts = erk::default_solver_options();
	int method = erk::CASH_KARP_54;
	method = erk::name_to_method(method_str);
	erk::rk_output sol = erk::odeint( brus, t0, t1, Y0,
	                                  solver_opts, method );

	std::cerr << "Solved ODE with " << sol.t_vals.size() << " time steps in "
	          << sol.elapsed_time / 1000.0 << " seconds.\n";
	for( std::size_t i = 0; i < sol.t_vals.size(); ++i ){
		std::cout << sol.t_vals[i];
		for( std::size_t j = 0; j < sol.y_vals[i].size(); ++j ){
			std::cout << " " << sol.y_vals[i][j];
		}
		std::cout << "\n";
	}
	return 0;
}



int main( int argc, char **argv )
{
	double t0 = 0.0;
	double t1 = 1e4;
	arma::vec Y0 = { 2.0, 2.0 };

	brusselator brus;
	brus.a = 2.0;
	brus.b = brus.a*brus.a + 2.5; // Just past Hopf bifurcation.
	std::string method_name = "RADAU_IIA_53";
	if (argc > 1) {
		method_name = argv[1];
	}

	int method = irk::name_to_method(method_name);
	if (method) {
		// method = 0 indicates that this method is not an irk.
		std::cerr << "method = " << method << " for name "
		          << method_name << "\n";
		return solve_irk(method_name, brus, Y0, t0, t1);
	}

	method = erk::name_to_method(method_name);
	if (method) {
		return solve_erk(method_name, brus, Y0, t0, t1);
	}
	
	return 0;
}
