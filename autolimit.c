/* Alimit list functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
* Soporte cambio modos de canal autolimit con cierto retardo "timers"
* basado en akill, por (2009)donostiarra http://euskalirc.wordpress.com
*
*
*/
#include "services.h"
#include "pseudo.h"
#include "P10.c" /*faltaba soporte p10*/
/*************************************************************************/

/* Haré con registra en lugar de akill para mandar una serie de privados a un nick no reg
   REGISTRA*/

struct alimit {
    char *elcanal;
    int elnumero;
    time_t time;
    time_t expires;	/* or 0 for no expiry */
};

static int32 nalimit = 0;
static int32 alimit_size = 0;
static struct alimit *alimits = NULL;

/*************************************************************************/
/****************************** Statistics *******************************/
/*************************************************************************/

void get_alimit_stats(long *nrec, long *memuse)
{
    long mem;
    int i;

    mem = sizeof(struct alimit) * alimit_size;
    for (i = 0; i < nalimit; i++) {
	mem += strlen(alimits[i].elcanal)+1;
	
	
    }
    *nrec = nalimit;
    *memuse = mem;
}


int num_alimits(void)
{
    return (int) nalimit;
}

/*************************************************************************/
/*********************** AREGISTRA database load/save ************************/
/*************************************************************************/

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de lectura en %s", AutolimitDBName);	\
	nalimit = i;					\
	break;						\
    }							\
} while (0)

void load_alimit(void)
{
    dbFILE *f;
    int i, ver;
    int16 tmp16;
    int32 tmp32;

    if (!(f = open_db("ALIMIT", AutolimitDBName, "r")))
	return;

    ver = get_file_version(f);

    read_int16(&tmp16, f);
    nalimit = tmp16;
    if (nalimit < 8)
	alimit_size = 16;
    else if (nalimit >= 16384)
	alimit_size = 32767;
    else
	alimit_size = 2*nalimit;
    alimits = scalloc(sizeof(*alimits), alimit_size);

    switch (ver) {
      case 8:
      case 7:
      case 6:
      case 5:
	for (i = 0; i < nalimit; i++) {
	  
	    SAFE(read_string(&alimits[i].elcanal, f));
	    SAFE(read_variable(alimits[i].elnumero, f));
	    SAFE(read_int32(&tmp32, f));
	    alimits[i].time = tmp32;
	    SAFE(read_int32(&tmp32, f));
	    alimits[i].expires = tmp32;
	}
	break;

      case 4:
      case 3: {
	struct {
	    char *elcanal;
	    int elnumero;
	    time_t time;
	    time_t expires;
	    long reserved[4];
	} old_alimit;

	for (i = 0; i < nalimit; i++) {
	    SAFE(read_variable(old_alimit, f));
	    alimits[i].time = old_alimit.time;
	    alimits[i].expires = old_alimit.expires;
	}
	for (i = 0; i < nalimit; i++) {
	     SAFE(read_string(&alimits[i].elcanal, f));
	     SAFE(read_variable(alimits[i].elnumero, f));
	}
	break;
      } /* case 3/4 */

      case 2: {
	struct {
	    char *elcanal;
	     int elnumero;
	    time_t time;
	} old_alimit;

	for (i = 0; i < nalimit; i++) {
	    SAFE(read_variable(old_alimit, f));
	    alimits[i].time = old_alimit.time;
	     alimits[i].expires = 0;
	}
	for (i = 0; i < nalimit; i++) {
	    SAFE(read_string(&alimits[i].elcanal, f));
	    SAFE(read_variable(alimits[i].elnumero, f));
	    
	}
	break;
      } /* case 2 */

      case 1: {
	struct {
	    char *elcanal;
	    int elnumero;
	    time_t time;
	} old_alimit;

	for (i = 0; i < nalimit; i++) {
	    SAFE(read_variable(old_alimit, f));
	    alimits[i].time = old_alimit.time;
	    alimits[i].expires = 0;
	}
	for (i = 0; i < nalimit; i++) {
	    SAFE(read_string(&alimits[i].elcanal, f));
	    SAFE(read_variable(alimits[i].elnumero, f));
	   	}
	break;
      } /* case 1 */

      default:
	fatal("Version no soportada (%d) en %s", ver, AutolimitDBName);
    } /* switch (version) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {							\
    if ((x) < 0) {							\
	restore_db(f);							\
	log_perror("Error de escritura en %s", AutolimitDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {			\
	    canalopers(NULL, "Error de escritura en %s: %s", AutolimitDBName,	\
			strerror(errno));				\
	    lastwarn = time(NULL);					\
	}								\
	return;								\
    }									\
} while (0)

void save_alimit(void)
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;

    f = open_db("ALIMIT", AutolimitDBName, "w");
    write_int16(nalimit, f);
    for (i = 0; i < nalimit; i++) {
	
	SAFE(write_string(alimits[i].elcanal, f));
	SAFE(read_variable(alimits[i].elnumero, f));
	SAFE(write_int32(alimits[i].time, f));
	SAFE(write_int32(alimits[i].expires, f));
    }
    close_db(f);
}

#undef SAFE


/*************************************************************************/

/* Borro autolimits expirados */

int expire_alimits(void)
{
    int i;
    time_t now = time(NULL);

    for (i = 0; i < nalimit; i++) {
	if (alimits[i].expires > now)
	    continue;
	

#ifdef IRC_UNDERNET_P10
        //send_cmd(NULL,"%c P * -%s", convert2y[ServerNumerico], alimits[i].elcanal);        	
#else
       
        send_cmd(s_ChanServ, "MODE %s +l %i", alimits[i].elcanal, alimits[i].elnumero);
	
#endif        
        free(alimits[i].elcanal);
	       nalimit--;
	if (i < nalimit)
	    memmove(alimits+i, alimits+i+1, sizeof(*alimits) * (nalimit-i));
	i--;
    }
}


void add_alimit(const char *elcanal,int elnumero, time_t expiry)
{
    if (nalimit >= 32767) {
	log("%s: Intento para añadir ALIMIT a la lista llena!", s_ChanServ);
	return;
    }
    if (nalimit >= alimit_size) {
	if (alimit_size < 8)
	    alimit_size = 8;
	else
	    alimit_size *= 2;
	alimits = srealloc(alimits, sizeof(*alimits) * alimit_size);
    }
    alimits[nalimit].elcanal = sstrdup(elcanal);
    alimits[nalimit].elnumero = elnumero;
    alimits[nalimit].time = time(NULL);
    alimits[nalimit].expires = expiry;

    nalimit++;
}
/*************************************************************************/

void del_alimit(const char *elcanal)
{
    int i;

    for (i = 0; i < nalimit && strcmp(alimits[i].elcanal, elcanal) != 0; i++)
	;
    if (i < nalimit) {
	free(alimits[i].elcanal);
	nalimit--;
	memmove(alimits+i, alimits+i+1, sizeof(*alimits) * (nalimit-i));
	
       }
}
