# PTP basis

### Event and General Messages
#### Event messages
* SYNC;
* DelayReq;
* PDelayReq;
* PDelayResp;
#### General messages: 6
* ANNOUNCE
* FollowUp;
* DelayResp;
* PDelayFollowUp;
* SIGNAL;
* MANAGEMENT;


## PTP message
* Every message from peer defined the LogMsgInterval for that type of message;
   * last field of header of every message type;
* Such as SyncLogInterval:
   * When 2 SyncLogInterval are different, multiple SYNC/FollowUp can be received in one local sync interval:
   * so stats->max_count>1, so debug offset/freqAdj/Delay is format of RMS;

## Boundary Clock and Transparent Clock

## Receive message

### Receive ANNOUNCE message: 
* update data set;

### Receive SYNC/FollowUp message:
* update/sync local clock;

### Receive DELAY_RESPONSE message:
* execute the delay request-response mechanism: 11.3
* measure the mean path delay;

### SIGNAL message:
???

## Data Strcutures

### Clock Data Set:
* default DS;
* Parent DS;
* Current DS;
* TimePrioperties: attributes in ANNOUNCE message;

### Port 
* Port DS;
* Foreign MasterDS;


## device types and mechanisms

### devices
* Ordinary Clock: Either mechanism
   * One physical interface;
   * a single PTP state;
   * clock data set: 
      * defaultDS; 
	  * currentDS; 
	  * parentDS;
	  * timePropertyDS;
   * Port data set;
* Boundary Clock: Either mechanism
   * several physical interfaces;
   * clock datasets are common to all ports;
   * local clock is common to all ports;
   * choose one port to update the local clock;
   * msg not forward, except Mgmt;
   * Network element;
* E2E Transparent Clock: 
   * independent of thse mechanism:
      * no state machine, stateless;
	  * not take part in the process of protocol;
   * forward all messages;
   * resident time --> corect field of event message;   
* P2P Transparent Clock: peer mechanism
   * differ in the way it corrects and handles the PTP timing message;
   * forward only 3 Peer Message: PDelayReq, PDelayResp, PDelayFollowUp;
* Management device:


### 2 Delay measuring Mechanisms
* Delay request-response mechanism: 
* Peer delay mechanism:
   * Non use CBMA algorithm to select master;

### Transparent Clock(TC)
* forward messages;
* correct message with ***residence time***;

#### E2E TC:
* correct all messages;
* never measuring delay;

#### P2P TC:
* measuring delay;
* correct SYNC, FollowUp message, only


### FSM fpr Transparent Clock


## Weaknesses of ptp4l
* p Impt design pattern;
* Only one thread, not utilizing CPU fully;
* bind to OS time(realtime clock);
* free_running can't use hardware clock, so accurracy is limited.

   