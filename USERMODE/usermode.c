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
#include <string.h>
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
#define IOCTL_SIZE 48

#define DEBUG 0

//Global Variables for crypto
unsigned char * iv_a;
unsigned char * key_a;
unsigned char * iv_b;
unsigned char * key_b;

int ioctl_set_data(int fd, char * data, char * dev)
{
    int i;
    char c;

    //printf("[+] %s called\n", __FUNCTION__ );

    ioctl(fd, IOCTL_WRITE_TO_KERNEL, data );

    //printf("[+]    Data written to %s: %x\n", dev, data );

    return 0;
}

int ioctl_read_data(int fd, char * data, char * dev)
{
    int i;
    char c;

    //printf("[+] %s called\n", __FUNCTION__ );

    ioctl(fd, IOCTL_READ_FROM_KERNEL, data );

    if (NULL != data[0])
    {
      //printf("[+]    Data read from %s: %x\n", dev, data );
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
      printf("\nERROR: You must enter a device name!\n");
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
        initializeEncryption(keypair_a);

        ret = ioctl_set_data(fd_a, keypair_a, dev_a);

        memset(keypair_b, 0, IOCTL_SIZE);

        //wait until we can read from other device
        printf("\n\nWill wait up to 60 seconds to receive keypair from user B.\n[");
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
          printf("]\nError: Did not receive keypair in time. Closing program.\n");
          return -1;
        }
        printf("]\n");
        for(int i = 0; i < IV_SIZE; i++) {
          iv_b[i] = keypair_b[i];
          iv_a[i] = keypair_a[i];
        }

        for(int i = 0; i < KEY_SIZE; i++) {
          key_b[i] = keypair_b[i + IV_SIZE];
          key_a[i] = keypair_a[i + IV_SIZE];
        }

        memset(read_from_b, 0, MAX_READ_SIZE);


        if(DEBUG) {
          printf("\nIV READ FROM B contains: ");
          for (int i=0; i < IV_SIZE; i++)
          {
            printf("%x", iv_b[i]);
          }

          printf("\n\nKEY READ FROM B contains: ");
          for (int i=0; i < KEY_SIZE; i++)
          {
            printf("%x", key_b[i]);
          }


          printf("\n\nCRYPTO IS DONE\n\n\n\n");
        }

        while(1)
        {
          int len = 0;
          memset(write_to_a, 0, MAX_WRITE_SIZE);
          printf("[Me]: ");
          fgets(write_to_a, MAX_WRITE_SIZE, stdin);
          for (int i = 0; i < MAX_WRITE_SIZE; i++)
          {
            len += 1;
            if (NULL == write_to_a[i])
            {
              break;
            }
          }



          unsigned char ciphertext[MAX_WRITE_SIZE];

          memset(ciphertext, 0, MAX_WRITE_SIZE);
          int ciphertext_len = encrypt (write_to_a, strlen ((char *)write_to_a), key_a, iv_a, ciphertext);

          char buffForLength[5] = {0};
          snprintf(buffForLength, sizeof(buffForLength), "%04d", ciphertext_len);

          char resultToSend[MAX_WRITE_SIZE] = {0};

          memcpy(&resultToSend, buffForLength, 4);
          memcpy(&resultToSend[4], ciphertext, MAX_WRITE_SIZE - 4);

          write(fd_a, resultToSend, MAX_WRITE_SIZE); //4 becuase new line gets taken away during strncat

          memset(resultToSend, 0, MAX_WRITE_SIZE);

          printf("\nWaiting on input from b...\n");
          while(read_from_b[0] == NULL) {
            read(fd_b, read_from_b, MAX_READ_SIZE);
            sleep(0.25);
          }

          char sizeCiphertextString[4] = {0};
          strncpy(sizeCiphertextString, read_from_b, 4);

          int sizeCiphertext = atoi(sizeCiphertextString);

          char middleMan[MAX_READ_SIZE - 4] = {0};

          memcpy(&middleMan[0], &read_from_b[4], sizeCiphertext);

          unsigned char decryptedtext[MAX_READ_SIZE];
          memset(decryptedtext, 0, MAX_WRITE_SIZE);

          int decryptedtext_len = decrypt(middleMan, sizeCiphertext, key_b, iv_b, decryptedtext);
          decryptedtext[decryptedtext_len] = '\0';
          memset(middleMan, 0, MAX_READ_SIZE);

          printf("\nRead from [b]: %s\n", decryptedtext);
          memset(read_from_b, 0, MAX_READ_SIZE);
        }

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

      //init crypto
      initializeEncryption(keypair_b);


      //write it to ourselves
      ret = ioctl_set_data(fd_b, keypair_b, dev_b);

      memset(keypair_a, 0, IOCTL_SIZE);

      //wait until we can read from other device
      printf("\n\nWill wait up to 60 seconds to receive keypair from user A.\n[");
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
      printf("]");

      for(int i = 0; i < IV_SIZE; i++) {
        iv_b[i] = keypair_b[i];
        iv_a[i] = keypair_a[i];
      }

      for(int i = 0; i < KEY_SIZE; i++) {
        key_b[i] = keypair_b[i + IV_SIZE];
        key_a[i] = keypair_a[i + IV_SIZE];
      }

      memset(read_from_a, 0, MAX_READ_SIZE);

      if(DEBUG) {
        printf("\nIV READ FROM A contains: ");
        for (int i=0; i < IV_SIZE; i++)
        {
          printf("%x", iv_a[i]);
        }

        printf("\n\nKEY READ FROM A contains: ");
        for (int i=0; i < KEY_SIZE; i++)
        {
          printf("%x", key_a[i]);
        }

        printf("\n\nCRYPTO IS DONE\n\n\n\n");
      }












      while(1)
      {
        int len = 0;
        printf("\nWaiting on input from a...\n");
        while(read_from_a[0] == NULL) {
          read(fd_a, read_from_a, MAX_READ_SIZE);
          sleep(0.25);
        }

        char sizeCiphertextString[4] = {0};
        strncpy(sizeCiphertextString, read_from_a, 4);

        int sizeCiphertext = atoi(sizeCiphertextString);

        char middleMan[MAX_READ_SIZE - 4] = {0};

        memcpy(&middleMan, &read_from_a[4], sizeCiphertext);


        /* Buffer for the decrypted text */
        unsigned char decryptedtext[MAX_READ_SIZE];
        memset(decryptedtext, 0, MAX_READ_SIZE);

        int decryptedtext_len = decrypt(middleMan, sizeCiphertext, key_a, iv_a, decryptedtext);
        decryptedtext[decryptedtext_len] = '\0';
        memset(middleMan, 0, MAX_READ_SIZE);

        printf("\nRead from [a]: %s\n", decryptedtext);

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

        unsigned char ciphertext[MAX_WRITE_SIZE];

        memset(ciphertext, 0, MAX_WRITE_SIZE);
        int ciphertext_len = encrypt (write_to_b, strlen ((char *)write_to_b), key_b, iv_b, ciphertext);

        char buffForLength[5] = {0};
        snprintf(buffForLength, sizeof(buffForLength), "%04d", ciphertext_len);


        char resultToSend[MAX_WRITE_SIZE] = {0};

        memcpy(&resultToSend, buffForLength, 4);
        memcpy(&resultToSend[4], ciphertext, MAX_WRITE_SIZE - 4);

        write(fd_b, resultToSend, MAX_WRITE_SIZE); //4 becuase new line gets taken away during strncat

        memset(resultToSend, 0, MAX_WRITE_SIZE);
      }

      close(fd_a);
      close(fd_b);
    }
    return 0;
}

void initializeEncryption(unsigned char * keypair) {
  int getrandom_success = 0;
  while (getrandom_success != (IOCTL_SIZE))
  {
    getrandom_success  = syscall(SYS_getrandom, keypair, IOCTL_SIZE, GRND_RANDOM);
    if (getrandom_success != (IOCTL_SIZE))
    {
      printf("\nERROR: getrandom() was unsuccessful\n");
      printf("Will wait 15 seconds before trying again... please move the mouse during the wait to increase entropy.\n\n");
      sleep(15);
    }
  }

  if(DEBUG) {
    printf("Keypair contains: ");
    for (int i=0; i < IOCTL_SIZE; i++)
    {
      printf("%x", keypair[i]);
    }
    printf("IV contains: ");
    for (int i=0; i < IV_SIZE; i++)
    {
      printf("%x", keypair[i]);
    }

    printf("\n\nKEY contains: ");
    for (int i=0; i < KEY_SIZE; i++)
    {
      printf("%x", keypair[i + IV_SIZE]);
    }
  }
}


int encrypt(unsigned char *plaintext, int plaintext_length, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *context;
    int length;
    int ciphertext_length;

    //this should return a valid pointer if it worked, if not then NULL
    if(!(context = EVP_CIPHER_CTX_new()))
        return -1;

    if(!EVP_EncryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv))
        return -2;

    if(!EVP_EncryptUpdate(context, ciphertext, &length, plaintext, plaintext_length))
        return -3;

    ciphertext_length = length;

    if(!EVP_EncryptFinal_ex(context, ciphertext + length, &length))
        return -4;

    ciphertext_length += length;

    EVP_CIPHER_CTX_free(context);

    return ciphertext_length;
}

int decrypt(unsigned char *ciphertext, int ciphertext_length, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *context;
    int length;
    int plaintext_length;

    //this should return a valid pointer if it worked, if not then NULL
    if(!(context = EVP_CIPHER_CTX_new()))
        return -1;

    if(!EVP_DecryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv))
        return -2;

    if(!EVP_DecryptUpdate(context, plaintext, &length, ciphertext, ciphertext_length))
        return -3;

    plaintext_length = length;

    if(!EVP_DecryptFinal_ex(context, plaintext + length, &length))
        return -4;

    plaintext_length += length;

    EVP_CIPHER_CTX_free(context);

    return plaintext_length;
}
