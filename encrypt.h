/* Include file for high-level encryption routines.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define NICKLEN 9

extern int encrypt(const char *src, int len, char *dest, int size);
extern int encrypt_in_place(char *buf, int size);
extern int check_password(const char *plaintext, const char *password);
extern void tea(unsigned long v[],unsigned long k[], unsigned long x[]);
extern unsigned int base64toint(const char *str);
extern const char *inttobase64(unsigned int i);
