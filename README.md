Project 2
Joseph Aulds && Adam Kotlinski

Explanation when opening the device from a program that does not have the appropriate capability.
  If you try and open /dev/secret612a or /dev/secret612b, the following error will print, "Can't open device file: secret612<a/b>". However, if the capability is enabled on the usermode file, you are able to open the /dev/device for reading or writing.
