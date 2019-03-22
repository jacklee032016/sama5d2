
lzj@lzj-Ubuntu:~/yocto/sama5d2/fcgi/mod_fastcgi$ make

/usr/share/apr-1.0/build/libtool --no-silent --mode=compile x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security     -DLINUX -D_REENTRANT -D_GNU_SOURCE   -D_FORTIFY_SOURCE=2    -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -prefer-pic -c mod_fastcgi.c && touch mod_fastcgi.slo
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c mod_fastcgi.c  -fPIC -DPIC -o .libs/mod_fastcgi.o
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c mod_fastcgi.c  -fPIC -DPIC -o mod_fastcgi.o >/dev/null 2>&1
/usr/share/apr-1.0/build/libtool --no-silent --mode=compile x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security     -DLINUX -D_REENTRANT -D_GNU_SOURCE   -D_FORTIFY_SOURCE=2    -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -prefer-pic -c fcgi_pm.c && touch fcgi_pm.slo


libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_pm.c  -fPIC -DPIC -o .libs/fcgi_pm.o
fcgi_pm.c: In function 'spawn_fs_process':
fcgi_pm.c:439:15: warning: ignoring return value of 'setuid', declared with attribute warn_unused_result [-Wunused-result]
         setuid(ap_user_id);
               ^
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_pm.c  -fPIC -DPIC -o fcgi_pm.o >/dev/null 2>&1


/usr/share/apr-1.0/build/libtool --no-silent --mode=compile x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security     -DLINUX -D_REENTRANT -D_GNU_SOURCE   -D_FORTIFY_SOURCE=2    -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -prefer-pic -c fcgi_util.c && touch fcgi_util.slo

libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_util.c  -fPIC -DPIC -o .libs/fcgi_util.o
libtool: compile:  x86_64-linux-gnu-gcc 
	-std=gnu99 -pthread -pipe -g -O2 
	-fstack-protector 
	--param=ssp-buffer-size=4 
	-Wformat -Werror=format-security 
	-DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 
	
	-I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_util.c  -fPIC -DPIC -o fcgi_util.o >/dev/null 2>&1


/usr/share/apr-1.0/build/libtool --no-silent --mode=compile x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security     -DLINUX -D_REENTRANT -D_GNU_SOURCE   -D_FORTIFY_SOURCE=2    -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -prefer-pic -c fcgi_protocol.c && touch fcgi_protocol.slo
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_protocol.c  -fPIC -DPIC -o .libs/fcgi_protocol.o
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_protocol.c  -fPIC -DPIC -o fcgi_protocol.o >/dev/null 2>&1



/usr/share/apr-1.0/build/libtool --no-silent --mode=compile x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security     -DLINUX -D_REENTRANT -D_GNU_SOURCE   -D_FORTIFY_SOURCE=2    -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -prefer-pic -c fcgi_buf.c && touch fcgi_buf.slo
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_buf.c  -fPIC -DPIC -o .libs/fcgi_buf.o
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_buf.c  -fPIC -DPIC -o fcgi_buf.o >/dev/null 2>&1



/usr/share/apr-1.0/build/libtool --no-silent --mode=compile x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security     -DLINUX -D_REENTRANT -D_GNU_SOURCE   -D_FORTIFY_SOURCE=2    -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -prefer-pic -c fcgi_config.c && touch fcgi_config.slo
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_config.c  -fPIC -DPIC -o .libs/fcgi_config.o
libtool: compile:  x86_64-linux-gnu-gcc -std=gnu99 -pthread -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -I/usr/include/apache2 -I. -I/usr/include/apr-1.0 -I/usr/include -c fcgi_config.c  -fPIC -DPIC -o fcgi_config.o >/dev/null 2>&1



/usr/share/apr-1.0/build/libtool --no-silent --mode=link x86_64-linux-gnu-gcc -std=gnu99  -pthread   -pipe -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security      -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now     -o mod_fastcgi.la -rpath /usr/lib/apache2/modules -module -avoid-version mod_fastcgi.lo fcgi_pm.lo fcgi_util.lo fcgi_protocol.lo fcgi_buf.lo fcgi_config.lo


libtool: link: x86_64-linux-gnu-gcc -shared  -fPIC -DPIC  
	.libs/mod_fastcgi.o 
	.libs/fcgi_pm.o 
	.libs/fcgi_util.o 
	.libs/fcgi_protocol.o 
	.libs/fcgi_buf.o 
	.libs/fcgi_config.o    
	
	-pthread -O2 -Wl,--as-needed -Wl,-Bsymbolic-functions -Wl,-z -Wl,relro -Wl,-z -Wl,now   -pthread -Wl,-soname -Wl,mod_fastcgi.so -o .libs/mod_fastcgi.so
libtool: link: ar cru .libs/mod_fastcgi.a  mod_fastcgi.o fcgi_pm.o fcgi_util.o fcgi_protocol.o fcgi_buf.o fcgi_config.o
libtool: link: ranlib .libs/mod_fastcgi.a
libtool: link: ( cd ".libs" && rm -f "mod_fastcgi.la" && ln -s "../mod_fastcgi.la" "mod_fastcgi.la" )
