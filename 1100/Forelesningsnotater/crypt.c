#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *encrypt(char *plain, int shift)
{
    int i;
    char c;
    char *cipher;

    // Duplicate plain text
    cipher = strdup(plain);

    for (i = 0; plain[i] != 0; i = i + 1) {
        // Skip anything but 'a..z'
        c = plain[i];
        if (c < 'a' || c > 'z') {
            // Assignment not needed since cipher is a copy of the plain array.
            // But initializing all elements of the cipher array inside the for loop
            // makes the code easier to read.
            cipher[i] = c;
            continue;
        }
        
        // Encrypt
        c = c - 'a';
        c = (c + shift) % 26;
        if (c < 0)              // Read up on C semantics for modulo of a negative number
            c = c + 26;
        c = c + 'a';

        // Store encrypted character
        cipher[i] = c;
    }
    cipher[i] = 0;
    return cipher;
}


char *decrypt(char *cipher, int shift)
{
    int i;
    char c;
    char *plain;

    // Duplicate ciphertext
    plain = strdup(cipher);

    for (i = 0; cipher[i] != 0; i = i + 1) {
        // Skip anything but 'a..z'
        c = cipher[i];
        if (c < 'a' || c > 'z') {
            // Assignment not needed since plain is a copy of the cipher array.
            // But initializing all elements of the plain array inside the for loop
            // makes the code easier to read.
            plain[i] = c;
            continue;
        }
        
        // Decrypt
        c = c - 'a';
        c = (c - shift) % 26;
        if (c < 0)              // Read up on C semantics for modulo of a negative number
            c = c + 26;
        c = c + 'a';

        // Store decrypted character
        plain[i] = c;
    }
    return plain;
}


int main(void)
{
    char *plaintext;
    char *ciphertext;
    char *decrypted;

    plaintext = "dette er hemmelig";
    ciphertext = encrypt(plaintext, 25);
    decrypted = decrypt(ciphertext, 25);
   
    printf("Plain    : '%s'\n", plaintext);
    printf("Cipher   : '%s'\n", ciphertext);
    printf("Decrypted: '%s'\n", decrypted);

    return 0;
}
