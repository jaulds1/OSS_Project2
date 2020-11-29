Project 2
Joseph Aulds && Adam Kotlinski

Explanation when opening the device from a program that does not have the appropriate capability.
  If you try and open /dev/secret612a or /dev/secret612b, the following error will print, "Can't open device file: secret612<a/b>". However, if the capability is enabled on the usermode file, you are able to open the /dev/device for reading or writing.

Explanation of kernel modifications for the capability.
  We added capability #38 CAP_SECRET_SIXONETWO to the kernel. This prevents unauthorized access to our devices. We added it in linux/capability.h and then attempted to recompile the kernel but received several errors, we had to modify another kernel file and change the end bound of existing capabilities that are looped through to instead look for our capability as the last one. 

Explanation of crypto implementation
	We are using OpenSSL's EVP libaraies to implement AES-256 in CBC mode.  We are following the standard calling convention/order for initializing the crypto context for encryption and decryption.  Both functions use an init function to create their context.  You define your cipher and mode as well as bring in your iv and key within this function.
	You then call an update function which will add the cipher/plaintext to the context.
	Finally, you will call its "final" function to encrypt/decrypt any partial blocks that were not encrypted/decrypted by the update function.
	
Setting file capability
	We have a compile.sh script in the USERMODE folder which sets the capability for us.  
	It is doing "sudo setcap 38+ep usermode".  This sets the 38th capabiltiy as an effective permission, which is our capability, on the usermode binary.
