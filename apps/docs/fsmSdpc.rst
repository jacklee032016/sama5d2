FSM of SDP Client
#######################################
Jack Lee, Juillet 3rd, 2019


Infrastructure
=====================================

2 threads and one system timer:
* system timer send SDP request to SdpManager thread;
* Thread SdpManager process events with FSM of every SDP client;
* Thread SdpRecv only poll the SDP clients which are in state of CONNECTED, and receive multiple packets for SDP request;


FSM of every SDP client
=====================================

state of WAIT
-----------------------
Only process event of NEW:


state of CONNECTED
-----------------------
Process following events:
#. New: 
   * sent by system timer, means new request arrives;
   * ignore it because SDP client is slower than timer;

#. RECV:
   * sent by SdpRecv thread, indicates http header parsing OK, and http data is ready;
   * parse SDP content, and send JSON object to System Manager or configure FPGA directly;

#. ERROR:
   * sent by SdpRecv, indicates parsing error in HTTP header or error from socket;
   * clear status of client, and return into state WAIT;

#. CLOSE:
   * sent by SdpRecv, indicates connection is closed by peer;
   * clear status of client, and return to state WAIT;

#. Timeout:
   * sent by SdpRecv, indicates timeout for this socket;
   * clear status of client, and return to state WAIT;
