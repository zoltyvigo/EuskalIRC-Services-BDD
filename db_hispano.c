/* Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * Modulo Soporte BDD hecho por zoltan <zolty@ctv.es>
 * 25-09-2000
 *
 * Soporte para Bases de Datos Distribuidas originario de ESNET-HISPANO
 * y soportados tambien para Globalchat y UpWorld
 *
 * El soporte solo esta disponible para Undernet P10         
 *
 * El codigo ha sido tomado del ircu de ESNET-HISPANO
 * Web de los devels de iRC-Hispano para documentacion y bajar ircu con
 * soporte de DB http://devel.irc-hispano.org
 */
 
#ifdef DB_NETWORKS

#include "services.h"
#include "pseudo.h"

#define NICKLEN 9

/*
 * TEA (cifrado)
 *
 * Cifra 64 bits de datos, usando clave de 64 bits (los 64 bits superiores 
 * son cero)
 * Se cifra v[0]^x[0], v[1]^x[1], para poder hacer CBC facilmente.
 *
 */

void tea(unsigned long v[],unsigned long k[],unsigned long x[])
{
    unsigned long y=v[0]^x[0],z=v[1]^x[1],sum=0,delta=0x9E3779B9;
    unsigned long a=k[0],b=k[1],n=32;
    unsigned long c=0,d=0;
                  
    while(n-->0) {
        sum += delta;
        y += (z << 4)+a ^ z+sum ^ (z >> 5)+b;
        z += (y << 4)+c ^ y+sum ^ (y >> 5)+d;
    }
    x[0]=y; x[1]=z;
}
 
void cifrado_tea(char *nick, char *pass)
{
    unsigned int v[2], k[2], x[2];
    int cont = (NICKLEN + 8) / 8;
    char tmpnick[8 * ((NICKLEN + 8) / 8) + 1];
    char tmppass[12 + 1];
    unsigned int *p=(unsigned long *)tmpnick;
    
    char passtea[14];
    char passteatemp1[7];
    char passteatemp2[7];                

    memset(tmpnick,0,sizeof(tmpnick));
    strncpy(tmpnick,nick,sizeof(tmpnick));
        
    memset(tmppass,0,sizeof(tmppass));
    strncpy(tmppass,pass,sizeof(tmppass));
              
    x[0]=x[1]=0;
    k[0]=base64toint(tmppass);
    k[1]=base64toint(tmppass+6);

    while(cont--) {
        v[0]=ntohl(*p++);
        v[1]=ntohl(*p++);
        tea(v,k,x);
    }

    snprintf(passtea1, sizeof(passtea1), "%s", inttobase64(x[0]));
    snprintf(passtea2, sizeof(passtea1), "%s", inttobase64(x[1]));
    snprintf(passtea, sizeof(passtea), "%s%s", passtea1, passtea2);    
    
    return passtea;
}    

/*************************************************************************/
 /* Handleamos el comando DB para pillar las series :) 
  *
  *      source =  Numerico server origen
  *      av[0] = Suele ser * => Todos los servers o el nombre server
  *      av[1] = 0 
  *      av[2] = Comando utilizado, J para listar, D para drop, etc..
  *      av[3] = Numero de serie
  *      av[4] = Tabla, si es un 2, tabla n
  *
  *  Significado de las tablas av[4]
  * 
  * 2 y n Tabla n de Nicks
  *     b Tabla de Bots Virtuales
  *     c Tabla de Canales
  *     i Tabla de Ilines
  *     o Tabla de Operadores
  *     v Tabla de Vhosts (ip virtual)
  *     t Tabla de nicks temporales (Solo en iRC-Hispano)
  *     m Tabla de Administradores (Solo en UpWorld)
  *     d Tabla de Developers (Solo en UpWorld)
  */
void do_db(const char *source, int ac, char **av)
{
#ifdef AUN_NO_RULA
/* Solo nos interesa un listado (J), no los demas (D,....) */
   if (strcmp(av[3], "J")  == 0) {
       if (strcmp(av[4], "2")  == 0)
           = atol(av[3]);
       if (strcmp(av[4], "b")  == 0)
                  = atol(av[3]);    
       if (strcmp(av[4], "c")  == 0)
                  = atol(av[3]);    
       if (strcmp(av[4], "i")  == 0)
                  = atol(av[3]);                  
       if (strcmp(av[4], "n")  == 0)
                  = atol(av[3]);
       if (strcmp(av[4], "o")  == 0)
                  = atol(av[3]);                                       
       if (strcmp(av[4], "v")  == 0)
                  = atol(av[3]);                  
#ifdef DB_HISPANO
       if (strcmp(av[4], "t")  == 0)
             = atol(av[3]);                  
#endif
#ifdef DB_UPWORLD
       if (strcmp(av[4], "d")  == 0)
                  = atol(av[3]);                  
#endif                  
   }
#endif   
}                 
                                            
#endif /* DB_NETWORKS */