Apache and CGI 
############################################
Jac k Lee 03.20, 2019


versions
=====================
* ubuntu : 2.4.7;  /usr/lib/apache2/modules;
* onboard: 2.4.29; /usr/modules;


fcgi module
======================
* register one handler fcgid_script
   * SetHandler: for the module or directory;
   * AddHandler: add extension name for all scripts, such as php;
* httpd -L: list all configuration items, includes that from newly-configured modules;


Configuration onboard
=================================================
* **fcgid** is not working, only binary is download when configured as fcgid handler;
* configure one URL of cgi, used to test different cgi functions;
* condigure pne URL of apis, mapping all URLs within it into one cgi program;



Add fcgid module: cgid module has been included in default release of Apache;

Enable modules
---------------------
::

   # enable this module, Jack Lee
   LoadModule cgid_module modules/mod_cgid.so

   # LoadModule fastcgi_module modules/mod_fastcgi.so
   # mod_unixd must be enabled when fcgid want to use;
   LoadModule fcgid_module modules/mod_fcgid.so                           

define script alias for both modules
---------------------------------------
::

   <IfModule alias_module>
       # Jack Lee, 03.20, 2019
       ScriptAlias /apis "/var/www/cgi-bin"
   </IfModule>

   # script of fcgid module
   <IfModule fcgid_module>
      scriptAlias "/fapis/" "/var/www/fastcgi/"
   </IfModule>

# directroy of ScriptAliased
---------------------------------------
::

   <Directory "/var/www/cgi-bin">
       AllowOverride None
       # Options None
       # AddHandler fcgid-script .fcgi
       Options +ExecCGI

       Require all granted
   </Directory>


   <Directory "/var/www/fastcgi">                                                       
       AllowOverride None                                                         
       # Options None                                                            
       # AddHandler fcgid-script .fcgi                                       
       Options +ExecCGI                                                      
                                                                                               
       Require all granted                                                    
   </Directory>


Configuration items in mod_cgid in httpd.conf
=================================================
::

		<IfModule cgid_module>
		    #
		    # ScriptSock: On threaded servers, designate the path to the UNIX
		    # socket used to communicate with the CGI daemon of mod_cgid.
		    #
		    #Scriptsock cgisock
		
		   # Jack Lee, added, 03.20, 2019
		   ScriptSock /var/run/cgid.sock
		   ScriptAlias "/apis/" "/var/www/cgi-bin/"
		
		#   Require all granted
		
		   #<Directory /home/*/public_html/cgi-bin/>
		   #    Options ExecCGI SymLinksifOwnerMatch
		   #    SetHandler cgi-script
		   #    AddHandler cgi-script .cgi .pl .py
		   #    Require all granted
		   #    AllowOverride All
		   #</Directory>
		
		</IfModule>
		
		#
		# "/usr/cgi-bin" should be changed to whatever your ScriptAliased CGI directory exists, if you have that configured.
		# this directory must be defined with 'Require all granted', otherwise 
		<Directory "/var/www/cgi-bin">
		    AllowOverride None
		    Options None
		    Require all granted
		</Directory>




mode_fcgi in ubuntu
=====================


# Jack Lee                                                                           
LoadModule fastcgi_module modules/mod_fastcgi.so                                     
                                                                                     
                                                                               
<IfModule fastcgi_module>                                                                      
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


 468 root     httpd
  469 daemon   httpd
  470 daemon   {httpd} fcgi-
  471 daemon   httpd
  473 daemon   /var/www/fastcgi/fastcgi_test
  488 daemon   httpd
  489 daemon   httpd
                                                                                               
                                                                                     
<IfModule unixd_module>                                                              
#                                                                                    
# If you wish httpd to run as a different user or group, you must run                
# httpd as root initially and it will switch.                                        
#                                                                                              
# User/Group: The name (or #number) of the user/group to run httpd as.                         
# It is usually good practice to create a dedicated user and group for                         
# running httpd, as with most system services.                                                 
#                                                                                              
User daemon                                                                    
Group daemon                                                                   
                                                                                     
</IfModule>         


For ap:

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

sudo apt install apache2-dev


Debugging
=========================
::

   httpd -M|grep cgid



<IfModule fcgid_module>                         
  # FastCgiIpcDir /var/lib/apache2/fastcgi
  <Directory /var/www/fastcgi>
    SetHandler fcgid-script
    Options +ExecCGI
    Require all granted
  </Directory>                                                           
  <Location "/foo">                                                  
    Require all granted
  </Location>                   
  # FastCgiServer /var/www/fastcgi/fastcgi_test -flush
  AliasMatch "/foo(.*)" "/var/www/fastcgi/fastcgi_test"
</IfModule>

<IfModule fcgid_module>
  AddHandler fcgid-script .fcgi # you can put whatever extension you want
</IfModule>
