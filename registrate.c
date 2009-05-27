/* Autoregistra list functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
* Soporte envio de mensajes automaticos informativos para nicks no registrados
* basado en akill, por (2009)donostiarra http://euskalirc.wordpress.com
*
*
*/
#include "services.h"
#include "pseudo.h"
#include "P10.c" /*faltaba soporte p10*/
/*************************************************************************/

/* Hare con registra en lugar de akill para mandar una serie de privados a un nick no reg
   REGISTRA*/

struct aregistra {
    char *elnick;
    time_t time;
    time_t expires;	/* or 0 for no expiry */
};

static int32 naregistra = 0;
static int32 aregistra_size = 0;
static struct aregistra *aregistras = NULL;

/*************************************************************************/
/****************************** Statistics *******************************/
/*************************************************************************/

void get_aregistra_stats(long *nrec, long *memuse)
{
    long mem;
    int i;

    mem = sizeof(struct aregistra) * aregistra_size;
    for (i = 0; i < naregistra; i++) {
	mem += strlen(aregistras[i].elnick)+1;
	
    }
    *nrec = naregistra;
    *memuse = mem;
}


int num_aregistras(void)
{
    return (int) naregistra;
}

/*************************************************************************/
/*********************** AREGISTRA database load/save ************************/
/*************************************************************************/

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de lectura en %s", AutoregistraDBName);	\
	naregistra = i;					\
	break;						\
    }							\
} while (0)

void load_aregistra(void)
{
    dbFILE *f;
    int i, ver;
    int16 tmp16;
    int32 tmp32;

    if (!(f = open_db("AREGISTRA", AutoregistraDBName, "r")))
	return;

    ver = get_file_version(f);

    read_int16(&tmp16, f);
    naregistra = tmp16;
    if (naregistra < 8)
	aregistra_size = 16;
    else if (naregistra >= 16384)
	aregistra_size = 32767;
    else
	aregistra_size = 2*naregistra;
    aregistras = scalloc(sizeof(*aregistras), aregistra_size);

    switch (ver) {
      case 8:
      case 7:
      case 6:
      case 5:
	for (i = 0; i < naregistra; i++) {
	  
	    SAFE(read_string(&aregistras[i].elnick, f));
	    SAFE(read_int32(&tmp32, f));
	    aregistras[i].time = tmp32;
	    SAFE(read_int32(&tmp32, f));
	    aregistras[i].expires = tmp32;
	}
	break;

      case 4:
      case 3: {
	struct {
	    char *elnick;
	    time_t time;
	    time_t expires;
	    long reserved[4];
	} old_aregistra;

	for (i = 0; i < naregistra; i++) {
	    SAFE(read_variable(old_aregistra, f));
	    aregistras[i].time = old_aregistra.time;
	    aregistras[i].expires = old_aregistra.expires;
	}
	for (i = 0; i < naregistra; i++) {
	     SAFE(read_string(&aregistras[i].elnick, f));
	}
	break;
      } /* case 3/4 */

      case 2: {
	struct {
	    char *elnick;
	    time_t time;
	} old_aregistra;

	for (i = 0; i < naregistra; i++) {
	    SAFE(read_variable(old_aregistra, f));
	    aregistras[i].time = old_aregistra.time;
	     aregistras[i].expires = 0;
	}
	for (i = 0; i < naregistra; i++) {
	    SAFE(read_string(&aregistras[i].elnick, f));
	    
	}
	break;
      } /* case 2 */

      case 1: {
	struct {
	    char *elnick;
	    time_t time;
	} old_aregistra;

	for (i = 0; i < naregistra; i++) {
	    SAFE(read_variable(old_aregistra, f));
	    aregistras[i].time = old_aregistra.time;
	    aregistras[i].expires = 0;
	}
	for (i = 0; i < naregistra; i++) {
	    SAFE(read_string(&aregistras[i].elnick, f));
	   	}
	break;
      } /* case 1 */

      default:
	fatal("Version no soportada (%d) en %s", ver, AutoregistraDBName);
    } /* switch (version) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {							\
    if ((x) < 0) {							\
	restore_db(f);							\
	log_perror("Error de escritura en %s", AutoregistraDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {			\
	    canalopers(NULL, "Error de escritura en %s: %s", AutoregistraDBName,	\
			strerror(errno));				\
	    lastwarn = time(NULL);					\
	}								\
	return;								\
    }									\
} while (0)

void save_aregistra(void)
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;

    f = open_db("AREGISTRA", AutoregistraDBName, "w");
    write_int16(naregistra, f);
    for (i = 0; i < naregistra; i++) {
	
	SAFE(write_string(aregistras[i].elnick, f));
	SAFE(write_int32(aregistras[i].time, f));
	SAFE(write_int32(aregistras[i].expires, f));
    }
    close_db(f);
}

#undef SAFE

/*************************************************************************/
/************************** External functions ***************************/
/*************************************************************************/

/* Does the user match any AKILLs? */

/*************************************************************************/

/* Delete any expired autokills. */

int expire_aregistras(void)
{
    int i;
    time_t now = time(NULL);

    for (i = 0; i < naregistra; i++) {
	if (aregistras[i].expires > now)
	    continue;
	    //canalopers(s_OperServ, "AREGISTRA en %s ha expirado", aregistras[i].elnick);

#ifdef IRC_UNDERNET_P10
        send_cmd(NULL,"%c P * -%s", convert2y[ServerNumerico], aregistras[i].elnick);        	
#else
        //send_cmd(ServerName, "PRIVMSG  %s :expiro el aregistra", aregistras[i].elnick);
	privmsg(s_NickServ, aregistras[i].elnick, "Hola 12%s2",aregistras[i].elnick);
       privmsg(s_NickServ, aregistras[i].elnick, "Soy 4NiCK, el encargado de los registros de los apodos en la Red.");
       privmsg(s_NickServ, aregistras[i].elnick, "Veo que tu apodo no está registrado.");
	privmsg(s_NickServ, aregistras[i].elnick, "Registrate con nosotros,es bien sencillo y totalmente gratuito.");
	privmsg(s_NickServ, aregistras[i].elnick, " Una vez registrado,podrás acceder a servicios exclusivos de usuarios registrados.");
        privmsg(s_NickServ, aregistras[i].elnick, "Como por ejemplo, nuestro servicio de mensajería 5(MeMo),");
	privmsg(s_NickServ, aregistras[i].elnick, "entrar a canales restringidos a usuarios registrados,");
	privmsg(s_NickServ, aregistras[i].elnick, "o recibir soporte especializado por parte de los representantes de la red.");
        privmsg(s_NickServ, aregistras[i].elnick, "Para registrartelo sólo debes escribir aquí  mismo el comando 3/msg 2NiCK 12register 4email");
        privmsg(s_NickServ, aregistras[i].elnick, "cambiando unicamente  tu dirección de 4email");
        privmsg(s_NickServ, aregistras[i].elnick, "No Cambies la palabra 2NiCK,que es el bot que te registra.");
	privmsg(s_NickServ, aregistras[i].elnick, "Se te registrará el nick que lleves puesto en ese momento y recibirás");
	privmsg(s_NickServ, aregistras[i].elnick, "un email con los datos e instrucciones de tu registro.");
        privmsg(s_NickServ, aregistras[i].elnick, "Si tienes dudas,no dudes en acudir al canal oficial de la red");
	privmsg(s_NickServ, aregistras[i].elnick, "Gracias por leer este mensaje automático");
	privmsg(s_NickServ, aregistras[i].elnick, "Un Saludo.La Administración");
#endif        
        free(aregistras[i].elnick);
        naregistra--;
	if (i < naregistra)
	    memmove(aregistras+i, aregistras+i+1, sizeof(*aregistras) * (naregistra-i));
	i--;
    }
}

/*************************************************************************/
/************************** AKILL list editing ***************************/
/*************************************************************************/

/* Note that all parameters except expiry are assumed to be non-NULL.  A
 * value of NULL for expiry indicates that the AKILL should not expire.
 *
 * Not anymore. Now expiry represents the exact expiry time and may not be 
 * NULL. -TheShadow
 */

void add_aregistra(const char *elnick,const time_t expiry)
{
    if (naregistra >= 32767) {
	log("%s: Intento para añadir AREGISTRA a la lista llena!", s_OperServ);
	return;
    }
    if (naregistra >= aregistra_size) {
	if (aregistra_size < 8)
	    aregistra_size = 8;
	else
	    aregistra_size *= 2;
	aregistras = srealloc(aregistras, sizeof(*aregistras) * aregistra_size);
    }
    aregistras[naregistra].elnick = sstrdup(elnick);
    aregistras[naregistra].time = time(NULL);
    aregistras[naregistra].expires = expiry;
   // aregistras[naregistra].expires = expiry;
    /*
    if (expiry) {
	int amount = strtol(expiry, (char **)&expiry, 10);
	if (amount == 0) {
	    akills[nakill].expires = 0;
	} else {
	    switch (*expiry) {
		case 'd': amount *= 24;
		case 'h': amount *= 60;
		case 'm': amount *= 60; break;
		default : amount = -akills[nakill].time;
	    }
	    akills[nakill].expires = amount + akills[nakill].time;
	}
    } else {
	akills[nakill].expires = AutokillExpiry + akills[nakill].time;
    }
*/
    naregistra++;
}

