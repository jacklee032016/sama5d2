import SAMBA 3.2
//import SAMBA.Connection.Serial 3.2
import SAMBA.Connection.JLink 3.2
import SAMBA.Device.SAMA5D2 3.2

//SerialConnection {
JLinkConnection {

	device: SAMA5D2 {
		// to use a custom config, replace SAMA5D2Xplained by SAMA5D2 and
		// uncomment the following lines, or see documentation for
		// custom board creation.
		config {
			qspiflash {
				instance: 0
				ioset: 3
				freq: 50
			}
		}
	}
	onConnectionOpened: {
		// initialize QSPI applet
		initializeApplet("qspiflash")

		applet.erase(0x100000, 0x500000)

		applet.write(0x100000, "uImage", true)//

		// initialize boot config applet
//		initializeApplet("bootconfig")

		// Use BUREG0 as boot configuration word
//		applet.writeBootCfg(BootCfg.BSCR, BSCR.fromText("VALID,BUREG0"))

		// Enable external boot only on QSPI0 IOSET3
//		applet.writeBootCfg(BootCfg.BUREG0,
//			BCW.fromText("EXT_MEM_BOOT,UART1_IOSET1,JTAG_IOSET1," +
//			             "SDMMC1_DISABLED,SDMMC0_DISABLED,NFC_DISABLED," +
//			             "SPI1_DISABLED,SPI0_DISABLED," +
//			             "QSPI1_DISABLED,QSPI0_IOSET3"))
	}
}
