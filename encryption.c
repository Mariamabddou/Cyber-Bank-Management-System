#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AES-256-CBC.
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h> //Random Key, IV

// mainVariables
#define AES_KEY_SIZE 32 // 32 Byte: 256-bit key
#define AES_IV_SIZE 16  // Vector Initialization IV: size for AES-CBC 16 Byte = 128-bit
#define PASSWORD_LENGTH 8

char password[PASSWORD_LENGTH + 1];

//
void encryptAES(const unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext, int *ciphertext_len)
{
    // New context, error? handleErrors
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        handleErrors();
    // Initialization for the process: use EVP_aes_256_cbc()
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
        handleErrors();

    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1)
        handleErrors();
    *ciphertext_len = len;
    // End Encryption and Add Padding
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
        handleErrors();
    *ciphertext_len += len;
    // clean the memory
    EVP_CIPHER_CTX_free(ctx);
}

void decryptAES(const unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext, int *plaintext_len)
{
    // New context, error? handleErrors
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        handleErrors();
    // Initialization for the process: use EVP_aes_256_cbc()
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
        handleErrors();

    int len;
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1)
        handleErrors();
    *plaintext_len = len;
    // End Encryption and remove Padding
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1)
        handleErrors();
    *plaintext_len += len;
    // clean the memory
    EVP_CIPHER_CTX_free(ctx);
}

const char* currentTime()
{
    static char timeStr[30];
    time_t t;
    time(&t);
    strftime(timeStr, sizeof(timeStr), "[%a %b %d %Y - %H:%M:%S]", localtime(&t));

    return timeStr;
}

char* passwordChecker()
{
    char ch;
    char symbols[] = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/";

    while (1)
    {
        int i = 0;
        int hasDigit = 0, hasUpper = 0, hasLower = 0, hasSymbol = 0;

        printf("Enter Valid Password: ");
        while (i < PASSWORD_LENGTH)
        {

            ch = getch();

            if (ch == '\r')// && i >= PASSWORD_LENGTH)
                break;

            if (ch == '\b' && i > 0)
            {
                i--;
                printf("\b \b");
                continue;
            }


            if (ch >= 32 && ch <= 126)
            {
                password[i++] = ch;
                printf("*");

                if (isdigit(ch)) hasDigit = 1;
                else if (isupper(ch)) hasUpper = 1;
                else if (islower(ch)) hasLower = 1;
                else if (strchr(symbols, ch)) hasSymbol = 1;
            }
        }
        password[i] = '\0';
        if (hasDigit && hasUpper && hasLower && hasSymbol)
        {
            //printf("\nSuccess!\n");
            //printf("\nYour Password is: %s\n",password);
            return password;
            break;
        }
        else
        {
            printf("\nNot Valid!\n");
        }
    }

}
