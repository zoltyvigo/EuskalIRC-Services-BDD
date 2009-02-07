/*
 * Manejo de las BDD de Hispano
 * 
 * Parte de código extraida de cifranick.c Hispano.
 *
 * Aritz, aritz@itxaropena.org
 * Itxaropena Garapen Taldea - www.itxaropena.org
 */


#include "services.h"
#include "pseudo.h"

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>


#define NUMNICKLOG 6
#define NICKLEN 15
#define NUMNICKBASE 64          /* (2 << NUMNICKLOG) */
#define NUMNICKMASK 63          /* (NUMNICKBASE-1) */

#ifdef IRC_UNDERNET_P10

const char tabla_todas[] = {
'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l',
'm', 'n', 'o', 'p', 'q', 'r', 
's', 't', 'u', 'v', 'w', 'x', 
'y', 'z'
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
void meter_dato(int cual, char *dato1, char *dato2)
{

// 0 jaso badu bigarren datuan ezabatu egingo dugu tablatik.
if (dato2 == 0) {
send_cmd(NULL, "%c DB * %d %c %s", convert2y[ServerNumerico], tablas[cual], tabla_todas[cual], dato1);
} else {
send_cmd(NULL, "%c DB * %d %c %s :%s", convert2y[ServerNumerico], tablas[cual], tabla_todas[cual], dato1, dato2);
}
// Inkrementatu
tablas[cual]++;
}

void meter_dato2(char *cual, char *dato1, char *dato2)
{
int i;
int tablazki = 99;

for (i=0; i<TKOPURU; i++) {
if (tabla_todas[i] == *cual)
tablazki = i;
}

//canaladmins(s_OperServ, "%c DB * %d %c %s", convert2y[ServerNumerico], tablak[tablazki], tabla_guztiak[tablazki], datu1);
// 0 jaso badu bigarren datuan ezabatu egingo dugu tablatik.
if (dato2 == 0) {
send_cmd(NULL, "%c DB * %d %c %s", convert2y[ServerNumerico], tablas[tablazki], tabla_todas[tablazki], dato1);
} else {
send_cmd(NULL, "%c DB * %d %c %s :%s", convert2y[ServerNumerico], tablas[tablazki], tabla_todas[tablazki], dato1, dato2);
}
// Inkrementatu
tablas[tablazki]++;
}

void datobase(char cual, char *dato1, char *dato2)
{
// letra hori zein tablari dagokio?
int i;
int tablazki = 0;

for (i=0; i<TKOPURU; i++) {
if (tabla_todas[i] == cual)
tablazki = i;
}

meter_dato(tablazki, dato1, dato2);

}

int encontrardb(char cual)
{
// letra hori zein tablari dagokio?
int i;
int tablazki = 99;

for (i=0; i<TKOPURU; i++) {
if (tabla_todas[i] == cual)
tablazki = i;
}

return tablazki;
}



/*
 * TEA (cifrado)
 *
 * Cifra 64 bits de datos, usando clave de 64 bits (los 64 bits superiores son cero)
 * Se cifra v[0]^x[0], v[1]^x[1], para poder hacer CBC facilmente.
 *
 */
void cifrado_tea(unsigned int v[], unsigned int k[], unsigned int x[])
{
  unsigned int y = v[0] ^ x[0], z = v[1] ^ x[1], sum = 0, delta = 0x9E3779B9;
  unsigned int a = k[0], b = k[1], n = 32;
  unsigned int c = 0, d = 0;

  while (n-- > 0)
  {
    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;
  }

  x[0] = y;
  x[1] = z;
}

unsigned int base64toint_bdd(const char *s)
{
  unsigned int i = convert2n[(unsigned char)*s++];
  while (*s)
  {
    i <<= NUMNICKLOG;
    i += convert2n[(unsigned char)*s++];
  }
  return i;
}

const char *inttobase64_bdd(char *buf, unsigned int v, unsigned int count)
{
  buf[count] = '\0';
  while (count > 0)
  {
    buf[--count] = convert2y[(v & NUMNICKMASK)];
    v >>= NUMNICKLOG;
  }
  return buf;
}



/*
 * ezizen_eragiketa
 * Suspender o prohibir un nick
 * Aritz - aritz@itxaropena.org
 * 
 * Valores de ERAG:
 * erag = 0 : Suspende el nick.
 * erag > 0 : Prohibe el nick.
 */
void ezizen_eragiketa(char *elnick, char *password, int erag)
{
    unsigned int v[2], k[2], x[2];
    int cont = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *p = (unsigned int *)tmpnick; /* int == 32 bits */

    char nick[NICKLEN + 1];    /* Nick normalizado */
    char clave[12 + 1];                /* Clave encriptada */
    int i = 0;

    strcpy(nick, elnick);
    nick[NICKLEN] = '\0';


     /* Normalizar nick */
    while (nick[i] != 0)
    {
       nick[i] = toLower(nick[i]);
       i++;
    }  

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nick ,sizeof(tmpnick) - 1);

    memset(tmppass, 0, sizeof(tmppass));
    strncpy(tmppass, password, sizeof(tmppass) - 1);

    /* relleno   ->   123456789012 */
    strncat(tmppass, "AAAAAAAAAAAA", sizeof(tmppass) - strlen(tmppass) -1);

    x[0] = x[1] = 0;
    
    k[1] = base64toint_bdd(tmppass + 6);
    tmppass[6] = '\0';
    k[0] = base64toint_bdd(tmppass);

    while(cont--)
    {
      v[0] = ntohl(*p++);      /* 32 bits */
      v[1] = ntohl(*p++);      /* 32 bits */
      cifrado_tea(v, k, x);
    }

    inttobase64_bdd(clave, x[0], 6);
    inttobase64_bdd(clave + 6, x[1], 6);





if (erag == 1) {
    clave[sizeof(clave)+1] = '+';
    meter_dato(13, nick, clave);
} else 
    meter_dato(13, nick, "*");


}

/*
 * vhost_aldaketa
 * Cambios de vhost
 * Aritz - aritz@itxaropena.org
 * 
 * Valores de ERAG:
 * erag = 0 : Elimina el vhost en tabla w
 * erag = 1 : Inserta registro en tabla w
 * erag = 2 : Inserta registro en tabla v 
 * erag > 2 : Elimina el vhost en tabla v
 */
void vhost_aldaketa(char *elnick, char *vhost, int erag)
{
    unsigned int v[2], k[2], x[2];
    int cont = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *p = (unsigned int *)tmpnick; /* int == 32 bits */

    char nick[NICKLEN + 1];    /* Nick normalizado */
    char clave[12 + 1];                /* Clave encriptada */
    int i = 0;

    strcpy(nick, elnick);
    nick[NICKLEN] = '\0';


     /* Normalizar nick */
    while (nick[i] != 0)
    {
       nick[i] = toLower(nick[i]);
       i++;
    }  

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nick ,sizeof(tmpnick) - 1);

  // Procesar solicitud

if (erag == 0)
    meter_dato(22, nick, 0);
else if (erag == 1)
    meter_dato(22, nick, vhost);
else if (erag == 2) {
    meter_dato(21, nick, vhost);
} else 
    meter_dato(21, nick, 0);


}

/*
 * ep_tablan
 * Normalizar e insertar nick en una tabla con su pass
 * Aritz - aritz@itxaropena.org
 * 
 * 27-03-2008
 */
void ep_tablan(char *elnick, char *password, char cual)
{
    unsigned int v[2], k[2], x[2];
    int cont = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *p = (unsigned int *)tmpnick; /* int == 32 bits */

    char nick[NICKLEN + 1];    /* Nick normalizado */
    char clave[12 + 1];                /* Clave encriptada */
    int i = 0;

    strcpy(nick, elnick);
    nick[NICKLEN] = '\0';


     /* Normalizar nick */
    while (nick[i] != 0)
    {
       nick[i] = toLower(nick[i]);
       i++;
    }  

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nick ,sizeof(tmpnick) - 1);

    memset(tmppass, 0, sizeof(tmppass));
    strncpy(tmppass, password, sizeof(tmppass) - 1);

    /* relleno   ->   123456789012 */
    strncat(tmppass, "AAAAAAAAAAAA", sizeof(tmppass) - strlen(tmppass) -1);

    x[0] = x[1] = 0;
    
    k[1] = base64toint_bdd(tmppass + 6);
    tmppass[6] = '\0';
    k[0] = base64toint_bdd(tmppass);

    while(cont--)
    {
      v[0] = ntohl(*p++);      /* 32 bits */
      v[1] = ntohl(*p++);      /* 32 bits */
      cifrado_tea(v, k, x);
    }

    inttobase64_bdd(clave, x[0], 6);
    inttobase64_bdd(clave + 6, x[1], 6);

// Datua sartu
meter_dato(encontrardb(cual), nick, clave);
//datubasera(zein, nick, clave);

}

/*
 * ed_tablan
 * Normalizar nick e insertar datos en la tabla
 * Aritz - aritz@itxaropena.org
 * 
 * 27-03-2008
 */
void ed_tablan(char *elnick, char *dato, char cual)
{
    unsigned int v[2], k[2], x[2];
    int cont = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *p = (unsigned int *)tmpnick; /* int == 32 bits */

    char nick[NICKLEN + 1];    /* Nick normalizado */
    char clave[12 + 1];                /* Clave encriptada */
    int i = 0;


    strcpy(nick, elnick);
    nick[NICKLEN] = '\0';


     /* Normalizar nick */
    while (nick[i] != 0)
    {
       nick[i] = toLower(nick[i]);
       i++;
    }  

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nick ,sizeof(tmpnick) - 1);

// datu basera sartu datua
meter_dato(encontrardb(cual), nick, dato);
//datubasera(zein, nick, datua);

}

/*
 * dbchan_reg
 * Normalizar nick e insertar datos en la tabla
 * Aritz - aritz@itxaropena.org
 * 
 * 27-03-2008
 */
void dbchan_reg(char *elnick, char *dato, char cual)
{
    unsigned int v[2], k[2], x[2];
    int cont = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *p = (unsigned int *)tmpnick; /* int == 32 bits */

    char nick[NICKLEN + 1];    /* Nick normalizado */
    char clave[12 + 1];                /* Clave encriptada */
    int i = 0;


//    strcpy(nick, ezizena);
 //   nick[NICKLEN] = '\0';

/*
  
    while (nick[i] != 0)
    {
       nick[i] = toLower(nick[i]);
       i++;
    }  

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nick ,sizeof(tmpnick) - 1);
*/
// datu basera sartu datua
meter_dato(encontrardb(cual), dato, elnick);
//datubasera(zein, nick, datua);

}
#endif