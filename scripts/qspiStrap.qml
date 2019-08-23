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

		// erase 1M flash
		applet.erase(0, 0x10000)

		// write files
		var strapBinary = "Sama5d2Xpld-bootStrap-3.8.10.bin"
		//var strapBinary = "Mux774-bootStrap-3.8.10.bin"
		// this for uray code testing
		///var strapBinary = "muxlab_500774-dataflashboot--1.0.2.bin"
		print("-- Write strap '", strapBinary, "' --" )
		applet.write(0x00000, strapBinary, true)//
		
              
	}
}
