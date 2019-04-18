Work Logs for File System
################################
April 17th, 2019


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