#!/bin/bash 
#
# Set up we find the autotools for bootstrapping
#
set -x
export bmode=""
if [ `uname -m` = "x86_64" -o `uname -m` = " x86-64" ]; then
    LIBDIR="lib64"
    ALTLIBDIR="lib"
    echo "UNAME IS X86_64 FAMILY: LIBDIR=$LIBDIR"
    export LIBDIR="lib64"
elif [ `uname -m` = "ppc64" ]; then
   if [ $CBTF_PPC64_BITMODE_32 ]; then
    LIBDIR="lib"
    ALTLIBDIR="lib64"
    echo "UNAME IS PPC64 FAMILY, but 32 bitmode: LIBDIR=$LIBDIR"
    export LIBDIR="lib"
    export bmode="--with-ppc64-bitmod=32"
   else
    LIBDIR="lib64"
    ALTLIBDIR="lib"
    echo "UNAME IS PPC64 FAMILY, with 64 bitmode: LIBDIR=$LIBDIR"
    export LIBDIR="lib64"
    export CFLAGS=" -m64 $CFLAGS "
    export CXXFLAGS=" -m64 $CXXFLAGS "
    export CPPFLAGS=" -m64 $CPPFLAGS "
    export bmode="--with-ppc64-bitmod=64"
   fi
elif [ `uname -m` = "ppc" ]; then
    LIBDIR="lib"
    ALTLIBDIR="lib64"
    echo "UNAME IS PPC FAMILY: LIBDIR=$LIBDIR"
    export LIBDIR="lib"
    export bmode="--with-ppc64-bitmod=32"
else
    LIBDIR="lib"
    ALTLIBDIR="lib64"
    export LIBDIR="lib"
    echo "UNAME IS X86 FAMILY: LIBDIR=$LIBDIR"
fi

sys=`uname -n `
export MACHINE=$sys
echo ""
echo '    machine: ' $sys

export CC=`which gcc`
export CXX=`which c++`
export CPLUSPLUS=`which c++`

if [ -z "$CBTF_MPI_MPICH2" ]; then
 export CBTF_MPI_MPICH2=/usr
fi

if [ -z "$CBTF_MPI_MVAPICH" ]; then
 export CBTF_MPI_MVAPICH=/usr
fi

if [ -z "$CBTF_MPI_MVAPICH2" ]; then
 export CBTF_MPI_MVAPICH2=/usr
fi

if [ -z "$CBTF_MPI_MPT" ]; then
 export CBTF_MPI_MPT=/usr
fi

if [ -z "$CBTF_MPI_OPENMPI" ]; then
 export CBTF_MPI_OPENMPI=/usr
fi


# you may want to load the latest autotools for the bootstraps to succeed.
echo "-------------------------------------------------------------"
echo "-- START BUILDING CBTF  -------------------------------------"
echo "-------------------------------------------------------------"


# UNCOMMENT THIS WHEN READY TO BUILD, IT WORKED MARCH 11 2013 on yellowstone
echo "-------------------------------------------------------------"
echo "-- BUILDING cbtf-lanl ---------------------------------------"
echo "-------------------------------------------------------------"
#./bootstrap

echo "-- CONFIGURING cbtf-lanl  -----------------------------------"

if [ -z "$CBTF_TARGET_ARCH" ];
then
  ./configure --prefix=$CBTF_PREFIX $bmode --with-cbtf=$CBTF_PREFIX --with-cbtf-xml=$CBTF_PREFIX --with-cbtf-mrnet=$CBTF_PREFIX --with-cbtf-core=$CBTF_PREFIX --with-mrnet=$CBTF_MRNET_ROOT --with-boost=$CBTF_BOOST_ROOT --with-boost-libdir=$CBTF_BOOST_ROOT_LIB --with-libxerces-c-prefix=$CBTF_XERCESC_ROOT 
else
  ./configure --prefix=$CBTF_PREFIX $bmode --with-cbtf=$CBTF_PREFIX --with-cbtf-xml=$CBTF_PREFIX --with-cbtf-mrnet=$CBTF_PREFIX --with-cbtf-core=$CBTF_PREFIX --with-mrnet=$CBTF_MRNET_ROOT --with-boost=$CBTF_BOOST_ROOT --with-boost-libdir=$CBTF_BOOST_ROOT_LIB --with-libxerces-c-prefix=$CBTF_XERCESC_ROOT --with-target-os=$CBTF_TARGET_ARCH
fi
echo "-- UNINSTALLING cbtf-lanl -------------------------------------"
make uninstall 
echo "-- CLEANING cbtf-lanl -------------------------------------"
make clean 
echo "-- MAKING cbtf-lanl -------------------------------------"
make 
echo "-- INSTALLING cbtf-lanl -------------------------------------"
make install


if [ -f memTool/memtool.xml -a -f psTool/pstool.xml ]; then
   echo "CBTF CONTRIB/LANL BUILT SUCCESSFULLY."
else
   echo "CBTF CONTRIB/LANL FAILED TO BUILD - TERMINATING BUILD SCRIPT.  Please check for errors."
#   exit
fi

echo "-------------------------------------------------------------"
echo "-- END OF BUILDING CBTF  ------------------------------------"
echo "-------------------------------------------------------------"
