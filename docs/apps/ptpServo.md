# PTP Servo and clock discipline

12.06, 2019  Jack Lee


## Servo
keep pace with minimum drift;

two stages:
* Before locked: PLL, timestamp;
* After locked: FLL, only freq;

Decide:
* When locked;
* freq adjustment;

* parameters of Servo
   * For all types of Servo;
   
   * FirstUpdate:
   * Threshold:
   * FreqThreshold:

* updateInterval()
   * The intervals used to calculate freq is changed over time;
   * Interval is variable, which is defined by protocol;
* sample()
   * how to implement feedback in PI;
   * How to update Kp, ki component of offset;
   * How to calculate freq with Kp, Ki;
   
   

## PI Servo

* input: offset
* output: freq (ppm of freq variation)
* params: kp, ki
   * Proportional: absolute current value;
   * Integral: energy of signal;

**sensitive**: 
* Kp, Ki is big, eg current value (offset) is more important; such as 0.7 and 0.3;
* hardware clock: use sensitive schema, because the offset is more accurate, smaller jitter in offset measurement;

**insensitive**:
* Kp, Ki is small, such as 0.1, 0.001;
* software clock: use insensitive schema, because jitter in offset measurement is big; 
   * provent over-reacting;
   

## About Feedback


* How to determine whether locking or not locking in Servo?
* How does sensitivity involved in the decision?
* How to calculate frequency from offset within Servo?
* Why it is easy to lock when ptp run the second time?


### Measure offset
* **tms**: Ingress timestamp of FollowUp and the corresponding SYNC;
* **tsm**: timestamp of DelayReq and timestamp in packet of DelayResponse;
* delay=(tms+tsm)/2; then filtering
* offset=tms-delay; the filtering

* process of time stamp:
   * moving-average;
   * moving-median;
   

### Linux PLL/FLL theory

PLL: tune phase(timestamp)
FLL: tune frequency(freq wander)

