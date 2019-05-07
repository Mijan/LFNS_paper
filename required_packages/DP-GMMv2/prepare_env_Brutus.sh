#prepares the evironment for executing and installing cppStochEst on the Brutus cluster
module load boost/1.54.0
export CPPFLAGS="${CPPFLAGS} -I${BOOST_INCLUDEDIR}"
export LDFLAGS="-L${BOOST_LIBRARYDIR} ${LDFLAGS}"
