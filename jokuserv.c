/* JokuServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 *(C) 2009 donostiarra  admin.euskalirc@gmail.com  http://euskalirc.wordpress.com
 *
 */

#include "services.h"
#include "pseudo.h"


/*************************************************************************/

/* Joku admin list */
static NickInfo *joku_admins[MAX_SERVADMINS];

/* Joku operator list */
static NickInfo *joku_opers[MAX_SERVOPERS];

/* Canales del BOT */
static ChannelInfo *canales[MAX_SERVOPERS];
/* Número del canal */
/*static ChannelInfo *numero;*/



/*************************************************************************/
static void do_credits(User *u);
static void do_help(User *u);

static void do_admin(User *u);

static void do_oper(User *u);
static void do_canal(User *u);

/*************************************************************************/

static Command cmds[] = {
    { "CREDITS",    do_credits,    NULL,  -1,                   -1,-1,-1,-1 },
    
    { "CREDITOS",   do_credits,    NULL,  -1,                   -1,-1,-1,-1 },        
    { "HELP",       do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "AYUDA",      do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "SHOWCOMMANDS",    do_help,  NULL,  -1,                   -1,-1,-1,-1 },
    { ":?",         do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "?",          do_help,       NULL,  -1,                   -1,-1,-1,-1 },                
  
    { "ADMIN",      do_admin,      is_services_oper,  JOKU_HELP_ADMIN,      -1,-1,-1,-1 },
    
   { "OPER",       do_oper,       is_joku_oper,  JOKU_HELP_OPER,       -1,-1,-1,-1 },
   { "STAFF",  		staffjoku,    NULL,           -1,-1,-1,-1 },
   { "CANAL",  		do_canal,   is_joku_oper,    JOKU_HELP_CANAL,      -1,-1,-1,-1 },

    /* Fencepost: */
    { NULL }
};

/*************************************************************************/
/*************************************************************************/

/* JokuServ initialization. */

void jok_init(void)
{
    
}

/*************************************************************************/

/* Main JokuServ routine. */

void jokserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	logeo("%s: user record for %s not found", s_JokuServ, source);
	notice(s_JokuServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    logeo("%s: %s: %s", s_JokuServ, source, buf);

    cmd = strtok(buf, " ");
    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_JokuServ, source, "\1PING %s", s);
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_JokuServ, source, "\1VERSION %s %s -- %s\1",
               PNAME, s_JokuServ, version_build);                
    } else {
	run_cmd(s_JokuServ, u, cmds, cmd);
    }
}

/*************************************************************************/
/**************************** Privilege checks ***************************/
/*************************************************************************/

/* Load JokuServ data. */

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de escritura en %s", JokuDBName);	\
	failed = 1;					\
	break;						\
    }							\
} while (0)

void load_jok_dbase(void)
{
    dbFILE *f;
    int16 i, n, ver;
    char *s;
    int failed = 0;

    if (!(f = open_db(s_JokuServ, JokuDBName, "r")))
	return;
    switch (ver = get_file_version(f)) {
      case 9:
      case 8:
	SAFE(read_int16(&n, f));
	for (i = 0; i < n && !failed; i++) {
	    SAFE(read_string(&s, f));
	    if (s && i < MAX_SERVADMINS)
		joku_admins[i] = findnick(s);
	    if (s)
		free(s);
	}
	if (ver >= 8) {
	   SAFE(read_int16(&n, f));
	   for (i = 0; i < n && !failed; i++) {
	       SAFE(read_string(&s, f));
	       if (s && i < MAX_SERVOPERS)
	  	   joku_opers[i] = findnick(s);
	       if (s)
	  	   free(s);
	   }
	    SAFE(read_int16(&n, f));
	   for (i = 0; i < n && !failed; i++) {
	       SAFE(read_string(&s, f));
	       if (s && i < MAX_SERVOPERS)
	  	   canales[i] = cs_findchan(s);
		
	       if (s)
	  	   free(s);
	   }
	}

	
	int32 tmp32;
	SAFE(read_int32(&maxusercnt, f));
	SAFE(read_int32(&tmp32, f));
	maxusertime = tmp32;
	break;

      default:
	fatal("Version no soportada (%d) en %s", ver, OperDBName);
    } /* switch (version) */
    close_db(f);
}

#undef SAFE

/*************************************************************************/

/* Save JokuServ data. */

#define SAFE(x) do {						\
    if ((x) < 0) {						\
	restore_db(f);						\
	log_perror("Error de escritura en %s", JokuDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {		\
	    canalopers(NULL, "Error de escritura en %s: %s", JokuDBName,	\
			strerror(errno));			\
	    lastwarn = time(NULL);				\
	}							\
	return;							\
    }								\
} while (0)

void save_jok_dbase(void)
{
    dbFILE *f;
    int16 i, count = 0;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_JokuServ, JokuDBName, "w")))
	return;
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (joku_admins[i])
	    count++;
    }
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (joku_admins[i])
	    SAFE(write_string(joku_admins[i]->nick, f));
    }
  count = 0;
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (joku_opers[i])
	    count++;
    }
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (joku_opers[i])
	    SAFE(write_string(joku_opers[i]->nick, f));
    }
  count = 0;
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (canales[i])
	    count++;
    }
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (canales[i])
	    SAFE(write_string(canales[i]->name, f));
    }

    SAFE(write_int32(maxusercnt, f));
    SAFE(write_int32(maxusertime, f));
    close_db(f);
}

#undef SAFE



/* Does the given user have Joku admin privileges? */

int is_joku_admin(User *u)
{
    int i;
/**    if (!(u->mode & UMODE_O))
	return 0;     ***/
    if (is_services_oper(u))
	return 1;
    if (skeleton)
	return 1;
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (joku_admins[i] && u->ni == getlink(joku_admins[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}
/*************************************************************************/
/* Does the given user have Joku Oper  privileges? */



int is_joku_oper(User *u)
{
    int i;
 
    if (is_joku_admin(u))
	return 1;
    if (skeleton)
	return 1;
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (joku_opers[i] && u->ni == getlink(joku_opers[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}


/* Listar los OPERS/ADMINS on-line */

void staffjoku(User *u)
{
    int i;
    int online = 0;
    NickInfo *ni, *ni2;
    
    
    
    
    for (i = 0; i < MAX_SERVOPERS; i++) {
         if (joku_opers[i]) {
             ni = findnick(joku_opers[i]->nick);
             if (ni && (ni->status & NS_IDENTIFIED)) {
#if defined(IRC_UNDERNET_P10)
                 privmsg(s_JokuServ, u->numerico, "%-10s es 12MASTER-OPER de 4%s",
#else
                 privmsg(s_JokuServ, u->nick, "%-10s es 12MASTER-OPER de 4%s",
#endif
                     joku_opers[i]->nick, s_JokuServ);
                 online++;
              }
         }     
    }

                                                                                      
    for (i = 0; i < MAX_SERVADMINS; i++) {
         if (joku_admins[i]) {
             ni2 = findnick(joku_admins[i]->nick);
             if (ni2 && (ni2->status & NS_IDENTIFIED)) {             
#if defined(IRC_UNDERNET_P10)
                 privmsg(s_JokuServ, u->numerico, "%-10s es 12OWNER-ADMIN de 4%s",
#else
                 privmsg(s_JokuServ, u->nick, "%-10s es 12OWNER-ADMIN de 4%s",
#endif
                     joku_admins[i]->nick, s_JokuServ);
                 online++;
             }
         }    
    }
    
   
#if defined(IRC_UNDERNET_P10)
    privmsg(s_JokuServ, u->numerico, "12%d Miembros Staff del  Bot on-line", online);
#else
    privmsg(s_JokuServ, u->nick, "12%d Miembros Staff del  Bot on-line", online);
#endif    
}
            

/*************************************************************************/



int nick_is_joku_admin(NickInfo *ni)
{
    int i;

    if (!ni)
	return 0;

    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (joku_admins[i] && getlink(ni) == getlink(joku_admins[i]))
	    return 1;
    }
    return 0;
}



/* El nick es Oper de Game*/

int nick_is_joku_oper(NickInfo *ni)
{
   int i;
   
   if (!ni)
       return 0;
  
   if (nick_is_joku_admin(ni))
       return 1;
      
   for (i = 0; i < MAX_SERVOPERS; i++) {
       if (joku_opers[i] && getlink(ni) == getlink(joku_opers[i]))
       return 1;
   }
   return 0;
}
/*************************************************************************/


/* Expunge a deleted nick from the Services admin/oper lists. */

void jok_remove_nick(const NickInfo *ni)
{
    int i;
const ChannelInfo *ci;

    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (joku_admins[i] == ni)
	    joku_admins[i] = NULL;
    }
   
 for (i = 0; i < MAX_SERVOPERS; i++) {
	if (joku_opers[i] == ni)
	   joku_opers[i] = NULL;
    }
   for (i = 0; i < MAX_SERVOPERS; i++) {
	if (canales[i] == ci)
	   canales[i] = NULL;
    }
}

/*************************************************************************/
/*********************** JokuServ command functions **********************/
/*************************************************************************/
static void do_credits(User *u)
{
    notice_lang(s_JokuServ, u, SERVICES_CREDITS);
}
    
/***********************************************************************/
    

/* HELP command. */

static void do_help(User *u)
{
    const char *cmd = strtok(NULL, "");
	
    if (!cmd) {
	 if  (is_joku_admin(u))
	notice_help(s_JokuServ, u, JOKU_ADMIN_HELP);
	else if  (is_joku_oper(u))
	notice_help(s_JokuServ, u, JOKU_OPER_HELP);
	else if  (!is_joku_oper(u))
	notice_help(s_JokuServ, u, JOKU_HELP);
	} else {
	help_cmd(s_JokuServ, u, cmds, cmd);
    }

}


/* Services admin list viewing/modification. */

static void do_admin(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i;
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if (!is_services_oper(u)) {
	    notice_lang(s_JokuServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_JokuServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_JokuServ, u->nick, "No pueden añadirse nicks que no esten migrados a la BDD");
	       return;
            }
	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (!joku_admins[i] || joku_admins[i] == ni)
		    break;
	    }
	    if (joku_admins[i] == ni) {
		notice_lang(s_JokuServ, u, JOKU_ADMIN_EXISTS, ni->nick);
	    } else if (i < MAX_SERVADMINS) {
		joku_admins[i] = ni;
		notice_lang(s_JokuServ, u, JOKU_ADMIN_ADDED, ni->nick);
		canaladmins(s_JokuServ, "12%s añade a 12%s como ADMIN del BOT", u->nick, ni->nick);
		
	    } else {
		notice_lang(s_JokuServ, u, JOKU_ADMIN_TOO_MANY, MAX_SERVADMINS);
	    }
	    if (readonly)
		notice_lang(s_JokuServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_JokuServ, u, "ADMIN", JOKU_ADMIN_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_services_oper(u)) {
	    notice_lang(s_JokuServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_JokuServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	   	   	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (joku_admins[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVADMINS) {
		joku_admins[i] = NULL;
		notice_lang(s_JokuServ, u, JOKU_ADMIN_REMOVED, ni->nick);
		canaladmins(s_JokuServ, "12%s borra a 12%s como ADMIN del BOT", u->nick, ni->nick);
		
		if (readonly)
		    notice_lang(s_JokuServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_JokuServ, u, JOKU_ADMIN_NOT_FOUND, ni->nick);
	    }
	} else {
	    syntax_error(s_JokuServ, u, "ADMIN", JOKU_ADMIN_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_JokuServ, u, JOKU_ADMIN_LIST_HEADER);
	for (i = 0; i < MAX_SERVADMINS; i++) {
	    if (joku_admins[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_JokuServ, u->numerico, "%s", joku_admins[i]->nick);
#else	    
		privmsg(s_JokuServ, u->nick, "%s", joku_admins[i]->nick);
#endif
	}

    } else {
	syntax_error(s_JokuServ, u, "ADMIN", JOKU_ADMIN_SYNTAX);
    }
}


static void do_oper(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i;
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if (!is_joku_admin(u)) {
	    notice_lang(s_JokuServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_JokuServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_JokuServ, u->nick, "No pueden añadirse nicks que no esten migrados a la BDD");
	       return;
            }
	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (!joku_opers[i] || joku_opers[i] == ni)
		    break;
	    }
	    if (joku_opers[i] == ni) {
		notice_lang(s_JokuServ, u, JOKU_OPER_EXISTS, ni->nick);
	    } else if (i < MAX_SERVOPERS) {
		joku_opers[i] = ni;
		notice_lang(s_JokuServ, u, JOKU_OPER_ADDED, ni->nick);
		canaladmins(s_JokuServ, "12%s añade a 12%s como OPER del BOT", u->nick, ni->nick);
		
	    } else {
		notice_lang(s_JokuServ, u, JOKU_OPER_TOO_MANY, MAX_SERVOPERS);
	    }
	    if (readonly)
		notice_lang(s_JokuServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_JokuServ, u, "OPER", JOKU_OPER_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_joku_admin(u)) {
	    notice_lang(s_JokuServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_JokuServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	   	   	    
	    for (i = 0; i < MAX_SERVOPERS; i++) {
		if (joku_opers[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVOPERS) {
		joku_opers[i] = NULL;
		notice_lang(s_JokuServ, u, JOKU_OPER_REMOVED, ni->nick);
		canaladmins(s_JokuServ, "12%s borra a 12%s como OPER del BOT", u->nick, ni->nick);
		
		if (readonly)
		    notice_lang(s_JokuServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_JokuServ, u, JOKU_OPER_NOT_FOUND, ni->nick);
	    }
	} else {
	    syntax_error(s_JokuServ, u, "OPER", JOKU_OPER_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_JokuServ, u, JOKU_OPER_LIST_HEADER);
	for (i = 0; i < MAX_SERVOPERS; i++) {
	    if (joku_opers[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_JokuServ, u->numerico, "%s", joku_opers[i]->nick);
#else	    
		privmsg(s_JokuServ, u->nick, "%s", joku_opers[i]->nick);
#endif
	}

    } else {
	syntax_error(s_JokuServ, u, "OPER", JOKU_OPER_SYNTAX);
    }
}
static void do_canal(User *u)
{
    char *cmd, *canal;
    ChannelInfo *ci;
    int i;
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ENTRADA") == 0) {
	if (!is_joku_oper(u)) {
	    notice_lang(s_JokuServ, u, PERMISSION_DENIED);
	    return;
	}
	canal = strtok(NULL, " ");
	if (canal) {
	    if (!(ci = cs_findchan(canal))) {
		notice_lang(s_JokuServ, u, CHAN_X_NOT_REGISTERED, canal);
		return;
	    } 
	    
	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (!canales[i] || canales[i] == ci)
		    break;
	    }
	    if (canales[i] == ci) {
		notice_lang(s_JokuServ, u, JOKU_CANAL_EXISTS, ci->name);
	    } else if (i < MAX_SERVOPERS) {
		canales[i] = ci;
		notice_lang(s_JokuServ, u, JOKU_CANAL_ADDED, ci->name);
		canaladmins(s_JokuServ, "12%s añade a 12%s como CANAL del BOT", u->nick, ci->name);
		send_cmd(s_JokuServ, "JOIN  %s", ci->name);
	    } else {
		notice_lang(s_JokuServ, u, JOKU_CANAL_TOO_MANY, MAX_SERVOPERS);
	    }
	    if (readonly)
		notice_lang(s_JokuServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_JokuServ, u, "CANAL", JOKU_CANAL_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "SALIDA") == 0) {
	if (!is_joku_oper(u)) {
	    notice_lang(s_JokuServ, u, PERMISSION_DENIED);
	    return;
	}
	canal = strtok(NULL, " ");
	if (canal) {
	    if (!(ci = cs_findchan(canal))) {
		notice_lang(s_JokuServ, u, CHAN_X_NOT_REGISTERED, canal);
		return;
	    }
	   	   	    
	    for (i = 0; i < MAX_SERVOPERS; i++) {
		if (canales[i] == ci)
		    break;
	    }
	    if (i < MAX_SERVOPERS) {
		canales[i] = NULL;
		notice_lang(s_JokuServ, u, JOKU_CANAL_REMOVED, ci->name);
		canaladmins(s_JokuServ, "12%s borra a 12%s como CANAL del BOT", u->nick, ci->name);
		send_cmd(s_JokuServ, "PART  %s", ci->name);
		if (readonly)
		    notice_lang(s_JokuServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_JokuServ, u, JOKU_CANAL_NOT_FOUND, ci->name);
	    }
	} else {
	    syntax_error(s_JokuServ, u, "CANAL", JOKU_CANAL_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_JokuServ, u, JOKU_CANAL_LIST_HEADER);
	for (i = 0; i < MAX_SERVOPERS; i++) {
	    if (canales[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_JokuServ, u->numerico, "%s", canales[i]->name);
#else	    
		privmsg(s_JokuServ, u->nick, "%s", canales[i]->name);
#endif
	}

    } else {
	syntax_error(s_JokuServ, u, "CANAL", JOKU_CANAL_SYNTAX);
    }
}
void join_jokuserv(void)
{
    /*ChannelInfo *ci;*/
    int i;
            
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (canales[i])
#if defined(IRC_UNDERNET_P10)
             send_cmd(s_JokuServ, "J %s", canales[i]->name);
         #else
             send_cmd(s_JokuServ, "JOIN %s", canales[i]->name);
	               
#endif
       } 
    }

void joku_canal(const char *source,const char *chan, char *buf)
 {
 /*int i;*/
char *cmd;
 char adm_canal[BUFSIZE];
char op_canal[BUFSIZE];
snprintf(adm_canal, sizeof(adm_canal), "#%s", CanalAdmins);
snprintf(op_canal, sizeof(op_canal), "#%s", CanalOpers);
  /*User *u = finduser(source);*/
   

 
if (!strcmp(chan, adm_canal) || !strcmp(chan, op_canal)) return;
 cmd = strtok(buf, " ");

 	if (strstr(cmd, "!n")) {
	
            static char saltChars[] = "123456789123456789123456789123456789123456789";
            unsigned long long xi;
	    /* char salt[ilevel + 2]; */
	    char salt[5];
	    /*salt = (char *) malloc((ilevel+1)*sizeof(char));*/
	    int cnt = 0;
	    salt[4] = '\0';
            __asm__ volatile (".byte 0x0f, 0x31" : "=A" (xi));
	    srandom(xi);

	        for(cnt = 0; cnt < (4); ++cnt)
		  salt[cnt] = saltChars[random() % 45];
	privmsg(s_JokuServ, chan, " 12%s  :ahi va tu cifra aleatoria 4 %s", source,salt);
	       			return;
	} else if (strstr(cmd, "!f")) {
				privmsg(s_JokuServ, chan, "12%s :  Finalizando! ", source);
	       			return;
 }
}
