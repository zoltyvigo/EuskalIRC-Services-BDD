/*
 * Control de la Base de Datos Distribuida del IRCD de iRC-Hispano
 * archivo escrito por David Martin (Aka [x])                     
 *
 * Este programa es software libre. Puede redistribuirlo y/o modificarlo
 * bajo los t�rminos de la Licencia P�blica General de GNU seg�n es publicada
 * por la Free Software Foundation.
 *
 * ATENCI�N: Parte de este c�digo est� extraido del cifranick.c del
 * IRCd de IRC-Hispano, escrito originalmente por sus desarrolladores
 * (las rutinas de TEA)
 */
#define NUMNICKLOG 6
#define NICKLEN 30
#define NUMNICKBASE 64          /* (2 << NUMNICKLOG) */
#define NUMNICKMASK 63          /* (NUMNICKBASE-1) */
#include "services.h"
#include "pseudo.h"

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

 #if defined(IRC_UNDERNET_P09)

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
static unsigned int tabla_r; /*para redirecci�n autom�tica de canales*/
static unsigned int tabla_j; /*para jupeos de nicks*/
static unsigned int tabla_u; /*para uworld*/
static unsigned int tabla_v;
static unsigned int tabla_o;
static unsigned int tabla_w;
static unsigned int tabla_i;
static unsigned int tabla_z;
static void compactar_tablas(User *u);
static void regenerar_clave(User *u);
static void tocar_tablas(User *u);
static void actualizar_contadores(User *u);
static void do_help(User *u);


void do_write_bdd(char *entrada, int tabla, const char *valor, ...)
{
    
    unsigned int v[2], k[2], x[2];
    
    char *nick = entrada;
    int longitud_nick = strlen(nick);
    /* Para nicks <16 uso cont 2 para el resto lo calculo */
    int cont=(longitud_nick < 16) ? 2 : ((longitud_nick + 8) / 8);
    char tmpnick[8 * cont + 1];
    char tmppass[12 + 1];
    unsigned int *p = (unsigned int *)tmpnick; /* int == 32 bits */

    char clave[12 + 1];                /* Clave encriptada */
    int i = 0;

    /* Normalizar nick */
    while (nick[i] != 0)
    {
       nick[i] = toLower((int) nick[i]);
       i++;
    }

    memset(tmpnick, 0, sizeof(tmpnick));
    strncpy(tmpnick, nick ,sizeof(tmpnick) - 1);

    memset(tmppass, 0, sizeof(tmppass));
    strncpy(tmppass, valor, sizeof(tmppass) - 1);

    /* relleno   ->   123456789012 */
    strncat(tmppass, "AAAAAAAAAAAA", sizeof(tmppass) - strlen(tmppass) -1);

    x[0] = x[1] = 0;

    k[1] = base64toint(tmppass + 6);
    tmppass[6] = '\0';
    k[0] = base64toint(tmppass);

    while(cont--)
    {
      v[0] = ntohl(*p++);      /* 32 bits */
      v[1] = ntohl(*p++);      /* 32 bits */
      tea(v, k, x);
    }

    inttobase64(clave, x[0], 6);
    inttobase64(clave + 6, x[1], 6);
    
	 if (tabla == 1) {
		send_cmd(NULL, "DB * %d n %s :%s", tabla_n, nick, clave);
	 	tabla_n++;
	 } else if (tabla == 15) {
                send_cmd(NULL, "DB * %d n %s :", tabla_n, nick);
	 	tabla_n++;
	 } else if (tabla == 16) {
	 	send_cmd(NULL, "DB * %d n %s :%s+", tabla_n, nick, clave);
	 	tabla_n++;
	 } else if (tabla == 17) {
	 	send_cmd(NULL, "DB * %d n %s :AAAAAAAAA", tabla_n, nick);
		tabla_n++;
	 } else if (tabla == 2) {
	 	send_cmd(NULL, "DB * %d v %s :%s", tabla_v, nick, valor);
		tabla_v++;
	 } else if (tabla == 22) {
		send_cmd(NULL, "DB * %d v %s :%s%s.%s", tabla_v, nick, OperColor,nick, OperHost); 
	        tabla_v++;
	 } else if (tabla == 23){
	 	send_cmd(NULL, "DB * %d v %s :%s%s.%s", tabla_v, nick,AdminColor, nick, AdminHost);
	 	tabla_v++;
	 } else if (tabla == 24){
	 	send_cmd(NULL, "DB * %d v %s :%s%s.%s", tabla_v, nick,DevelColor, nick, DevelHost);
		tabla_v++;
	  } else if (tabla == 25){
	 	send_cmd(NULL, "DB * %d v %s :%s%s.%s", tabla_v, nick,PatrocinaColor, nick, PatrocinaHost);
	 	tabla_v++;
	  } else if (tabla == 26){
	 	send_cmd(NULL, "DB * %d v %s :%s%s.%s", tabla_v, nick, CoAdminColor,nick, CoAdminHost);
	 	tabla_v++;
	 } else if (tabla == 27){
	 	send_cmd(NULL, "DB * %d v %s :%s%s.%s", tabla_v, nick, RootColor,nick, RootHost);
	 	tabla_v++;
	 } else if (tabla == 3) {
	 	send_cmd(NULL, "DB * %d o %s :%s", tabla_o, nick, valor);
		tabla_o++;
	 } else  if (tabla == 4) {
	 	send_cmd(NULL, "DB * %d w %s :%s", tabla_w, nick, valor);
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
	 } else  if (tabla == 11) {
	 	send_cmd(NULL, "DB * %d u %s :%s", tabla_u, entrada, valor);
		tabla_u++;
	 
	 }
	 
/*send_cmd(NULL, "DB * %d %s %s :%s", reg, tab, ent, val);*/
send_cmd(s_BddServ, "STATS b");	

}
void do_write_canal(char *entrada, int tabla, const char *valor,const char *param, ...)
{
if (tabla == 8) {
	 	send_cmd(NULL, "DB * %d c %s :%s:%s", tabla_c, entrada, valor,param);
		tabla_c++;
}
send_cmd(s_BddServ, "STATS b");
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
	if (tabla == 11)
		tabla_u = valor +1;
}

static Command cmds[] = {
	{ "HELP",	do_help,		NULL,			-1,	-1,-1,-1,-1 },
	{ "COMPACTAR",	compactar_tablas,	is_services_oper,	BDD_HELP_COMPACTAR,	-1,-1,-1,-1 },
	{ "REGENERAR",	regenerar_clave,	is_services_admin, 	BDD_HELP_REGENERAR,	-1,-1,-1,-1 },
	{ "TOCAR",	tocar_tablas,		is_services_admin,	BDD_HELP_TOCAR,	-1,-1,-1,-1 },
	{ "ACTUALIZAR",	actualizar_contadores,	is_services_oper,	BDD_HELP_ACTUALIZAR,	-1,-1,-1,-1 },
	{ NULL }
};

void bddserv(const char *source, char *buf)
{
	char *cmd;
	User *u = finduser(source);
	
	if (!u)
	   return;
		
	cmd = strtok(buf, " ");
	
	if (!cmd) {
	   return;
	} else {
	   run_cmd(s_BddServ, u, cmds, cmd);
           return;
	}
}

static void do_help(User *u)
{
	char *cmd = strtok(NULL,"");
	
	if (!cmd) {
		notice_help(s_BddServ, u, BDD_HELP);
	} else {
		help_cmd(s_BddServ, u, cmds, cmd);
	}
}

static void actualizar_contadores(User *u)
{
	send_cmd(s_BddServ, "STATS b");
	notice_lang(s_BddServ, u, BDD_SEQ_OK);
}

static void compactar_tablas(User *u)
{
	char d[]="*";
	do_write_bdd(d, 15, "Compactando tabla n");
	do_write_bdd(d, 2, "Compactando tabla v");
	do_write_bdd(d, 3, "Compactando tabla o");
	do_write_bdd(d, 4, "Compactando tabla w");
       	notice_lang(s_BddServ, u, BDD_COMPACT);
}

static void regenerar_clave(User *u)
{
            static char saltChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]";
            unsigned long long xi;
	    /* char salt[ilevel + 2]; */
	    char salt[41];
	    /*salt = (char *) malloc((ilevel+1)*sizeof(char));*/
	    int cnt = 0;
	    salt[40] = '\0';
            __asm__ volatile (".byte 0x0f, 0x31" : "=A" (xi));
	    srandom(xi);

	        for(cnt = 0; cnt < (40); ++cnt)
		  salt[cnt] = saltChars[random() % 64];


		
	/*void do_write_bdd(char *entrada, int tabla, const char *valor, ...)*/
	char entrada[]="clave.de.cifrado.de.ips";
	const char *valor=salt;
	do_write_bdd(entrada,6,valor);
	notice_lang(s_BddServ, u, BDD_REG_SUCCESS,salt);
}

static void tocar_tablas(User *u)
{
	char *tabla = strtok(NULL, " ");
	char *clave = strtok(NULL, " ");
	char *valor = strtok(NULL, "");

	if ((!tabla) || (!clave)) {
		syntax_error(s_BddServ, u, "TOCAR", BDD_TOCAR_SYNTAX);
		return;
		}
       /*asi podemos desactivar la ip virtual con bdd*/
	if (stricmp(tabla, "V") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 2, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave, 2, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }
  /*asi podemos desactivar redirecciones de canales*/
	if (stricmp(tabla, "r") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 9, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave, 9, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }
/*asi podemos desactivar jupeos de nicks*/
	if (stricmp(tabla, "j") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 10, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave, 10, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }

 /*asi podemos desactivar registros  en tabla z*/
	if (stricmp(tabla, "z") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 6, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave, 6, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }
/*asi podemos desactivar registros  en tabla u Uworld*/
	if (stricmp(tabla, "u") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 11, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave,11, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }


       /*asi podemos desactivar los canales persistentes-tabla c- con bdd*/
	if (stricmp(tabla, "c") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 7, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave, 7, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }
/*asi podemos eliminar nicks con bdd*/
	if (stricmp(tabla, "n") == 0) {
        if (!valor) {
                     do_write_bdd(clave, 15, "");
		     do_write_bdd(clave, 2, "");
		    do_write_bdd(clave, 3, "");
		    do_write_bdd(clave, 4, "");
                     notice_lang(s_BddServ, u, BDD_SEQ_OK);
		      return;
		     }
         else {
             do_write_bdd(clave, 1, valor);
             notice_lang(s_BddServ, u, BDD_SEQ_OK);
              return;
              }
           
          }


	if (!valor) {
		syntax_error(s_BddServ, u, "TOCAR", BDD_TOCAR_SYNTAX);
		return;
	} else if (strlen(tabla) != 1) {
		notice_lang(s_BddServ, u, BDD_ERR_TABLE);
		return;
	} else {
	
		/*if (stricmp(tabla, "N") == 0) {
			do_write_bdd(clave, 1, valor); */
           if (stricmp(tabla, "O") == 0) {
			do_write_bdd(clave, 3, valor);
		}/* else if (stricmp(tabla, "V") == 0) {
			do_write_bdd(clave, 2, valor);
		}*/ else if (stricmp(tabla, "W") == 0) {
			do_write_bdd(clave, 4, valor);
		} else if (stricmp(tabla, "I") == 0) {
			do_write_bdd(clave, 5, valor);
		} /*else if (stricmp(tabla, "Z") == 0) {
			do_write_bdd(clave, 6, valor);
		}*/ else {
			notice_lang(s_BddServ, u, BDD_ERR_TABLE);
			return;
		}

	notice_lang(s_BddServ, u, BDD_SEQ_OK);

	}

}
#endif



