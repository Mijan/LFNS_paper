
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

For example, assuming `$HOME/libs/include` contains a folder `eigen` which contains the file
`signature_of_eigen3_matrix_library` (this is included in the Eigen library),
you can write

    export EIGEN3_ROOT=$HOME/libs/include

Installation
--------------------
To install the programm you need to run the cmake script followd by a make. Assuming the LNSv2 folder with the CMakeList.txt
is in the folder /home/user/DP-GMM, the default build routine would look like this:
	
	cd /home/user/DP-GMM
	cmake . 
	make

The above commands would create the the library libDPGMM.a in the folder /home/user/LNSv2
If you additionally run the 

	make install
	
command, the libDPGMM.a file will be installed in /usr/local/lib and the corresponding header files will be installed in 
/usr/local/include. If you would like to change the default install directory 
/usr/local/, you can specified an installation path with:  
	
	cmake /location/of/CMakeList.txt -DCMAKE_INSTALL_PREFIX=path/to/installation


