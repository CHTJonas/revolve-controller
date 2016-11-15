avrdude \
	-p ATmega2560 \
	-c avrisp2 \
	-P /dev/tty.usbmodem* \
	-D \
	-U flash:w:build/revolve_controller.ino.hex \
	$@
