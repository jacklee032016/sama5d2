import SAMBA 3.2
//import SAMBA.Connection.Serial 3.2
import SAMBA.Connection.JLink 3.2
import SAMBA.Device.SAMA5D2 3.2

//SerialConnection {
JLinkConnection {

	//device: SAMA5D2Xplained {
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
	//}



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

		// erase all memory
		//applet.erase(0, applet.memorySize)
		// erase 1M flash
		applet.erase(0, 0x10000)

		// applet.erase(0, 0x100000)
		//applet.erase(0, 0xa0000)
		// write files
		applet.write(0x00000, "sama5d2_xplained-dataflashboot--3.8.12.bin", true)//
		//applet.write(0x00000, "muxlab_500768-dataflashboot--3.8.10.bin", true)//
		//applet.write(0x00000, "bootstrap.bin", true)//
		// applet.write(0x20000, "u-boot.bin")
		//applet.write(0xa0000, "m500768_0717.dtb")
		//applet.write(0xb0000, "zImage_0813")
		//applet.write(0x4d0000, "ramdisk0813.gz")
		//applet.write(0x18d0000, "jffs2_txapp")

              

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
