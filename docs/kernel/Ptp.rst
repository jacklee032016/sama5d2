PTP Hardware timestamp
################################


Configuration Items:

* enable hardware timestamp: CONFIG_MACB_USE_HWSTAMP
* define compatible = "atmel,sama5d2-gem" for macb0;
* define struct macb_config sama5d2_config to support MACB_CAPS_GEM_HAS_PTP;

But "macb f8008000.ethernet eth0: DMA bus error: HRESP not OK"

So, disable it in code of ``macb_configure_caps()``;
