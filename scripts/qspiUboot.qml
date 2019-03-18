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

		// old version: 0x20000, size 0x80000
		applet.erase(0x20000, 0x80000)
		// write old
		applet.write(0x20000, "u-boot.bin", true)//
		//applet.write(0x20000, "m500774-u-boot.bin", true)//
		
		
		// new version 0x60000, size 0x100000
		// applet.erase(0x60000, 0x100000)
		// write new version
		// applet.write(0x60000, "u-boot.bin", true)//

	}
}
