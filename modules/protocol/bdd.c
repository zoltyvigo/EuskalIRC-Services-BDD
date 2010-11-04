/*
 * Control de la Base de Datos Distribuida del IRCD de iRC-Hispano
 * archivo escrito por David Martin (Aka [x])                     
 *
 * Este programa es software libre. Puede redistribuirlo y/o modificarlo
 * bajo los términos de la Licencia Pública General de GNU según es publicada
 * por la Free Software Foundation.
 *
 * ATENCIÓN: Parte de este código está extraido del cifranick.c del
 * IRCd de IRC-Hispano, escrito originalmente por sus desarrolladores
 * (las rutinas de TEA)
 */
#define NUMNICKLOG 6
#define NICKLEN 15
#define NUMNICKBASE 64          /* (2 << NUMNICKLOG) */
#define NUMNICKMASK 63          /* (NUMNICKBASE-1) */
#include "services.h"
#include "conffile.h"
#include "databases.h"
#include "messages.h"
#include "modules.h"
#include "language.h"
#include "version.h"

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static const char convert2y[] = {
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
  'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
  'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
  'w','x','y','z','0','1','2','3','4','5','6','7','8','9','[',']'
};

static const unsigned int convert2n[] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
   0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,62, 0,63, 0, 0,
   0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned int base64toint(const char *s)
{
  unsigned int i = convert2n[(unsigned char)*s++];
  while (*s)
  {
    i <<= NUMNICKLOG;
    i += convert2n[(unsigned char)*s++];
  }
  return i;
}

const char *inttobase64(char *buf, unsigned int v, unsigned int count)
{
  buf[count] = '\0';
  while (count > 0)
  {
    buf[--count] = convert2y[(v & NUMNICKMASK)];
    v >>= NUMNICKLOG;
  }
  return buf;
}

const char ANTL_tolower_tab[] = {
       /* x00-x07 */ '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
       /* x08-x0f */ '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
       /* x10-x17 */ '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
       /* x18-x1f */ '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
       /* ' '-x27 */    ' ',    '!',    '"',    '#',    '$',    '%',    '&', '\x27',
       /* '('-'/' */    '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
       /* '0'-'7' */    '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
       /* '8'-'?' */    '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
       /* '@'-'G' */    '@',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
       /* 'H'-'O' */    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
       /* 'P'-'W' */    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
       /* 'X'-'_' */    'x',    'y',    'z',    '{',    '|',    '}',    '~',    '_',
       /* '`'-'g' */    '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
       /* 'h'-'o' */    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
       /* 'p'-'w' */    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
       /* 'x'-x7f */    'x',    'y',    'z',    '{',    '|',    '}',    '~', '\x7f'
};
#define stoLower(c) (ANTL_tolower_tab[(c)])


/*
 * TEA (cifrado)
 *
 * Cifra 64 bits de datos, usando clave de 64 bits (los 64 bits superiores son cero)
 * Se cifra v[0]^x[0], v[1]^x[1], para poder hacer CBC facilmente.
 *
 */
void tea(unsigned int v[], unsigned int k[], unsigned int x[])
{
  unsigned int y = v[0] ^ x[0], z = v[1] ^ x[1], sum = 0, delta = 0x9E3779B9;
  unsigned int a = k[0], b = k[1], n = 32;
  unsigned int c = 0, d = 0;

  while (n-- > 0)
  {
    sum += delta;
    y += ((z << 4) + a) ^ ((z + sum) ^ ((z >> 5) + b));
    z += ((y << 4) + c) ^ ((y + sum) ^ ((y >> 5) + d));
  }

  x[0] = y;
  x[1] = z;
}




static unsigned int tabla_n;
static unsigned int tabla_c; /* para los canales persistentes*/
static unsigned int tabla_r; /*para redirección automática de canales*/
static unsigned int tabla_j; /*para jupeos de nicks*/
static unsigned int tabla_v;
static unsigned int tabla_o;
static unsigned int tabla_w;
static unsigned int tabla_i;
static unsigned int tabla_z;



void do_write_bdd(char *entrada, int tabla, const char *valor, ...)
{
    
    unsigned int v[2], k[2], x[2];
    int conts = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *ps = (unsigned int *)tmpnick; /* int == 32 bits */

    char nicks[NICKLEN + 1];    /* Nick normalizado */
    char clave[12 + 1];                /* Clave encriptada */
    int ir = 0;




    strcpy(nicks, entrada);
    nicks[NICKLEN] = '\0';

    while (nicks[ir] != 0)
    {
       nicks[ir] = stoLower(nicks[ir]);
       ir++;
    }

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nicks ,sizeof(tmpnick) - 1);

    memset(tmppass, 0, sizeof(tmppass));
    strncpy(tmppass, valor, sizeof(tmppass) - 1);

    /* relleno   ->   123456789012 */
    strncat(tmppass, "AAAAAAAAAAAA", sizeof(tmppass) - strlen(tmppass) -1);

    x[0] = x[1] = 0;

    k[1] = base64toint(tmppass + 6);
    tmppass[6] = '\0';
    k[0] = base64toint(tmppass);

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nicks ,sizeof(tmpnick) - 1);

    memset(tmppass, 0, sizeof(tmppass));
    strncpy(tmppass, valor, sizeof(tmppass) - 1);

    /* relleno   ->   123456789012 */
    strncat(tmppass, "AAAAAAAAAAAA", sizeof(tmppass) - strlen(tmppass) -1);

    x[0] = x[1] = 0;

    k[1] = base64toint(tmppass + 6);
    tmppass[6] = '\0';
    k[0] = base64toint(tmppass);

    while(conts--)
    {
      v[0] = ntohl(*ps++);      /* 32 bits */
      v[1] = ntohl(*ps++);      /* 32 bits */
      tea(v, k, x);
    }

    inttobase64(clave, x[0], 6);
    inttobase64(clave + 6, x[1], 6);



	 if (tabla == 1) {
		send_cmd(NULL, "DB * %d n %s :%s", tabla_n, nicks, clave);
/*		tab[0] = 'n';
		reg = tabla_n;
		ent = nicks;
		val = clave; */
	 	tabla_n++;
	 } else if (tabla == 15) {
                send_cmd(NULL, "DB * %d n %s :", tabla_n, nicks);
	 	tabla_n++;
	 } else if (tabla == 16) {
	 	send_cmd(NULL, "DB * %d n %s :%s+", tabla_n, nicks, clave);
	 	tabla_n++;
	 } else if (tabla == 17) {
	 	send_cmd(NULL, "DB * %d n %s :AAAAAAAAA", tabla_n, nicks);
		tabla_n++;
	 } else if (tabla == 2) {
	 	send_cmd(NULL, "DB * %d v %s :%s", tabla_v, nicks, valor);
		tabla_v++;
	} else if (tabla == 22) {
		send_cmd(NULL, "DB * %d v %s :12%s%s", tabla_v, nicks, nicks, OperHost); 
	        tabla_v++;
	 } else if (tabla == 23){
	 	send_cmd(NULL, "DB * %d v %s :4%s%s", tabla_v, nicks, nicks, AdminHost);
	 	tabla_v++;
	 } else if (tabla == 24){
	 	send_cmd(NULL, "DB * %d v %s :4%s.Devel.Services", tabla_v, nicks, nicks);
		tabla_v++;
	  } else if (tabla == 25){
	 	send_cmd(NULL, "DB * %d v %s :3%s.Patrocinador.Services", tabla_v, nicks, nicks);
	 	tabla_v++;
	  } else if (tabla == 26){
	 	send_cmd(NULL, "DB * %d v %s :4%s.CoAdmin.Services", tabla_v, nicks, nicks);
	 	tabla_v++;
	 } else if (tabla == 3) {
	 	send_cmd(NULL, "DB * %d o %s :%s", tabla_o, nicks, valor);
		tabla_o++;
	 } else  if (tabla == 4) {
	 	send_cmd(NULL, "DB * %d w %s :%s", tabla_w, nicks, valor);
		tabla_w++;
	 } else  if (tabla == 5) {
	 	send_cmd(NULL, "DB * %d i %s :%s", tabla_i, entrada, valor);
		tabla_w++;
	 } else  if (tabla == 6) {
	 	send_cmd(NULL, "DB * %d z %s :%s", tabla_z, entrada, valor);
		tabla_z++;
	 
          } else  if (tabla == 7) {
	 	send_cmd(NULL, "DB * %d c %s :%s", tabla_c, entrada, valor);
		tabla_c++;
	 
	 } else  if (tabla == 9) {
	 	send_cmd(NULL, "DB * %d r %s :%s", tabla_r, entrada, valor);
		tabla_r++;
	 } else  if (tabla == 10) {
	 	send_cmd(NULL, "DB * %d j %s :%s", tabla_j, entrada, valor);
		tabla_r++;
	 
	 }
	 
//send_cmd(NULL, "DB * %d %s %s :%s", reg, tab, ent, val);
send_cmd(NULL, "STATS b");	

}
void do_write_canal(char *entrada, int tabla, const char *valor,const char *param, ...)
{
if (tabla == 8) {
	 	send_cmd(NULL, "DB * %d c %s :%s:%s", tabla_c, entrada, valor,param);
		tabla_c++;
}
send_cmd(NULL, "STATS b");
}

void do_count_bdd(int tabla, unsigned int valor)
{

	if (tabla == 1)
		tabla_n = valor +1;
			
	if (tabla == 2)
		tabla_v = valor +1;
		
	if (tabla == 3)
		tabla_o = valor +1;

	if (tabla == 4)
		tabla_w = valor +1;

	if (tabla == 5)
		tabla_i = valor +1;
	if (tabla == 6)
		tabla_z = valor +1;
        if (tabla == 7)
		tabla_c = valor +1;
	 if (tabla == 8)
		tabla_c = valor +1;
	 if (tabla == 9)
		tabla_r = valor +1;
	if (tabla == 10)
		tabla_j = valor +1;
}






