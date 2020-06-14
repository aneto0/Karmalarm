
USB_PORT?=/dev/ttyUSB0

ESP_FIRMWARE?=esp8266-20191220-v1.12.bin 

flash:
	esptool.py --chip auto --port ${USB_PORT} --baud 115200 erase_flash
	esptool.py --port ${USB_PORT} --baud 460800 write_flash --flash_size=detect 0 ${ESP_FIRMWARE}

install:
	ampy --port ${USB_PORT} put wifi_secrets.py /wifi_secrets.py
	ampy --port ${USB_PORT} put wifi.py /wifi.py
	ampy --port ${USB_PORT} put boot.py /boot.py
	ampy --port ${USB_PORT} put main.py /main.py
	ampy --port ${USB_PORT} put diagnostics.py /diagnostics.py

serial:
	picocom ${USB_PORT} -b115200
