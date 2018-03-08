 Multi Mouse Setup - Ubuntu
 
 Instructions to disable regular functionality for a mouse
 and rename it as a specific multiMouse device, rather than a normal mouse.
 
 1. Add the file 90-multiMouse.rules to /etc/udev/rules.d/
 
 2. If adding a new mouse, follow same format and add to 90-multiMouse.rules
 	-specific mouse information is located in /dev/input/
 	-reference for further instructions
 	http://web.me.com/haroldsoh/tutorials/technical-skills/using-optical-mice-for-othe-2/
 	
 3. use this command to find the ATTRS{name}
 
 udevadm info -a -p `udevadm info -q path -n /dev/input/by-id/usb-Microsoft_Microsoft_3-Button_Mouse_with_IntelliEye_TM_-mouse`

