# PTP time/frequency adjustment

## system call adjtimex

### change frequency offset
in ```struct timex```, freq is in unit of ppm, is encoded with 16 bit value, every bit represent `2^(-16) ppm`, 
and value of `2^16` means one ppm;

For frequency draft of 1 ppm, it means 0.0864 seonds draft per day: 86400/1000,000=0.0864;

So in order to correct the offset of 0.64 seconds one day, the frequency offset should be:

  (2^16)*0.64/0.0864=485452

This value is written to the ```timex.freq```;

### change time offset or time value
* in unit of nansecond or microsecond;
* ```ADJ_OFFSET```: set offset, in the range of (-0.5s, +0.5s);
* ```ADJ_SETOFFSET```: set current time;


## ptp adjust time/frequency

### JUMP state of Servo
* adjust the time value in nanosecond;
* adjust the freq offset in ppb;

### LOCKED state of Servo
* only adjust freq offset;
* sync RTC to system time;


### set date time with busybox

date -s 2010.02.17-19:14:32


### Others

* router will block multicast traffic, never forward it to the all ports;
* When debug level is 7 for ptp daemon, it will timeout for FD_DELAY_TIMER;
* 9223372036, maximum number of seconds, kernel can set


* PTP runs in TAI Time
   * Start 1.1.1970 Midnight => Second 0
   * 48bit Seconds 32bit Nanoseconds => overflow in 8‘925‘512 years

* PTP provides UTC offset and Leap second to convert time between TAI and UTC in the end node

* our time source was sending out ARB time rather than PTP time.  The Hardware timestamped node was always out by the TAI / UTC offset (36 seconds at the moment) from the software timestamped node.

When it sends ARB (there is a flag in the PTP packet), it ends up sending the UTC time which appears to 
get interpreted by the hardware timestamping system as TAI, whereas the software system correctly picks 
it as UTC.  When it sends PTP time, it has to be TAI, so both interpret it correctly.

Once we managed to get the timesource to output PTP time (a Symmetricom source that wouldn't send PTP 
time unless it has a valid GPS lock) the problem went away.


As you observed, the difference is in the timestamping. With HW timestamping ptp4l keeps the PTP clock in TAI and it's 
up to phc2sys to convert it to UTC when it synchronizes the system clock. With SW timestamping ptp4l synchronizes the 
system clock, so it must convert it to UTC.


	c69b6a.fffe.c5e007-0 seq 0 RESPONSE MANAGEMENT GRANDMASTER_SETTINGS_NP 
		clockClass              255
		clockAccuracy           0xfe
		offsetScaledLogVariance 0xffff
		currentUtcOffset        37
		leap61                  0
		leap59                  0
		currentUtcOffsetValid   0
		ptpTimescale            0
		timeTraceable           0
		frequencyTraceable      0
		timeSource              0xa0
	000c17.fffe.48216b-1 seq 0 RESPONSE MANAGEMENT GRANDMASTER_SETTINGS_NP 
		clockClass              255
		clockAccuracy           0xfe
		offsetScaledLogVariance 0xffff
		currentUtcOffset        37
		leap61                  0
		leap59                  0
		currentUtcOffsetValid   0
		ptpTimescale            1
		timeTraceable           0
		frequencyTraceable      0
		timeSource              0xa0

### 2 masters in same LAN
* Normally Clocks are negotiated with each other, so only one master is in one LAN;
* But some clocks are configured as master only, it may always multicast message no matter it does not play as master in LAN;
* So in this seneory, slave may be in UNCALIBRATED state for ever.

* In real network, only one master can exist in one LAN;
* When more than 2 masters(for example, ppt4l can be configured as MASTER_ONLY), the result is undefined:
   * no unicast dest address in all of the 10 types of PTP message, all messages are sent to multicast address;
   

### stat->max_count
* ***max_count*** indicates how many sync packets received in one local sync interval;
   * if it is 1, so in local sync interval, the offset and ingress ts can be shown directly;
   * if it is more than 1, so the offset and ingress ts can be only shown in RMS format;
   
   Master will teld slaves its SynInterval??? Which message???

### about system clocks
* there are about 5 or 6 clock systems in kernel, such as REALTIME, MONOTONIC, CPU_RELATED, hardware clock, etc.
* When hardware timestamp is used, PHC is used to get time from REALTIME Clock which is updated by ppt4l;
* No matter hardware timestamp or software timestamp, ppt4l always update REALTIME clock:
   * MONOTONIC clock can't be updated by user, so it never be used in ptp;
* So when software timestamp is used, only REALTIME clock are updated;

### free_running
* When update timestamp in free_running, it never return LOCKED state???


### ts_process and clock servo
Used in state of SLACE or UNCALIBRATED;

* ***ts_process***
    * calculate offset from t1~t4
	* algorithm: move_average or move_median
	* result is used to update offset;

* ***clock servo***
    * determine whether tracking master clock or not;
	* calculate frequency adjustment;
	* set frequency after offset has been set up;
	   * PI sample: at least 2 samples are needed;
	   * Linear regression:
	   * NTP shm: send shared memory to other program;


### Sometimes, system clock adject will fail:
* Realtime clock: wall time;
* Monotonic clock: always increase
* failed:
     * minus offset
	 * offset is still in the range of realtime clock, eg, after 1970.01.01

* How to make it happen again or always happen
   * start time, eg. monotonic time is small;
   * offset is bigger: smaller than timestamp of realtime, but bigger than timestamp of monotonic

* Kernel space
   * do_adjusttimex
   * time_keeper: data structure for time maintainence in kernel space
   * update with wall clock time:
   * check it is also validate for monotonic before it update
   

### Slave always stay in state of **UNCALIBRATED**
* No DELAY_REQUEST is sent out or no DELAY_RESPONSE is received
   * DELAY_TIME timeout, the DELAY_REQUEST is sent:
      * clear existed DELAY_REQUEST which has been sent out;
	  * reset the timeout value of time:
	     * based on portDS.logMinDelatReqInterval, which is updated by DELAY_RESPONSE from master;
	  * sent and add into queue to wait receiving DELAY_RESPONSE;
	  * when debug is off, it work much better;

* Receive DELAY_RESPONSE:
   * check it is response for the existing DELAY_REQUEST;
   * begin ```E2E delay mechanism```: update t3 and t4, check t1, t2 has been set in Clock Servo;

* Receive SYNC or FOLLOW_UP:
   * if t1, t2, t3, t4 all have been set;
   * begin timestamp process and SERVO process;
   * update offset of timestamp and enter locked state, eg. from UNCALIBRATED to SLAVE;


## ptp4l design
### When software clock is used
* only two-step mode can be used; one-step only used in hardware clock
* When hardware clock is used, the timestamp of every PTP message is filled by driver, user space software will not write any timestamp;
* When software clock is used:
   * timestamp of ANNOUNCE and SYNC message is 0;
   * timestamp of FOLLOW_UP is current local time stamp, so the time is updated to current time if master is ptp4l;
      * timestamp of local time is fetched after every message is sent out;
	  * this timestamp is maintainent in user space and fill the timestamp of FOLLOW_UP message or DELAY_REQUEST message;
	  

**Questions**:

* How to fill the timestamp field of different message when play as master?
* Locked or SLAVE state: when it is in SALVE state, the clock must be locked?
* How to use profile ID?

   

## Management msg

* "GET CLOCK_DESCRIPTION"
   * local clock information, such as profileID;

* PARENT_DATA_SET
   * parent, like master's data
		parentPortIdentity                    0090e8.fffe.6da412-1
		parentStats                           0
		observedParentOffsetScaledLogVariance 0xffff
		observedParentClockPhaseChangeRate    0x7fffffff
		grandmasterPriority1                  12
		gm.ClockClass                         221
		gm.ClockAccuracy                      0x23
		gm.OffsetScaledLogVariance            0xffff
		grandmasterPriority2                  128
		grandmasterIdentity                   0090e8.fffe.6da412
   
* GRANDMASTER_SETTINGS_NP 
		clockClass              255
		clockAccuracy           0xfe
		offsetScaledLogVariance 0xffff
		currentUtcOffset        37
		leap61                  0
		leap59                  0
		currentUtcOffsetValid   0
		ptpTimescale            0
		timeTraceable           0
		frequencyTraceable      0
		timeSource              0xa0
   

* **CURRENT_DATA_SET**
   *    stepsRemoved     1
		offsetFromMaster -646.0
		meanPathDelay    49025.0

* **DEFAULT_DATA_SET**
   * local data set
   
* **TIME_PROPERTIES_DATA_SET**
		currentUtcOffset      0
		leap61                0
		leap59                0
		currentUtcOffsetValid 0
		ptpTimescale          1
		timeTraceable         0
		frequencyTraceable    0
		timeSource            0xa0
* **TIME_STATUS_NP**
		master_offset              -6641: offset from master, ns
		ingress_time               16464375793513: local timestamp when ingress packet, ns
		cumulativeScaledRateOffset +0.000000000
		scaledLastGmPhaseChange    0
		gmTimeBaseIndicator        0
		lastGmPhaseChange          0x0000'0000000000000000.0000
		gmPresent                  true
		gmIdentity                 0090e8.fffe.6da412

* **PORT_DATA_SET**
		portIdentity            c69b6a.fffe.c5e007-1
		portState               SLAVE
		logMinDelayReqInterval  0
		peerMeanPathDelay       0
		logAnnounceInterval     1
		announceReceiptTimeout  3
		logSyncInterval         -3
		delayMechanism          1
		logMinPdelayReqInterval 0
		versionNumber           2

* **PORT_DATA_SET_NP**
		neighborPropDelayThresh 20000000
		asCapable               1

   