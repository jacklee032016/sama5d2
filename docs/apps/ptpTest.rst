./Linux.bin.X86/usr/bin/pmc -u -b 0 'GET CURRENT_DATA_SET'
sending: GET CURRENT_DATA_SET
2019-10-11 14:01:32 [INFO,pmc]: [msg.c-389.ptpMsgReceive()]:Receiving MSG MANAGEMENT with length 48, total 72
	080027.fffe.e7b763-0 seq 0 RESPONSE MANAGEMENT CURRENT_DATA_SET 
		stepsRemoved     1
		offsetFromMaster 47535.0
		meanPathDelay    235288.0

./Linux.bin.X86/usr/bin/pmc -u -b 0 'GET CLOCK_DESCRIPTION'
sending: GET CLOCK_DESCRIPTION
2019-10-11 14:15:22 [INFO,pmc]: [msg.c-389.ptpMsgReceive()]:Receiving MSG MANAGEMENT with length 48, total 132
	080027.fffe.e7b763-1 seq 0 RESPONSE MANAGEMENT CLOCK_DESCRIPTION 
		clockType             0x8000
		physicalLayerProtocol IEEE 802.3
		physicalAddress       08:00:27:e7:b7:63
		protocolAddress       1 192.168.168.101
		manufacturerId        03:02:00
		productDescription    PTP Clock of MuxLab;;
		revisionData          01 02;;
		userDescription       Mux774;
		profileId             00:1b:19:00:01:00


./Linux.bin.X86/usr/bin/pmc -u -b 0 'GET TIME_STATUS_NP'
sending: GET TIME_STATUS_NP
2019-10-11 14:04:41 [INFO,pmc]: [msg.c-389.ptpMsgReceive()]:Receiving MSG MANAGEMENT with length 48, total 104
	080027.fffe.e7b763-0 seq 0 RESPONSE MANAGEMENT TIME_STATUS_NP 
		master_offset              -124753
		ingress_time               1570817081304662227
		cumulativeScaledRateOffset +0.000000000
		scaledLastGmPhaseChange    0
		gmTimeBaseIndicator        0
		lastGmPhaseChange          0x0000'0000000000000000.0000
		gmPresent                  true
		gmIdentity                 0090e8.fffe.6da412


./Linux.bin.X86/usr/bin/pmc -u -b 0 'GET PORT_DATA_SET'
sending: GET PORT_DATA_SET
2019-10-11 14:23:18 [INFO,pmc]: [msg.c-389.ptpMsgReceive()]:Receiving MSG MANAGEMENT with length 48, total 80
	080027.fffe.e7b763-1 seq 0 RESPONSE MANAGEMENT PORT_DATA_SET 
		portIdentity            080027.fffe.e7b763-1
		portState               SLAVE
		logMinDelayReqInterval  0
		peerMeanPathDelay       0
		logAnnounceInterval     1
		announceReceiptTimeout  3
		logSyncInterval         0
		delayMechanism          1
		logMinPdelayReqInterval 0
		versionNumber           2
lzj@lzj-Ubuntu:~/yocto/sama5d2/apps$ ./Linux.bin.X86/usr/bin/pmc -u -b 0 'GET PORT_DATA_SET_NP'
sending: GET PORT_DATA_SET_NP
2019-10-11 14:23:42 [INFO,pmc]: [msg.c-389.ptpMsgReceive()]:Receiving MSG MANAGEMENT with length 48, total 62
	080027.fffe.e7b763-1 seq 0 RESPONSE MANAGEMENT PORT_DATA_SET_NP 
		neighborPropDelayThresh 20000000
		asCapable               1
