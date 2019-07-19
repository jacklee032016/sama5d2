Common readme for all modules
##################################
Feb,11, 2019

Colorized output of gcc
============================================

Update gcc to version 4.9 or later
--------------------------------------------
in 167~14.04.1-Ubuntu 

::

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install gcc-4.9
	
	change symbol link /usr/bin/gcc to /usr/bin/gcc-4.9 from /usr/bin/gcc-4.8
	

make process
--------------------------------------------
::

    GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'
	export GCC_COLORS


    add ``-fdiagnostics-color=always`` in flags, optional



man local file
==============================

::

    man DIRECTORY/MAN_FILE

