import SAMBA 3.2
//import SAMBA.Connection.Serial 3.2
import SAMBA.Connection.JLink 3.2
import SAMBA.Device.SAMA5D2 3.2

//SerialConnection {
JLinkConnection {

	device: SAMA5D2Xplained {
	}

	function initNand() {
		/* Placeholder: Nothing to do */
	}

	function getEraseSize(size) {
		/* get smallest erase block size supported by applet */
		var eraseSize
		for (var i = 0; i <= 32; i++) {
			eraseSize = 1 << i
			if ((applet.eraseSupport & eraseSize) !== 0)
				break;
		}
		eraseSize *= applet.pageSize

		/* round up file size to erase block size */
		return (size + eraseSize - 1) & ~(eraseSize - 1)
	}

	function eraseWrite(offset, filename, bootfile) {
		/* get file size */
		var file = File.open(filename, false)
		var size = file.size()
		file.close()

		applet.erase(offset, getEraseSize(size))
		applet.write(offset, filename, bootfile)
	}

	onConnectionOpened: {

		var itbFileName = "sama5d2_xplained.itb"
		var ubootEnvFileName = "u-boot-env.bin"
		print("-I- === Initialize serialflash access ===")
		initializeApplet("serialflash")

		// erase then write files
		print("-I- === Load AT91Bootstrap ===")
		eraseWrite(0x00000000, "at91bootstrap-sama5d2_xplained.bin", true)

		print("-I- === Load u-boot environment ===")
		eraseWrite(0x00006000, ubootEnvFileName)

		print("-I- === Load u-boot ===")
		eraseWrite(0x00008000, "u-boot-sama5d2-xplained.bin")

		// initialize SD/MMC applet for on-board eMMC
		print("-I- === Initialize eMMC access ===")
		initializeApplet("sdmmc")

		print("-I- === Load rootfs on eMMC ===")
		applet.write(0, "atmel-xplained-demo-image-sama5d2-xplained.wic", false)

		print("-I- === Done. ===")
	}
}
