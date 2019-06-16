Work Logs for File System
################################
April 17th, 2019

06.16, 2019
==============================
Add scripts of runlevel-5 in /etc/rc5.d/S95Led
   * scripts can be symbol link to /etc/init.d/;
   * script must be in executing previledge;

05.16, 2019
==============================
Add i2c-tools
------------------------------
Recipe position: poky/meta/recipes-devtools/i2c-tools/i2c-tools_4.0.bb

Add into image: IMAGE_INSTALL_append += " i2c-tools "  in local.conf


Add python3-six
------------------------------
Module six is needed by argparser in flask;

Recipe position: poky/meta/recipes-devtools/python/python3-six_1.11.0.bb

Add into image: CORE_IMAGE_EXTRA_INSTALL = "python3-core python3-pip python3-flask python3-flask-restful python3-six"  in local.conf

Add fw-printenv
------------------------------
fw-printenv is from u-boot, used to operate environment of u-boot in Linux;

It failed when build with yocto SDK;

Recipe position: poky/meta/recipes-bsp/u-boot/u-boot-fw-utils_2018.01.bb

Add into image: CORE_IMAGE_EXTRA_INSTALL = "python3-core python3-pip python3-flask python3-flask-restful python3-six"  in local.conf


bitbake core-image-minima
bitbake core-image-minimal -c populate_sdk



04.18, 2019
------------------------------
* remove `ethtool` from image in local.conf;
* remove `phpdbg` 

* hostname, J.L. 04.18, 2019, add in local.conf
   * `hostname_pn-base-files = "mux744"`
   * recipe file is "base-files";
   * "hostname" is a variable of this recipe file, default it is `hostname = "${MACHINE}"`;
* Change password of root to Angelica_M774;
* Add new user jackl with password of 123;
   
Apache + Flask REST service
++++++++++++++++++++++++++++++++++++
* Enable mod_proxy+mod_http_proxy and configure http proxy between apache service and flash service:
   * /etc/apache2/httpd.conf for board
   * /etc/apache2/apache2.conf for ubuntun; `sudo service apache2 restart` re-start apache

::

    LoadModule proxy_module lib/apache2/modules/mod_proxy.so
    LoadModule proxy_http_module lib/apache2/modules/mod_proxy_http.so

    ProxyPass /api/ http://127.0.0.1:5000/
    ProxyPassReverse /api/ http://127.0.0.1:5000/


04.17, 2019
------------------------------
Configuration
++++++++++++++++++
* support bonjour service: found by Bonjour browser
  * add multicast route for eth0: 
::

   route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
   
  * start `avahi-daemon`;
  * Create file /etc/avahi/services/apache.service with following content:
::

   <?xml version="1.0" standalone='no'?><!--*-nxml-*-->
   <!DOCTYPE service-group SYSTEM "avahi-service.dtd">
   <service-group>
       <name replace-wildcards="yes">%h HTTP</name>
       <service>
          <type>_http._tcp</type>
          <port>80</port>
       </service>
   </service-group>

   * Be careful the router or switch configuration
   * Bonjour browser will find all services in /etc/avahi/services/;

Testing
++++++++++++++++++
With following commands in PC Linux:

::

   avahi-resolve -a 192.168.168.102 
       # IP is the address of avahi-daemon host;
   
   avahi-browse --resolve -a 
       # list all mDNS records received from network;

   sudo tcpdump -i eth0 igmp
       dump info like `IP 192.168.168.102 > 224.0.0.251: igmp v2 report 224.0.0.251`