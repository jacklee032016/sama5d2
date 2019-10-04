/*
* Common for PHC (Ptp Hardware Clock) access in Linux
* JL.
*/ 


static clockid_t clock_open(char *device, int *phc_index)
{
	struct sk_ts_info ts_info;
	char phc_device[19];
	int clkid;

	/* check if device is CLOCK_REALTIME */
	if (!strcasecmp(device, "CLOCK_REALTIME"))
		return CLOCK_REALTIME;

	/* check if device is valid phc device */
	clkid = phc_open(device);
	if (clkid != CLOCK_INVALID)
	{/* when it is phc device */
		return clkid;
	}

	/* check if device is a valid ethernet device */
	if (sk_get_ts_info(device, &ts_info) || !ts_info.valid)
	{
		EXT_ERRORF("unknown clock %s: %m", device);
		return CLOCK_INVALID;
	}

	if (ts_info.phc_index < 0)
	{
		EXT_ERRORF("interface %s does not have a PHC", device);
		return CLOCK_INVALID;
	}

	/* if ethernet device is also PHC, then open phc and associated with this ethernet device */
	sprintf(phc_device, "/dev/ptp%d", ts_info.phc_index);
	clkid = phc_open(phc_device);
	if (clkid == CLOCK_INVALID)
	{
		EXT_ERRORF("cannot open %s: %m", device);
	}
	*phc_index = ts_info.phc_index;
	return clkid;
}

