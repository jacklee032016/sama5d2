Enhancement of Data Connection
#######################################
Jack Lee, 06.17, 2019


User cases
=====================

#. Timeout, then close data connection: TCP/Unix/UDP;
#. Closed before reply; then close: Only TCP and Unix; UDP: no data socket for Data Connection;
#. Reply, then close;

#. Data connections are created and removed only by Broker thread;
#. Manager thread only reply with the connection, never close or free it;
#. No clock is needed for creation and removal of Data Connection then;


Modifications
=====================
* status between 2 threads: broker and manager;

broker:
---------------------------
* lock;
* modify status to waiting/closing when submit it to manager or timeout/broken;
* unlock;

* lock;
* if status==finished:
    * close all and free resources;
* unlock;


manager:
---------------------------
* operations on data;
* lock;
* if status==waiting:
    * write data;
* else if status==closing:
    * do nothing;
* set status=finished;
* unlock;	



Test Cases of client
=====================
*. send/read normally, then close;
*. send/read repeatedly, never close;
*. send/read once, but never close;
*. send then close, without wait to reply;

