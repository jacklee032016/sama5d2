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
	
	VERBOSE=-v
	
	mkdir -p $PKGDIR
	mkdir -p $PKGDIR/usr/bin
	mkdir -p $PKGDIR/sbin
	mkdir -p $PKGDIR/lib
	mkdir -p $PKGDIR/etc/apache2
	mkdir -p $PKGDIR/etc/avahi
	mkdir -p $PKGDIR/etc/mLab
	mkdir -p $PKGDIR/etc/network
	mkdir -p $PKGDIR/etc/rc5.d
	mkdir -p $PKGDIR/etc/sys
	mkdir -p $PKGDIR/var/www/apis
	mkdir -p $PKGDIR/var/www/cgi-bin
	mkdir -p $PKGDIR/opt

	# muxConfig.dat: default no configurtion data. It is created with hardware detection
	CONFIG_FILES="muxLab.png muxMain.conf  
		muxSystem.json muxPtpd.cfg muxPhcd.cfg
		"
	
	echo ""
	echo "   Copy Configuration into $PKGDIR..."
	cp $VERBOSE -r $SRCDIR/etc/mime.types	$PKGDIR/etc
	cp $VERBOSE $SRCDIR/etc/fw_env.config	$PKGDIR/etc/
	cp $VERBOSE -r $SRCDIR/etc/avahi		$PKGDIR/etc
	cp $VERBOSE -r $SRCDIR/etc/network		$PKGDIR/etc
	cp $VERBOSE -r $SRCDIR/etc/rc5.d		$PKGDIR/etc
	cp $VERBOSE -r $SRCDIR/etc/sys			$PKGDIR/etc
	for cfg in $CONFIG_FILES; do
		cp $VERBOSE -r $SRCDIR/etc/mLab/$cfg $PKGDIR/etc/mLab
	done
	cp $VERBOSE $SRCDIR/etc/apache2/httpd.conf	$PKGDIR/etc/apache2/

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
#	echo "   Copy CGI into $PKGDIR..."
	EXES=`find $DATDIR/var/www/apis/ -type f `
	for p in $EXES; do
			f=`basename $p`
			echo "                  $f is copied..."
			cp $VERBOSE $p $PKGDIR/var/www/apis/
	done

	
		echo ""
#	echo "   Copy CGI into $PKGDIR..."
#	EXES=`find $DATDIR/var/www/cgi-bin/ -type f `
#	for p in $EXES; do
#			f=`basename $p`
#			echo "                  $f is copied..."
#			cp $VERBOSE $p $PKGDIR/var/www/cgi-bin/
#	done


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
#	tar czf $RELEASES_NAME *
	echo "zip $RELEASES_NAME *"
	zip -r $RELEASES_NAME etc lib sbin usr var opt
	mv $RELEASES_NAME $ROOT_DIR

cat << EOF
======================================================================
Shared Library and EXEs for $ARCH are installed 
     _________________________________________________________
     *****  $RELEASES_NAME  *****
     ---------------------------------------------------------
              has been build in $BUILDTIME!
Please Check it with Your Board!
======================================================================
EOF

exit 0
