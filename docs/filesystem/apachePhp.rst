Apache and php7 
################################
April 9th, 2019

version: Apache/2.4.29
php7:


Configure fcgi in apache2
================================
Configure mod_proxy_fcgi module:

:: 

    ProxyPass "/api/" "fcgi://localhost:5000/"

Start standard fast CGI program:
::

    fcgistarter -c /var/www/fastcgi_test -p 5000: 

After started successfully, /var/www/fastcgi_test is running daemon to service http request in port 5000; and this port is used;


Notes:
* **fcgistarter** can't start flup python cgi program because it is in wsgi standard;
* **fcgi://** is supported by proxy_fcgi module; **http://** is supported by proxy_http module;



Configuration of Yocto Project
================================
In `local.conf`:

::

   # recipe php is in OE/meta-oe/recipes-devtools/
   # `php-cli` is PACKAGES in php (refers to php.inc)
   $ PHP and CLI PHP support
   IMAGE_INSTALL_append = " php php-cli "

Removing `php-cli` only remove `/usr/bin/php`, not '/usr/bin/phpdbg`;

::

   # `apache2` is PACKAGE in php, not the recipe of apache2
   PACKAGECONFIG_append_pn-php = " apache2"

   # After apache2 is used, php-modphp (eg. module of php in apache) is added
   IMAGE_INSTALL_append += " php-modphp "


Results
-----------------
* /etc/apaches/modules.d/70_mod_php7.conf
* MODULES/libphp7.so, 12.615MB

* root path of php:

   DocumentRoot "/usr/share/apache2/htdocs"
   <Directory "/usr/share/apache2/htdocs">


Questions
-----------------
#. MariaDB/myQSql, sqlite client should be disabled to save space;
*. /usr/bin/phpdbg should be removed from image to save 12MB;





Debug
===================   

* **messages**

    PHP Fatal error:  Unknown: Failed opening required '/usr/share/apache2/htdocs/index.php' (include_path='.:/usr/lib/php7/php') in Unknown on line 0

Fixing
-------------


   chmod -R 755 *(*.php)

