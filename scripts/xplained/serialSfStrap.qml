import SAMBA 3.2
import SAMBA.Connection.Serial 3.2
//import SAMBA.Connection.JLink 3.2
import SAMBA.Device.SAMA5D2 3.2

// Serial Flash, programming Strap in serial port

SerialConnection {
//JLinkConnection {

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
		print("-I- === Load AT91Bootstrap Sama5d2Xpld-bootStrap-3.8.10 ===")
		eraseWrite(0x00000000, "Sama5d2Xpld-bootStrap-3.8.10.bin", true)

		print("-I- === Done. ===")
	}
}
