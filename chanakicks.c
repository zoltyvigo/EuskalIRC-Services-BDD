/* achanakick list functions.
por (2010)donostiarra http://euskalirc.wordpress.com
*/

#include "services.h"
#include "pseudo.h"
/*************************************************************************/

struct achanakick {
    char *canal;
    char *nick;
    char *reason;
    char who[NICKMAX];
    time_t time;
    time_t caza_akick;
    //time_t expires;	/* or 0 for no expiry */
};

static int32 nachanakick = 0;
static int32 achanakick_size = 0;
static struct achanakick *achanakicks = NULL;

/*************************************************************************/
/****************************** Statistics *******************************/
/*************************************************************************/

void get_achanakick_stats(long *nrec, long *memuse)
{
    long mem;
    int i;

    mem = sizeof(struct achanakick) * achanakick_size;
    for (i = 0; i < nachanakick; i++) {
	mem += strlen(achanakicks[i].canal)+1;
	mem += strlen(achanakicks[i].nick)+1;
	mem += strlen(achanakicks[i].reason)+1;
	mem += strlen(achanakicks[i].who)+1;
	
    }
    *nrec = nachanakick;
    *memuse = mem;
}


int num_achanakicks(void)
{
    return (int) nachanakick;
}

/*************************************************************************/
/*********************** Autochanakick database load/save ************************/
/*************************************************************************/

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de lectura en %s", AutochanakickDBName);	\
	nachanakick = i;					\
	break;						\
    }							\
} while (0)

void load_achanakick(void)
{
    dbFILE *f;
    int i, ver;
    int16 tmp16;
    int32 tmp32;

    if (!(f = open_db("ACHANAKICK", AutochanakickDBName, "r")))
	return;

    ver = get_file_version(f);

    read_int16(&tmp16, f);
    nachanakick = tmp16;
    if (nachanakick < 8)
	achanakick_size = 16;
    else if (nachanakick >= 16384)
	achanakick_size = 32767;
    else
	achanakick_size = 2*nachanakick;
    achanakicks = scalloc(sizeof(*achanakicks), achanakick_size);

    switch (ver) {
      case 8:
      case 7:
      case 6:
      case 5:
	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_string(&achanakicks[i].canal, f));
	    SAFE(read_string(&achanakicks[i].nick, f));
	    SAFE(read_string(&achanakicks[i].reason, f));
	    SAFE(read_buffer(achanakicks[i].who, f));
	    SAFE(read_int32(&tmp32, f));
	    achanakicks[i].time = tmp32;
	    SAFE(read_int32(&tmp32, f));
	    achanakicks[i].caza_akick = tmp32;
	    /*SAFE(read_int32(&tmp32, f));
	    achanakicks[i].expires = tmp32;*/
	}
	break;

      case 4:
      case 3: {
	struct {
	    char *canal;
	    char *nick;
	    char *reason;
	    char who[NICKMAX];
	    time_t time;
	     time_t caza_akick;
	    //time_t expires;
	    long reserved[4];
	} old_achanakick;

	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_variable(old_achanakick, f));
	    strscpy(achanakicks[i].who, old_achanakick.who, NICKMAX);
	    achanakicks[i].time = old_achanakick.time;
	    achanakicks[i].caza_akick = old_achanakick.caza_akick;
	    //achanakicks[i].expires = old_achanakick.expires;
	}
	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_string(&achanakicks[i].canal, f));
	    SAFE(read_string(&achanakicks[i].nick, f));
	    SAFE(read_string(&achanakicks[i].reason, f));
	  
	}
	break;
      } /* case 3/4 */

      case 2: {
	struct {
	    char *canal;
	    char *nick;
	    char *reason;
	    char who[NICKMAX];
	    time_t time;
	    time_t caza_akick;
	} old_achanakick;

	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_variable(old_achanakick, f));
	    achanakicks[i].time = old_achanakick.time;
	    achanakicks[i].caza_akick = old_achanakick.caza_akick;
	    strscpy(achanakicks[i].who, old_achanakick.who, sizeof(achanakicks[i].who));
	    //achanakicks[i].expires = 0;
	}
	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_string(&achanakicks[i].canal, f));
	    SAFE(read_string(&achanakicks[i].nick, f));
	    SAFE(read_string(&achanakicks[i].reason, f));
	
	}
	break;
      } /* case 2 */

      case 1: {
	struct {
	    char *canal;
	    char *nick;
	    char *reason;
	    char *who;
	    time_t time;
	    time_t caza_akick;
	} old_achanakick;

	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_variable(old_achanakick, f));
	    achanakicks[i].time = old_achanakick.time;
	     achanakicks[i].caza_akick = old_achanakick.caza_akick;
	    achanakicks[i].who[0] = 0;
	    //achanakicks[i].expires = 0;
	}
	for (i = 0; i < nachanakick; i++) {
	    SAFE(read_string(&achanakicks[i].canal, f));
	    SAFE(read_string(&achanakicks[i].nick, f));
	    SAFE(read_string(&achanakicks[i].reason, f));
	   
	}
	break;
      } /* case 1 */

      default:
	fatal("Version no soportada (%d) en %s", ver, AutochanakickDBName);
    } /* switch (version) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {							\
    if ((x) < 0) {							\
	restore_db(f);							\
	log_perror("Error de escritura en %s", AutochanakickDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {			\
	    canalopers(NULL, "Error de escritura en %s: %s", AutochanakickDBName,	\
			strerror(errno));				\
	    lastwarn = time(NULL);					\
	}								\
	return;								\
    }									\
} while (0)

void save_achanakick(void)
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;

    f = open_db("ACHANAKICK", AutochanakickDBName, "w");
    write_int16(nachanakick, f);
    for (i = 0; i < nachanakick; i++) {
	SAFE(write_string(achanakicks[i].canal, f));
	SAFE(write_string(achanakicks[i].nick, f));
	SAFE(write_string(achanakicks[i].reason, f));
	SAFE(write_buffer(achanakicks[i].who, f));
	SAFE(write_int32(achanakicks[i].time, f));
        SAFE(write_int32(achanakicks[i].caza_akick, f));
	//SAFE(write_int32(achanakicks[i].expires, f));
    }
    close_db(f);
}

#undef SAFE




void add_achanakick(const char *canal, const char *nick,const char *reason, const char *who)
{
    if (nachanakick >= 32767) {
	log("%s: Intento para añadir ACHANAKICK a la lista llena!", s_ChanServ);
	return;
    }
    if (nachanakick >= achanakick_size) {
	if (achanakick_size < 8)
	    achanakick_size = 8;
	else
	    achanakick_size *= 2;
	achanakicks = srealloc(achanakicks, sizeof(*achanakicks) * achanakick_size);
    }
    achanakicks[nachanakick].canal = sstrdup(canal);
    achanakicks[nachanakick].nick = sstrdup(nick);
     achanakicks[nachanakick].reason = sstrdup(reason);
    achanakicks[nachanakick].time = time(NULL);
    //achanakicks[nachanakick].expires = expiry;
    strscpy(achanakicks[nachanakick].who, who, NICKMAX);

    nachanakick++;
}
int add_cazakick(const char *canal, const char *nick)
{
    int j;
 for (j = 0; j < nachanakick && strcmp(achanakicks[j].canal, canal) != 0 && strcmp(achanakicks[j].nick,nick) != 0; j++);
return j;
   
}
/*************************************************************************/

/*Miramos toda la lista pero solo borramos el primero encontrado*/
/*También validamos con dígitos*/
static int del_achanakick(const char *mask,const char *canal,User *u)
{

if (isdigit(*mask)) {
int i=0;
	int cont=0;
	//notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_HEADER, canal);
        while (i < nachanakick )  {
	    if ((stricmp(achanakicks[i].canal, canal) == 0)) {
	cont++;
	
	if (cont == atoi(mask)) {
	if (achanakicks[i].canal)
           free(achanakicks[i].canal);
	 notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED, achanakicks[i].nick, canal);
	 if (achanakicks[i].nick)
           free(achanakicks[i].nick);
	if (achanakicks[i].caza_akick)
           achanakicks[i].caza_akick=0;
	
	nachanakick--;
	 memmove(achanakicks+i, achanakicks+i+1, sizeof(*achanakicks) * (nachanakick-i));
	
         return 1;
	
	}


	    }
	i++;
	}
      return 0;
 } else {


   int  i=0;
while (i < nachanakick )  {

if ((stricmp(achanakicks[i].canal, canal) == 0 && strcmp(achanakicks[i].nick, mask) == 0)) {
        if (achanakicks[i].canal)
           free(achanakicks[i].canal);
	 if (achanakicks[i].nick)
           free(achanakicks[i].nick);
	if (achanakicks[i].caza_akick)
           achanakicks[i].caza_akick=0;
	memmove(achanakicks+i, achanakicks+i+1, sizeof(*achanakicks) * (nachanakick-i));
	nachanakick--;
         return 1;
       }
i++;
        }

	return 0;
}
    
}
 /*no borramos el primero que encontramos,sino todos*/
int borra_akick(const char *chan)
{
       int i=0;
while (i < nachanakick )  {


if (stricmp(achanakicks[i].canal,chan) == 0) {

        if (achanakicks[i].canal)
           free(achanakicks[i].canal);
	 if (achanakicks[i].nick)
           free(achanakicks[i].nick);
		

         memmove(achanakicks+i, achanakicks+i+1, sizeof(*achanakicks) * (nachanakick-i));
	nachanakick--;
	i=-1;
	
       }
i++;
        }

	return 0;
    
}
/* funcion que nos avisa si un canal tiene akicks*/
int akick_entradas(const char *chan)
{
       int i=0;
	int cont =0;
while (i < nachanakick )  {


if (stricmp(achanakicks[i].canal,chan) == 0) {

 cont++;
	
       }
i++;
        }
      if (cont >= 1 )
	return 1;

    return 0;
}

/*************************************************************************/

/* Handle an ChanServ AKICK command. */

void do_achanakick(User *u)
{
   
    char *canal   = strtok(NULL, " ");
    char *cmd    = strtok(NULL, " ");
    char *mask   = strtok(NULL, " ");
    char *reason = strtok(NULL, "");
    //char *s = strtok(NULL, "");
NickInfo *ni = findnick(u->nick);
    ChannelInfo *ci;
    int i,cont;
      //AutoKick *akick;    

    if (!cmd || (!mask && 
		(stricmp(cmd, "ADD") == 0 || stricmp(cmd, "DEL") == 0 ))) {
	syntax_error(s_ChanServ, u, "AKICK", CHAN_AKICK_SYNTAX);
    } else if (!(ci = cs_findchan(canal))) {
	notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, canal);
    } else if (ci->flags & CI_VERBOTEN) {
	notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, canal);
   } else if (((stricmp(cmd,"LIST") != 0 && stricmp(cmd,"VIEW") != 0  && stricmp(cmd,"ENFORCE") != 0 ) && (ci->flags & CI_SUSPEND))) {
        notice_lang(s_ChanServ, u, CHAN_X_SUSPENDED, canal);          
   

    } else if (stricmp(cmd, "ADD") == 0) {

	if (nachanakick >= 32767) {
	    notice_lang(s_ChanServ, u, OPER_TOO_MANY_AKILLS);
	    return;
	}
        if (!check_access(u, ci, CA_AKICK) && !is_services_oper(u)) {
             if (ci->founder && getlink(ci->founder) == u->ni) {
	    notice_lang(s_ChanServ, u, CHAN_IDENTIFY_REQUIRED, s_ChanServ,canal);
            return; }
	else {
	    notice_lang(s_ChanServ, u, ACCESS_DENIED);
            return;
            }
         }
		cont=0;
	       for (i = 0; i < nachanakick; i++) {
	    if (stricmp(achanakicks[i].canal, canal) == 0) {
	//privmsg(s_ChanServ, u->nick, "%s-->%s",achanakicks[i].nick,achanakicks[i].who);	
cont++;
//return;
	    }
}
	if (cont >=CSAutokickMax) {
	notice_lang(s_ChanServ, u, CHAN_AKICK_REACHED_LIMIT,
			CSAutokickMax);
		return;
          }
	NickInfo *ni = findnick(mask);
	char *nick, *user, *host;
   /* para escribir menos al añadir nicks akicks no valido ni */ 
	if (!ni || ni) {
	    split_usermask(mask, &nick, &user, &host);
	    mask = smalloc(strlen(nick)+strlen(user)+strlen(host)+3);
	    sprintf(mask, "%s!%s@%s", nick, user, host);
	    /*free(nick);
	    free(user);
	    free(host);*/
           } 
		
	if (!user || !host) {
	 snprintf(mask, sizeof(mask), "%s!*@*", nick);
         }
      
       
            
for (i = 0; i < nachanakick; i++) {
	      if ((stricmp(achanakicks[i].canal, canal) == 0 && (stricmp(achanakicks[i].nick, mask) == 0))) {
	//privmsg(s_ChanServ, u->nick, "%s-->%s",achanakicks[i].nick,achanakicks[i].who);	
notice_lang(s_ChanServ, u, CHAN_AKICK_ALREADY_EXISTS,
			achanakicks[i].nick,canal);


return;
	    }
}
	      if (!reason)
                 reason= CSAutokickReason;
	      
	      add_achanakick(canal, mask,reason,u->nick);
notice_lang(s_ChanServ, u, CHAN_AKICK_ADDED,mask, canal);

/* Para no expulsar al OPER*/
 if (ni && is_oper(ni->nick)) { 
		return;
		}

Channel *c = canal ? findchan(canal) : NULL;
    struct c_userlist *u;
const char *source = nick;
 char buf[BUFSIZE];
   snprintf(buf, sizeof(buf), "%s!*@*",nick);

   //mask = create_mask(user);



if (!c) {
	return;
    }
 for (u = c->users; u; u = u->next) {
	if (stricmp(u->user->nick, nick) == 0) {
	 mask = create_mask(u->user);
	send_cmd(s_ChanServ, "MODE %s +b %s  %lu", canal, mask, time(NULL));
        send_cmd(MODE_SENDER(s_ChanServ), "KICK %s %s :%s", canal, nick, reason);
/*miramos por nicks al canal para añadir ultimo uso*/
check_akick(u->user, canal);

       }
	}
	   

                        
    } else if (stricmp(cmd, "DEL") == 0) {
	NickInfo *ni = findnick(mask);
	char *nick, *user, *host;
        if (!check_access(u, ci, CA_AKICK_DEL) && !is_services_oper(u)) {
             if (ci->founder && getlink(ci->founder) == u->ni) {
	    notice_lang(s_ChanServ, u, CHAN_IDENTIFY_REQUIRED, s_ChanServ,canal);
            return; }
	else {
	    notice_lang(s_ChanServ, u, ACCESS_DENIED);
            return;
            }
         }
 /* para escribir menos al eliminar nicks de  akicks no valido ni */ 
	if (!ni || ni) {
	    split_usermask(mask, &nick, &user, &host);
	    mask = smalloc(strlen(nick)+strlen(user)+strlen(host)+3);
	    sprintf(mask, "%s!%s@%s", nick, user, host);
	    free(nick);
	    free(user);
	    free(host);
           } 
		
	if (!user || !host) {
          sprintf(mask, "%s!*@*", nick);
           }
	if (stricmp(mask, "all!*@*") == 0) {
			if (akick_entradas(canal)) {
				 	borra_akick(canal);
				notice_lang(s_ChanServ, u, CHAN_AKICK_ALL_DELETED, canal);
			 return;
				}
                      	 notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, canal);
			 return;
			} 
	
	if (mask) {
	    if (del_achanakick(mask,canal,u)) {    
                 if  (!isdigit(*mask)) {      	                  
		notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED, mask, canal);
                       }
		
		if (readonly)
		    notice_lang(s_ChanServ, u, READ_ONLY_MODE);
	    } else {
		
		notice_lang(s_ChanServ, u, CHAN_AKICK_NOT_FOUND, mask, canal);
	    }
	} else {
	    syntax_error(s_ChanServ, u, "AKICK", CHAN_AKICK_SYNTAX);
	}

    } else if (stricmp(cmd, "ENFORCE") == 0) {
       if (!check_access(u, ci, CA_AKICK) && !is_services_oper(u)) {
             if (ci->founder && getlink(ci->founder) == u->ni) {
	    notice_lang(s_ChanServ, u, CHAN_IDENTIFY_REQUIRED, s_ChanServ,canal);
            return; }
	else {
	    notice_lang(s_ChanServ, u, ACCESS_DENIED);
            return;
            }
         }
	Channel *c = canal ? findchan(canal) : NULL;
    struct c_userlist *u;
char *nick, *user, *host;

int count = 0;


if (!c) {
	return;
    }
i=0;
cont=0;
char *av[3];

char mascara[BUFSIZE];
char masknickuserip[BUFSIZE];
char maskuserip[BUFSIZE];
char maskip[BUFSIZE];

User *u2 = finduser(ni->nick); 

 for (u = c->users; u; u = u->next) {

 snprintf(mascara, sizeof(mascara), "%s!*@*", u->user->nick);
snprintf(masknickuserip, sizeof(masknickuserip), "%s!%s@%s",u->user->nick,u->user->username,u->user->host);
 snprintf(maskuserip, sizeof(maskuserip), "*!%s@%s",u->user->username,u->user->host);
 snprintf(maskip, sizeof(maskip), "*!*@%s",u->user->host);


  i=0;
  while (i < nachanakick )  {
	    if ((stricmp(achanakicks[i].canal, canal) == 0 && (stricmp(achanakicks[i].nick, strlower(mascara)) == 0 || match_wild(achanakicks[i].nick,strlower(masknickuserip)) ||  match_wild(achanakicks[i].nick,strlower(maskuserip)) ||  match_wild(achanakicks[i].nick,strlower(maskip))))) {
 
         reason = achanakicks[i].reason;
	achanakicks[i].caza_akick= time(NULL);
mask = create_mask(u->user);
if (!(is_oper(u->user->nick))) { 
	 send_cmd(s_ChanServ, "MODE %s +b %s  %lu", canal, mask, time(NULL));
privmsg(s_ChanServ, ni->nick, "12ENFORCE 4BAN usado sobre host 4%s",mask);
   // send_cmd(s_ChanServ, "MODE %s +b %s  %lu", canal, u->user->host, time(NULL)); <---da el host completo
//privmsg(s_ChanServ, ni->nick, "ENFORCE BAN en %s usado sobre host %s",canal,u->user->host,cont);

} //fin if si no es oper
	
#ifdef IRC_UNDERNET_P10
    send_cmd(s_ChanServ, "K %s %s :%s", canal, u->user->numerico, achanakicks[i].reason);
#else
if (!(is_oper(u->user->nick))) { 
    send_cmd(s_ChanServ, "KICK %s %s :%s", canal, u->user->nick, achanakicks[i].reason);
privmsg(s_ChanServ,ni->nick, "12ENFORCE 3KICK usado sobre 3%s",u->user->nick);
count++;
#endif

} else { 
//solo si el que ejecuta comando es OPER,revelo que en el ENFORCE cazó a un OPER
   if (is_oper(ni->nick))
    privmsg(s_ChanServ,ni->nick, "12ENFORCE 5BAN/KICK No usado sobre OPER 5%s",u->user->nick);

} //fin if si no es oper

    //return 1;
	    }
 
i++;
}

}
	notice_lang(s_ChanServ, u2, CHAN_AKICK_ENFORCE_DONE, canal, count);   

       } else if (stricmp(cmd, "COUNT") == 0) {
           if (!check_access(u, ci, CA_AKICK_LIST) && !is_services_oper(u)) {
             if (ci->founder && getlink(ci->founder) == u->ni) {
	    notice_lang(s_ChanServ, u, CHAN_IDENTIFY_REQUIRED, s_ChanServ,canal);
            return; }
	else {
	    notice_lang(s_ChanServ, u, ACCESS_DENIED);
            return;
            }
         }
	i=0;
	cont=0;
	  if (!mask) {
		 mask ="*";		
			}
	//notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_HEADER, canal);


        while (i < nachanakick )  {

	if ((stricmp(achanakicks[i].canal, canal) == 0 &&  (!mask ||
	match_wild(mask, achanakicks[i].nick)))) { 

	cont++;
	
	    }
	i++;
	}
privmsg(s_ChanServ, u->nick, " El canal %s tiene %d entradas en su lista de autokick",canal,cont);

  } else if (stricmp(cmd, "LIST") == 0) {
        if (!check_access(u, ci, CA_AKICK_LIST) && !is_services_oper(u)) {
             if (ci->founder && getlink(ci->founder) == u->ni) {
	    notice_lang(s_ChanServ, u, CHAN_IDENTIFY_REQUIRED, s_ChanServ,canal);
            return; }
	else {
	    notice_lang(s_ChanServ, u, ACCESS_DENIED);
            return;
            }
         }
	//expire_achanakicks();
	 if (!mask) {
		 mask ="*";		
				}

if (strchr(mask, '@'))
	    strlower(strchr(mask, '@'));


	i=0;
	cont=0;
	notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_HEADER, canal);


        while (i < nachanakick )  {

	if ((stricmp(achanakicks[i].canal, canal) == 0 &&  (!mask ||
	match_wild(mask, achanakicks[i].nick)))) { 

	cont++;
	
	privmsg(s_ChanServ, u->nick, "%d: %s(%s)-->%s",cont,achanakicks[i].nick,achanakicks[i].reason,achanakicks[i].who);
 //privmsg(s_ChanServ, u->nick, "%s", buf);

	    }
	i++;
	}

    } else if (stricmp(cmd, "VIEW") == 0) {
         if (!check_access(u, ci, CA_AKICK_VIEW) && !is_services_oper(u)) {
             if (ci->founder && getlink(ci->founder) == u->ni) {
	    notice_lang(s_ChanServ, u, CHAN_IDENTIFY_REQUIRED, s_ChanServ,canal);
            return; }
	else {
	    notice_lang(s_ChanServ, u, ACCESS_DENIED);
            return;
            }
         }
	//expire_achanakicks();
	 if (!mask) {
		 mask ="*";		
				}

if (strchr(mask, '@'))
	    strlower(strchr(mask, '@'));

	i=0;
	cont=0;
	notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_HEADER, canal);

	 while (i < nachanakick )  {

  if ((stricmp(achanakicks[i].canal, canal) == 0 &&  (!mask ||
	match_wild(mask, achanakicks[i].nick)))) { 
	   char timebuf[32],timecaza[32],tcazakick[32],buf[BUFSIZE];	
		struct tm tm;
		struct tm tv;
		time_t t = time(NULL);
		time_t v;
		tm = *localtime(achanakicks[i].time ? &achanakicks[i].time : &t);
		strftime_lang(timebuf, sizeof(timebuf),
			u, STRFTIME_DATE_TIME_FORMAT, &tm);

		/*tv = *localtime(achanakicks[i].caza_akick ? &achanakicks[i].caza_akick : &v);
		strftime_lang(timecaza, sizeof(timecaza),
			u, STRFTIME_DATE_TIME_FORMAT, &tv);*/
		if (achanakicks[i].caza_akick) {

tv = *localtime(&achanakicks[i].caza_akick);                     
 
   strftime_lang(tcazakick, sizeof(tcazakick), u, STRFTIME_DATE_TIME_FORMAT, &tv);
snprintf(buf, sizeof(buf), ",último uso  el 12%s",tcazakick);

} else {
       snprintf(buf, sizeof(buf), ",nunca usado");
	
}
cont++;
privmsg(s_ChanServ, u->nick, "%d:%s (%s) (puesto por %s %s) %s",cont,achanakicks[i].nick,achanakicks[i].reason,achanakicks[i].who,timebuf,buf);
		/*notice_lang(s_ChanServ, u, OPER_AKILL_VIEW_FORMAT,
				achanakicks[i].nick,
				*achanakicks[i].who ? achanakicks[i].who : "<desconocido>",
				timebuf, expirebuf, achanakicks[i].nick);*/
	    }
   	i++;
	}


} else {
	syntax_error(s_ChanServ, u, "AKICK", CHAN_AKICK_SYNTAX);
    }
}

char *masktonick(const char *chan,const char *masc) {

    Channel *c = chan ? findchan(chan) : NULL;
    struct c_userlist *u;
char *nick,*unick, *ident, *ip;
char *mask;

if (!c) {
	return NULL;
    }
   for (u = c->users; u; u = u->next) {
	mask = create_mask(u->user);
	if (match_wild(mask,masc))
	    split_usermask(mask, &unick, &ident, &ip);
		return nick;
}
  

}
int check_akick(User *user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);
    AutoKick *akick;
    int i;
    NickInfo *ni;
    char *av[3], *mask;
    const char *reason;
    Timeout *t;
    int stay;


    if (!ci)
	return 0;

    if (is_oper(user->nick) || is_services_oper(user))
	return 0;

/* Si el canal esta suspendido, no deberia rular los akicks
 * por esto lo desactivo
 */
    if (ci->flags & CI_SUSPEND)
        return 0;

 

    if (nick_recognized(user))
	ni = user->ni;
    else
	ni = NULL;



   mask = create_mask(user);
char mascara[BUFSIZE];
     snprintf(mascara, sizeof(mascara), "%s!*@*", user->nick);
 
char masknickuserip[BUFSIZE];
      snprintf(masknickuserip, sizeof(masknickuserip), "%s!%s@%s",user->nick,user->username,user->host);
char maskuserip[BUFSIZE];
      snprintf(maskuserip, sizeof(maskuserip), "*!%s@%s",user->username,user->host);
char maskip[BUFSIZE];
      snprintf(maskip, sizeof(maskip), "*!*@%s",user->host);

   i=0;
  while (i < nachanakick )  {
	    if ((stricmp(achanakicks[i].canal, chan) == 0 && (stricmp(achanakicks[i].nick, strlower(mascara)) == 0 || match_wild(achanakicks[i].nick,strlower(masknickuserip)) ||  match_wild(achanakicks[i].nick,strlower(maskuserip)) ||  match_wild(achanakicks[i].nick,strlower(maskip))))) {
         reason = achanakicks[i].reason;
	achanakicks[i].caza_akick= time(NULL);
		

goto kick;
	    }
 
i++;
}
if (time(NULL)-start_time >= CSRestrictDelay
				&& check_access(user, ci, CA_NOJOIN)) {
	mask = create_mask(user);
	reason = getstring(user->ni, CHAN_NOT_ALLOWED_TO_JOIN);
	  
	goto kick;
    }


    return 0;

kick:
   
    if (debug) {
	log("debug: channel: AutoKicking %s!%s@%s",
		user->nick, user->username, user->host);
   }

	

   

    /* Remember that the user has not been added to our channel user list
     * yet, so we check whether the channel does not exist */
    stay = !findchan(chan);
    av[0] = sstrdup(chan);
    if (stay) {
	send_cmd(s_ShadowServ, "JOIN %s", chan);
    }
    strcpy(av[0], chan);
    av[1] = sstrdup("+b");
    av[2] = mask;
    send_cmd(s_ChanServ, "MODE %s +b %s  %lu", chan, av[2], time(NULL));
	
    do_cmode(s_ChanServ, 3, av);
    free(av[0]);
    free(av[1]);
    free(av[2]);
#ifdef IRC_UNDERNET_P10
    send_cmd(s_ChanServ, "K %s %s :%s", chan, user->numerico, reason);
#else
    send_cmd(s_ChanServ, "KICK %s %s :%s", chan, user->nick, reason);
 

#endif

    return 1;
}

/*************************************************************************/
