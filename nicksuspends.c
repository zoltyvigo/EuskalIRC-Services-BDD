/* Anicksuspends list functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
* Soporte suspensiones de nicks con tiempos
* por (2009)donostiarra http://euskalirc.wordpress.com
*
*
*/
#include "services.h"
#include "pseudo.h"
#include "P10.c" /*faltaba soporte p10*/
/*************************************************************************/


/*Almaceno los nicks suspendidos con sus tiempos*/

struct anick {
    char *elnick;
    char *reason;
    time_t time;
    time_t expires;	/* or 0 for no expiry */
};

static int32 nanick = 0;
static int32 anick_size = 0;
static struct anick *anicks = NULL;

/*************************************************************************/
/****************************** Statistics *******************************/
/*************************************************************************/

void get_anick_stats(long *nrec, long *memuse)
{
    long mem;
    int i;

    mem = sizeof(struct anick) * anick_size;
    for (i = 0; i < nanick; i++) {
	mem += strlen(anicks[i].elnick)+1;
	
	
    }
    *nrec = nanick;
    *memuse = mem;
}


int num_anicks(void)
{
    return (int) nanick;
}

/*************************************************************************/
/*********************** AREGISTRA database load/save ************************/
/*************************************************************************/

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de lectura en %s", NickSuspendsDBName);	\
	nanick = i;					\
	break;						\
    }							\
} while (0)

void load_anick(void)
{
    dbFILE *f;
    int i, ver;
    int16 tmp16;
    int32 tmp32;

    if (!(f = open_db("ANICKSUSPENDS", NickSuspendsDBName, "r")))
	return;

    ver = get_file_version(f);

    read_int16(&tmp16, f);
    nanick = tmp16;
    if (nanick < 8)
	anick_size = 16;
    else if (nanick >= 16384)
	anick_size = 32767;
    else
	anick_size = 2*nanick;
    anicks = scalloc(sizeof(*anicks), anick_size);

    switch (ver) {
      case 8:
      case 7:
      case 6:
      case 5:
	for (i = 0; i < nanick; i++) {
	  
	    SAFE(read_string(&anicks[i].elnick, f));
	    SAFE(read_string(&anicks[i].reason, f));
	    SAFE(read_int32(&tmp32, f));
	    anicks[i].time = tmp32;
	    SAFE(read_int32(&tmp32, f));
	    anicks[i].expires = tmp32;
	}
	break;

      case 4:
      case 3: {
	struct {
	    char *elnick;
	    char *reason;
	    time_t time;
	    time_t expires;
	    long reserved[4];
	} old_anick;

	for (i = 0; i < nanick; i++) {
	    SAFE(read_variable(old_anick, f));
	    anicks[i].time = old_anick.time;
	    anicks[i].expires = old_anick.expires;
	}
	for (i = 0; i < nanick; i++) {
	     SAFE(read_string(&anicks[i].elnick, f));
	     SAFE(read_string(&anicks[i].reason, f));
	}
	break;
      } /* case 3/4 */

      case 2: {
	struct {
	    char *elnick;
	    char *reason;
	    time_t time;
	} old_anick;

	for (i = 0; i < nanick; i++) {
	    SAFE(read_variable(old_anick, f));
	    anicks[i].time = old_anick.time;
	     anicks[i].expires = 0;
	}
	for (i = 0; i < nanick; i++) {
	    SAFE(read_string(&anicks[i].elnick, f));
	    SAFE(read_string(&anicks[i].reason, f));
	    }
	break;
      } /* case 2 */

      case 1: {
	struct {
	    char *elnick;
	    char *reason;
	    time_t time;
	} old_anick;

	for (i = 0; i < nanick; i++) {
	    SAFE(read_variable(old_anick, f));
	    anicks[i].time = old_anick.time;
	    anicks[i].expires = 0;
	}
	for (i = 0; i < nanick; i++) {
	    SAFE(read_string(&anicks[i].elnick, f));
	    SAFE(read_string(&anicks[i].reason, f));
	      	}
	break;
      } /* case 1 */

      default:
	fatal("Version no soportada (%d) en %s", ver, NickSuspendsDBName);
    } /* switch (version) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {							\
    if ((x) < 0) {							\
	restore_db(f);							\
	log_perror("Error de escritura en %s", NickSuspendsDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {			\
	    canalopers(NULL, "Error de escritura en %s: %s",NickSuspendsDBName,	\
			strerror(errno));				\
	    lastwarn = time(NULL);					\
	}								\
	return;								\
    }									\
} while (0)

void save_anick(void)
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;

    f = open_db("ANICKSUSPENDS", NickSuspendsDBName, "w");
    write_int16(nanick, f);
    for (i = 0; i < nanick; i++) {
	
	SAFE(write_string(anicks[i].elnick, f));
	SAFE(write_string(anicks[i].reason, f));
	SAFE(write_int32(anicks[i].time, f));
	SAFE(write_int32(anicks[i].expires, f));
    }
    close_db(f);
}

#undef SAFE


/*************************************************************************/

/* Borro autonicks expirados */

int expire_anicks(void)
{
    int i;
    time_t now = time(NULL);
    NickInfo *ni;

    for (i = 0; i < nanick; i++) {
	if (anicks[i].expires > now)
	    continue;
	

#if defined(IRC_UNDERNET_P10)
        //send_cmd(NULL,"%c P * -%s", convert2y[ServerNumerico], alimits[i].elcanal);        	
#else
     if  (ni = findnick(anicks[i].elnick)) {
      
       ni->status &= ~NS_SUSPENDED;
        free(ni->suspendby);
        free(ni->suspendreason);
        ni->time_suspend = 0;
        ni->time_expiresuspend = 0;
	  canalopers(s_NickServ, "Se ha reactivado el nick 12%s por expiración de la Suspensión",anicks[i].elnick);

	  if (ni->status & NI_ON_BDD)
		  do_write_bdd(ni->nick, 1, ni->pass);
	
         if (finduser(anicks[i].elnick)) {
              privmsg (s_NickServ, ni->nick, "Tu nick 12%s ha sido reactivado.", anicks[i].elnick);
              privmsg (s_NickServ, ni->nick, "Vuelve a identificarte con tu nick.");
              send_cmd(NULL, "RENAME %s", ni->nick);
	  }
       }
 
	
#endif        
        free(anicks[i].elnick);
	free(anicks[i].reason);
	       nanick--;
	if (i < nanick)
	    memmove(anicks+i, anicks+i+1, sizeof(*anicks) * (nanick-i));
	i--;
    }
}


void add_anick(const char *elnick,const char *reason,time_t expiry)
{
    if (nanick >= 32767) {
	logeo("%s: Intento para añadir ANICKSUSPENDS a la lista llena!", s_ChanServ);
	return;
    }
    if (nanick >= anick_size) {
	if (anick_size < 8)
	    anick_size = 8;
	else
	    anick_size *= 2;
	anicks = srealloc(anicks, sizeof(*anicks) * anick_size);
    }
    anicks[nanick].elnick = sstrdup(elnick);
     anicks[nanick].reason = sstrdup(reason);
    anicks[nanick].time = time(NULL);
    anicks[nanick].expires = expiry;

    nanick++;
}
/*************************************************************************/

void del_anick(const char *elnick)
{
    int i;

    for (i = 0; i < nanick && strcmp(anicks[i].elnick, elnick) != 0; i++)
	;
    if (i < nanick) {
	free(anicks[i].elnick);
	nanick--;
	memmove(anicks+i, anicks+i+1, sizeof(*anicks) * (nanick-i));
	
       }
}
