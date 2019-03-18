
cat /etc/apache2/mods-enabled/fastcgi.conf 

<IfModule mod_fastcgi.c>
  AddHandler fastcgi-script .fcgi
  #FastCgiWrapper /usr/lib/apache2/suexec
  FastCgiIpcDir /var/lib/apache2/fastcgi
</IfModule>


<IfModule mod_fastcgi.c>
  FastCgiIpcDir /var/lib/apache2/fastcgi
  <Directory /var/www/fastcgi>
    SetHandler fastcgi-script
  </Directory>
  <Location "/foo">
    Require all granted
  </Location>
  FastCgiServer /var/www/fastcgi/fastcgi_test -flush
  AliasMatch "/foo(.*)" "/var/www/fastcgi/fastcgi_test"
</IfModule>


sudo apt update 
sudo apt install apache2 libapache2-mod-fastcgi

a2enmod actions fastcgi alias proxy_fcgi


sudo service apache2 restart

sudo apt install libfcgi-dev

gcc -o hello hello.c -lfcgi
