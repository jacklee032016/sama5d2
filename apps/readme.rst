Application for REST API and IP Commands
##########################################
Jack Lee  April 5th, 2019


Change Log
===========================

#. **April 5th, 2019**
------------------------
* build 4 libraries from **'Shared'** directory:
   #. **libShare**:
   #. **libMux**:
   #. **libCmnSys**:
   #. **libCgi**:
* 3 different programs and one plugin from **'src'** directory:
   * **muxController**: control daemon program;
   * **apiClient**: IP command client, use TCP/UDP socket;
   * **muxApis**: REST CGI program;
   * **MuxPluginWeb**: plugin for web service;

   