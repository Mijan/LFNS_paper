
DP-GMM Documentation
====================

General
----------
  * Author: Jan Mikelson <jan.mikelson@bsse.ethz.ch>

This programm performs density estimation based on a Dirichle Process Gaussian Mixture Model as described in 
Dilan G ̈or ̈ur and Carl Edward Rasmussen. Dirichlet process gaussian mixture models: Choice of the base distribution.
 Journal of Computer Science and Technology,
25(4):653–664, 2010.


Dependencies
--------------------
  * [Boost](http://www.boost.org): minimal version: 1.53
  * [Eigen3](http://eigen.tuxfamily.org/index.php) matrix library


INSTALLATION
--------------------
Preparation
--------------------

Before attempting to compile, you must ensure that the required dependencies can be found by CMake.
If you have everything installed in standard locations, the CMake find scripts should locate everything.
If not, you can specify the environment variables `EIGEN3_ROOT` and `BOOST_ROOT`.

For example, assuming '/usr/local/include` contains a folder `Eigen` and the file
`signature_of_eigen3_matrix_library`,you can write

    export EIGEN3_ROOT=/usr/local/include

Installation
--------------------
To install the program you need to run the cmake script followed by a make.
	
	mkdir build
	cd build
	cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local/ ../
	make
	make install

The above commands would create the the library libDPGMM.so in the folder build and install them along with the
necessary header files and the executable "estimate_density" into the folder "/usr/local/lib", "/usr/local/include" and "/usr/local/bin" respectively.


