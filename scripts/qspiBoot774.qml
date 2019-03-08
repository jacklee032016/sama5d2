import SAMBA 3.2
import SAMBA.Connection.JLink 3.2
import SAMBA.Device.SAMA5D2 3.2

JLinkConnection {
	//port: "99999999"

	device: SAMA5D2Xplained {
		// to use a custom config, replace SAMA5D2Xplained by SAMA5D2 and
		// uncomment the following lines, or see documentation for
		// custom board creation.
		//config {
		//	qspiflash {
		//		instance: 0
		//		ioset: 3
		//		freq: 66
		//	}
		//}
	}

	onConnectionOpened: {
		// initialize QSPI applet
		initializeApplet("qspiflash")

		// erase and write bootstrap
		applet.erase(0, 0x10000)
		applet.write(0x00000, "Binaries/744/m500774-bootstrap.bin", true)
		
		//
		applet.erase(0x20000, 0x80000)
		applet.write(0x20000, "Binaries/744/m500774-u-boot.bin")

		// initialize boot config applet
		initializeApplet("bootconfig")

		// Use BUREG0 as boot configuration word
		applet.writeBootCfg(BootCfg.BSCR, BSCR.fromText("VALID,BUREG0"))

		// Enable external boot only on QSPI0 IOSET3
		applet.writeBootCfg(BootCfg.BUREG0,
			BCW.fromText("EXT_MEM_BOOT,UART1_IOSET1,JTAG_IOSET1," +
			             "SDMMC1_DISABLED,SDMMC0_DISABLED,NFC_DISABLED," +
			             "SPI1_DISABLED,SPI0_DISABLED," +
			             "QSPI1_DISABLED,QSPI0_IOSET3"))
	}
}
