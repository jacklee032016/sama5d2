# PTP basis

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
* E2E Transparent Clock: independent of thse mechanism
   * forward all messages;
   * resident time --> corect field of event message;   
* P2P Transparent Clock: peer mechanism
   * differ in the way it corrects and handles the PTP timing message;
* Management device:

### 2 Delay measuring Mechanisms
* Delay request-response mechanism: 
* Peer delay mechanism:

### FSM fpr Transparent Clock

   