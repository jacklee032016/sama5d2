import SAMBA 3.2
//import SAMBA.Connection.Serial 3.2
import SAMBA.Connection.JLink 3.2
import SAMBA.Device.SAMA5D2 3.2

//SerialConnection {
JLinkConnection {

	// device: SAMA5D2 {//this is for Mux7XX board
	device: SAMA5D2Xplained { // This is for xplained board, SD/MMC instance only in xplained 
	}// other board don't has mmc card
	
	onConnectionOpened: {

		// initialize SD/MMC applet for on-board eMMC
		print("-I- === Initialize eMMC access ===")
		initializeApplet("sdmmc")

		print("-I- === Load rootfs on eMMC ===")
		applet.write(0, "atmel-xplained-demo-image-sama5d2-xplained.wic", false)
		applet.write(0, "App.ext4.2019_05_26", false)

		print("-I- === Done. ===")
	}
}
