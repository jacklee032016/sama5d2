#!/bin/sh
# pack the build Shared into package and release it
# Jack Lee
#


DATDIR=`echo $1 | sed 's/\/\//\//g'`
PKGDIR="$DATDIR/../pkg"
SRCDIR=`echo $2 | sed 's/\/\//\//g'`

	echo ""
	echo ""
	echo "Install Shared Library and EXEs for "$ARCH
	echo "Build Directris in $PKGDIR"
	
	# VERBOSE=-v
	
	if [ -d $PKGDIR ];
	then
		echo "   remove existed $PKGDIR"
		rm -rf $PKGDIR
	fi
	
	mkdir -p $PKGDIR
	mkdir -p $PKGDIR/usr/bin
	mkdir -p $PKGDIR/sbin
	mkdir -p $PKGDIR/lib
	mkdir -p $PKGDIR/etc/mLab
	mkdir -p $PKGDIR/opt
	mkdir -p $PKGDIR/var/www
	mkdir -p $PKGDIR/usr/share/udhcpc

	# muxConfig.dat: default no configurtion data. It is created with hardware detection
	CONFIG_FILES=" muxSystem.json 
		"
	
	echo ""
	echo "   Copy Configuration into $PKGDIR..."
	#cp $VERBOSE -r $SRCDIR/etc/sys			$PKGDIR/etc
	for cfg in $CONFIG_FILES; do
		cp $VERBOSE -r $SRCDIR/etc/mLab/$cfg $PKGDIR/etc/mLab
	done

	#	cp $VERBOSE -r $SRCDIR/etc/mLab/muxWeb.conf $PKGDIR/etc/mLab/muxWeb.conf

	echo ""

	echo "   Copy Python and driver into $PKGDIR..."
	cp $VERBOSE -r $SRCDIR/opt/* $PKGDIR/opt/
	echo ""
	echo ""

	echo "   Copy PHP into $PKGDIR..."
	cp $VERBOSE -r $SRCDIR/var/www/* $PKGDIR/var/www/


	echo ""
	echo "   Copy Library into $PKGDIR..."
	EXES=`find $DATDIR -name *.so -prune `
	for p in $EXES; do
			f=`basename $p`
			echo "                   $f is copied..."
			cp $p $PKGDIR/lib/
	done

	echo ""
#	echo "   Copy resources for web into $PKGDIR..."
#	cp $VERBOSE -r $SRCDIR/var	$PKGDIR/


	echo ""
	echo "   Copy EXE into $PKGDIR..."
	EXES=`find $DATDIR/usr/bin/ -type f `
	for p in $EXES; do
			f=`basename $p`
			echo "                  $f is copied..."
			cp $VERBOSE $p $PKGDIR/usr/bin
	done

	echo ""
	EXES=`find $DATDIR/sbin/ -type f `
	for p in $EXES; do
			f=`basename $p`
			echo "                  $f is copied..."
			cp $VERBOSE $p $PKGDIR/sbin
	done

	
	echo ""

	PYTHON_TEMP_FILES=`find $PKGDIR -name __pycache__ `
	
	for f in $PYTHON_TEMP_FILES; do
		echo "          Remove $f"
		rm -rf $f
	done


	CVS_FILES=`find $PKGDIR -name \.svn `
	
	for f in $CVS_FILES; do
		echo "          Remove $f"
		rm -rf $f
	done

	cd $PKGDIR
#	tar czf $UPDATE_NAME *
	echo "zip $UPDATE_NAME *"
	zip -r -q $UPDATE_NAME etc lib sbin usr var opt
	mv $UPDATE_NAME $ROOT_DIR

cat << EOF
======================================================================
Shared Library and EXEs for $ARCH are installed 
     _________________________________________________________
     *****  $UPDATE_NAME  *****
     ---------------------------------------------------------
              has been build in $BUILDTIME!
Please Check it with Your Board!
======================================================================
EOF

exit 0
