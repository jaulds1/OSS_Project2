/*********************************************************************
*
* This is a skeleton usermode program for the char device
* JHU Operating Systems Security (695.612)
*
*
* Author: T. McGuire
* License: GPL
*
*
*********************************************************************/

#include "../COMMON/char_ioctl.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/random.h>
#include <syscall.h>

//
// This header is required for the ioctl() call
#include <sys/ioctl.h>

#define MAX_READ_SIZE 1024
#define MAX_WRITE_SIZE 1024
#define IV_SIZE 16
#define KEY_SIZE 32
#define IOCTL_SIZE 49

//Global Variables for crypto
unsigned char * iv_a;
unsigned char * key_a;
unsigned char * iv_b;
unsigned char * key_b;

int ioctl_set_data(int fd, char * data, char * dev)
{
    int i;
    char c;

    printf("[+] %s called\n", __FUNCTION__ );

    ioctl(fd, IOCTL_WRITE_TO_KERNEL, data );

    printf("[+]    Data written to %s: %x\n", dev, data );

    return 0;

}

int ioctl_read_data(int fd, char * data, char * dev)
{
    int i;
    char c;



    ioctl(fd, IOCTL_READ_FROM_KERNEL, data );

    if (NULL != data[0])
    {
      printf("[+] %s called\n", __FUNCTION__ );
      printf("[+]    Data read from %s: %x\n", dev, data );
    }



    return 0;

}

int main( int argc, char ** argv )
{

    int fd_a = -1;
    int fd_b = -1;
    char* dev_a = "a";
    char* dev_b = "b";
    int ret = -1;


    char unsigned read_from_b[MAX_READ_SIZE];
    char unsigned read_from_a[MAX_READ_SIZE];
    char unsigned write_to_a[MAX_WRITE_SIZE];
    char unsigned write_to_b[MAX_WRITE_SIZE];
    char unsigned keypair_a[IOCTL_SIZE];
    char unsigned keypair_b[IOCTL_SIZE];


    memset(read_from_a, 0, MAX_READ_SIZE);
    memset(read_from_b, 0, MAX_READ_SIZE);

    iv_a = malloc(IV_SIZE);
    key_a = malloc(KEY_SIZE);;
    iv_b = malloc(IV_SIZE);
    key_b = malloc(KEY_SIZE);;



    if (argc != 2)
    {
      printf("ERROR: You must enter a device name!\n");
      return -1;
    }

    /**************************************************************************/

    /*****************************AAAAAAAAAAAAAAAAAAAA*************************/

    /**************************************************************************/

    if (0 == strncmp(argv[1], dev_a, 1))
    {
        int wait_count = 60;
        printf("EXECUTING A\n");
        char devname_a[32];
        strcpy(devname_a, "/dev/");
        strcat(devname_a, DEVICE_NAME_A );

        fd_a = open(devname_a, O_WRONLY);

        if (fd_a < 0)
        {
            printf("Can't open device file: %s\n", DEVICE_NAME_A);
            return -1;
        }



        char devname_b[32];
        strcpy(devname_b, "/dev/");
        strcat(devname_b, DEVICE_NAME_B );

        fd_b = open(devname_b, O_RDONLY);

        if (fd_b < 0)
        {
            printf("Can't open device file: %s\n", DEVICE_NAME_B);
            return -1;
        }


        //init crypto
        initializeEncryption(iv_a, key_a);

        strncat(keypair_a, iv_a, IV_SIZE);
        strncat(keypair_a, ":", 1);
        strncat(keypair_a, key_a, KEY_SIZE);

        //write it to ourselves
        ret = ioctl_set_data(fd_a, keypair_a, dev_a);

        memset(keypair_b, 0, IOCTL_SIZE);

        //wait until we can read from other device
        printf("Will wait up to 60 seconds to receive keypair from user B.\n[");
        while(keypair_b[0] == NULL && wait_count > 0) {
          wait_count -= 1;
          printf("#");
          fflush(stdout);
          ret = ioctl_read_data(fd_b, keypair_b, dev_b);
          sleep(1);
        }
        //if more than 60 seconds then die
        if (0 == wait_count)
        {
          printf("Error: Did not receive keypair in time. Closing program.\n");
          return -1;
        }

        memset(read_from_b, 0, MAX_READ_SIZE);
        printf("CRYPTO IS DONE");


        //
        //printf("keypair_b: %s", keypair_b);
        //
        // You will also use the read() and write() system calls
        //

        while(1)
        {
          int len = 0;
          memset(write_to_a, 0, MAX_WRITE_SIZE);
          printf("\n[Me]: ");
          fgets(write_to_a, MAX_WRITE_SIZE, stdin);
          for (int i = 0; i < MAX_WRITE_SIZE; i++)
          {
            len += 1;
            if (NULL == write_to_a[i])
            {
              break;
            }
          }
          write(fd_a, write_to_a, len);

          //read(fd_b, read_from_b, MAX_READ_SIZE);
          while(read_from_b[0] == NULL) {
            read(fd_b, read_from_b, MAX_READ_SIZE);
            sleep(0.25);
          }
          printf("\nRead from [b]: %s\n", read_from_b);
          memset(read_from_b, 0, MAX_READ_SIZE);
        }

    //JAULDS1: Possibly do not close here!!
        close(fd_a);
        close (fd_b);
    }

/**************************************************************************/

/*******************************BBBBBBBBBBBBBBBB***************************/

/**************************************************************************/

    else if (0 == strncmp(argv[1], dev_b, 1))
    {
      int wait_count = 60;
      printf("EXECUTING B\n");
      char devname_a[32];
      strcpy(devname_a, "/dev/");
      strcat(devname_a, DEVICE_NAME_A );

      fd_a = open(devname_a, O_RDONLY);

      if (fd_a < 0)
      {
          printf("Can't open device file: %s\n", DEVICE_NAME_A);
          return -1;
      }



      char devname_b[32];
      strcpy(devname_b, "/dev/");
      strcat(devname_b, DEVICE_NAME_B );

      fd_b = open(devname_b, O_WRONLY);

      if (fd_b < 0)
      {
          printf("Can't open device file: %s\n", DEVICE_NAME_B);
          return -1;
      }

      //ret = ioctl_set_data(fd_b, keypair_b, dev_b);
      //ret = ioctl_read_data(fd_a, keypair_a, dev_a);

      //init crypto
      initializeEncryption(iv_b, key_b);

      strncat(keypair_b, iv_b, IV_SIZE);
      strncat(keypair_b, ":", 1);
      strncat(keypair_b, key_b, KEY_SIZE);

      //write it to ourselves
      ret = ioctl_set_data(fd_b, keypair_b, dev_b);

      memset(keypair_a, 0, IOCTL_SIZE);

      //wait until we can read from other device
      printf("Will wait up to 60 seconds to receive keypair from user A.\n[");
      while(keypair_a[0] == NULL && wait_count > 0)
      {
        wait_count -= 1;
        printf("#");
        fflush(stdout);
        ret = ioctl_read_data(fd_a, keypair_a, dev_a);
        sleep(1);
      }

      //if more than 60 seconds then die
      if (0 == wait_count)
      {
        printf("]\nError: Did not receive keypair in time. Closing program.\n");
        return -1;
      }
      //if more than 60 seconds then die
      memset(read_from_a, 0, MAX_READ_SIZE);
      printf("CRYPTO IS DONE");
      while(1)
      {
        int len = 0;
        while(read_from_a[0] == NULL) {
          read(fd_a, read_from_a, MAX_READ_SIZE);
          sleep(0.25);
        }
        printf("\nRead from [a]: %s\n", read_from_a);
        memset(read_from_a, 0, MAX_READ_SIZE);
        memset(write_to_b, 0, MAX_WRITE_SIZE);
        printf("[Me]: ");
        fgets(write_to_b, MAX_WRITE_SIZE, stdin);
        for (int i = 0; i < MAX_WRITE_SIZE; i++)
        {
          len += 1;
          if (NULL == write_to_b[i])
          {
            break;
          }
        }

        write(fd_b, write_to_b, len);

      }
      //
      // You will also use the read() and write() system calls
      //
      close(fd_a);
      close(fd_b);
    }
    return 0;
}

void initializeEncryption(char * iv, char * key) {

  int getrandom_success;

  getrandom_success  = syscall(SYS_getrandom, iv, IV_SIZE, GRND_RANDOM);
  if (getrandom_success != IV_SIZE)
  {
    printf("ERROR: getrandom() was unsuccessful");
  }
  printf("IV contains: ");
  for (int i=0; i <= IV_SIZE; i++)
  {
    printf("%x", iv[i]);
  }


  getrandom_success  = syscall(SYS_getrandom, key, KEY_SIZE, GRND_RANDOM);
  if (getrandom_success != KEY_SIZE)
  {
    printf("ERROR: getrandom() was unsuccessful");
  }
  printf("\n\nKEY contains: ");
  for (int i=0; i <= KEY_SIZE; i++)
  {
    printf("%x", key[i]);
  }
  /* A 256 bit key */
   // unsigned char *key_old = (unsigned char *)"01234567890123456789012345678901";
   //
   //  /* A 128 bit IV */
   //  unsigned char *iv_old = (unsigned char *)"0123456789012345";
   //
   //  /* Message to be encrypted */
   //  unsigned char *plaintext =
   //      (unsigned char *)"The quick brown fox jumps over the lazy dog";
   //
   //  /*
   //   * Buffer for ciphertext. Ensure the buffer is long enough for the
   //   * ciphertext which may be longer than the plaintext, depending on the
   //   * algorithm and mode.
   //   */
   //  unsigned char ciphertext[128];
   //
   //  /* Buffer for the decrypted text */
   //  unsigned char decryptedtext[128];
   //
   //  int decryptedtext_len, ciphertext_len;
   //
   //  /* Encrypt the plaintext */
   //  ciphertext_len = encrypt (plaintext, strlen ((char *)plaintext), key, iv,
   //                            ciphertext);
   //
   //  /* Do something useful with the ciphertext here */
   //  printf("Ciphertext is:\n");
   //  BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);
   //
   //  /* Decrypt the ciphertext */
   //  decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,
   //                              decryptedtext);
   //
   //  /* Add a NULL terminator. We are expecting printable text */
   //  decryptedtext[decryptedtext_len] = '\0';
   //
   //  /* Show the decrypted text */
   //  printf("Decrypted text is:\n");
   //  printf("%s\n", decryptedtext);
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
