Project 2
Joseph Aulds && Adam Kotlinski

Explanation when opening the device from a program that does not have the appropriate capability.
  If you try and open /dev/secret612a or /dev/secret612b, the following error will print, "Can't open device file: secret612<a/b>". However, if the capability is enabled on the usermode file, you are able to open the /dev/device for reading or writing.

Explanation of kernel modifications for the capability.
  We added capability #38 CAP_SECRET_SIXONETWO to the kernel. This prevents unauthorized access to our devices. We added it in linux/capability.h and then attempted to recompile the kernel but received several errors, we had to modify another kernel file and change the end bound of existing capabilities that are looped through to instead look for our capability as the last one. 
