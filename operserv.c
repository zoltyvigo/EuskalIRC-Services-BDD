/* OperServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"
#include "pseudo.h"
#if defined(SOPORTE_JOOMLA15)
#include <mysql.h>
#endif
#define MAXPARAMS	8
#define aliases 6000
/*************************************************************************/
int32 autogeoip=0;
int32 notifinouts=0;
/* Services admin list */
/*static NickInfo *services_admins[MAX_SERVADMINS];*/
NickInfo *services_admins[MAX_SERVADMINS];
/* Services devel list */
/*static NickInfo *services_devels[MAX_SERVDEVELS];*/
NickInfo *services_devels[MAX_SERVDEVELS];
/* Services operator list */
/*static NickInfo *services_opers[MAX_SERVOPERS];*/
NickInfo *services_opers[MAX_SERVOPERS];
/* Services patrocinadores list */
/*static NickInfo *services_patrocinas[MAX_SERVPATROCINAS];*/
NickInfo *services_patrocinas[MAX_SERVPATROCINAS];

/* Services bots list */
/*static NickInfo *services_bots[MAX_SERVOPERS];*/
NickInfo *services_bots[MAX_SERVOPERS];
/* Services cregadmins-coadmins list */
/*static NickInfo *services_cregadmins[MAX_SERVADMINS];*/
NickInfo *services_cregadmins[MAX_SERVADMINS];

/*************************************************************************/
static void do_credits(User *u);
static void do_help(User *u);
static void do_global(User *u);
static void do_globaln(User *u);
static void do_director(User *u);
static void do_admin(User *u);
static void do_devel(User *u);
static void do_patrocina(User *u);
static void do_oper(User *u);
static void do_coadmin(User *u);
static void do_os_op(User *u);
static void do_os_deop(User *u);
static void do_os_voice(User *u);
static void do_os_devoice(User *u);
static void do_os_mode(User *u);
static void do_os_kick(User *u);
static void do_clearmodes(User *u);
static void do_apodera(User *u);
static void do_limpia(User *u);
static void do_block(User *u);
static void do_unblock(User *u);
static void do_set(User *u);
static void do_settime(User *u);
static void do_jupe(User *u); 
static void do_raw(User *u);
static void do_update(User *u);
static void do_os_quit(User *u);
static void do_shutdown(User *u);
static void do_restart(User *u);
static void do_reload(User *u);
static void do_listignore(User *u);
static void do_debugserv(User *u);
static void do_skill (User *u);
static void do_vhost (User *u);
static void do_matchwild(User *u);
static void do_svsjoinparts(User *u);
static void do_svsmodes(User *u);

/*************************************************************************/

static Command cmds[] = {
    { "CREDITS",    do_credits,    NULL,  -1,                   -1,-1,-1,-1 },
    { "VHOST",	    do_vhost,	   is_services_cregadmin,   OPER_HELP_VHOST,   -1,-1,-1,-1 },
    { "CREDITOS",   do_credits,    NULL,  -1,                   -1,-1,-1,-1 },        
    { "HELP",       do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "AYUDA",      do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "SHOWCOMMANDS",    do_help,  NULL,  -1,                   -1,-1,-1,-1 },
    { ":?",         do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "?",          do_help,       NULL,  -1,                   -1,-1,-1,-1 },                
    { "GLOBAL",     do_global,     NULL,  OPER_HELP_GLOBAL,     -1,-1,-1,-1 },
    { "GLOBALN",    do_globaln,    NULL,  OPER_HELP_GLOBAL,     -1,-1,-1,-1 },
    { "SPAM",       do_spam,       NULL,  OPER_HELP_SPAM,      -1,-1,-1,-1 },
    { "DUDA",	    do_euskal,	   is_services_oper,   OPER_HELP_DUDA,   -1,-1,-1,-1 },
	/*Para el Bot EuskalIRC*/
    { "GEOIP",	    do_geoip,	   is_services_oper,   GEOIP_HELP_DUDA,   -1,-1,-1,-1 },
	/*#ifdef SOPORTE_JOOMLA15
     { "MYSQL",	   geoip_mysql,	   is_services_oper,   -1,   -1,-1,-1,-1 },
	#endif*/
    { "FORZAR",	    do_svsjoinparts,	   is_services_oper,   OPER_HELP_FORZAR,   -1,-1,-1,-1 },
    { "MODOS",	    do_svsmodes,	   is_services_admin,   OPER_HELP_MODOS,   -1,-1,-1,-1 }, /*is_services_admin*/


	/* --donostiarra(2009)--
        con este ejemplo,ahorramos codigo en "MODOS" no asi en "FORZAR",
	Que queriendo restringir mas aun el acceso,
	con  <CregAdmin>*,nos obliga a declarar mas condicionales*/

   

    /* Anyone can use the LIST option to the ADMIN and OPER commands; those
     * routines check privileges to ensure that only authorized users
     * modify the list. */
    { "DIRECTOR",      do_director,      NULL,  OPER_HELP_DIRECTOR,      -1,-1,-1,-1 },
    { "ADMIN",      do_admin,      NULL,  OPER_HELP_ADMIN,      -1,-1,-1,-1 },
    { "DEVEL",      do_devel,      NULL,  OPER_HELP_DEVEL,      -1,-1,-1,-1 },
    { "OPER",       do_oper,       NULL,  OPER_HELP_OPER,       -1,-1,-1,-1 },
    { "COADMIN",  do_coadmin,  NULL,  CREG_SERVADMIN_HELP,      -1,-1,-1,-1 },
    { "PATROCINA",       do_patrocina,       NULL,  OPER_HELP_PATROCINA,       -1,-1,-1,-1 },
         /* Similarly, anyone can use *NEWS LIST, but *NEWS {ADD,DEL} are
     * reserved for Services admins. */
    { "LOGONNEWS",  do_logonnews,  NULL,  NEWS_HELP_LOGON,      -1,-1,-1,-1 },
    { "OPERNEWS",   do_opernews,   NULL,  NEWS_HELP_OPER,       -1,-1,-1,-1 },

    /* Commands for Services opers: */
    { "OP",         do_os_op,      is_services_oper,
        -1,-1,-1,-1,-1},
    { "DEOP",       do_os_deop,    is_services_oper,
        -1,-1,-1,-1,-1},        
     { "VOICE",         do_os_voice,      is_services_oper,
        -1,-1,-1,-1,-1},
    { "DEVOICE",       do_os_devoice,    is_services_oper,
        -1,-1,-1,-1,-1},        
    { "MODE",       do_os_mode,    is_services_oper,
	OPER_HELP_MODE, -1,-1,-1,-1 },
    { "CLEARMODES", do_clearmodes, is_services_oper,
	OPER_HELP_CLEARMODES, -1,-1,-1,-1 },
    { "KICK",       do_os_kick,    is_services_oper,
	OPER_HELP_KICK, -1,-1,-1,-1 },
    { "BLOCK",      do_block,      is_services_oper,
            -1,-1,-1,-1,-1},
    { "UNBLOCK",    do_unblock,    is_services_oper,
            -1,-1,-1,-1,-1},
    { "UNGLINE",    do_unblock,    is_services_oper,
            -1,-1,-1,-1,-1},
    { "LIMPIA",     do_limpia,     is_services_oper,
            -1,-1,-1,-1,-1},
    { "APODERA",    do_apodera,    is_services_oper,
            -1,-1,-1,-1,-1},
    { "KILL", 	    do_skill,	   is_services_oper,
            OPER_HELP_KILL,-1,-1,-1,-1},
    { "SETTIME",    do_settime,    is_services_oper,
            -1,-1,-1,-1,-1},
    { "GLINE",      do_akill,      is_services_oper,
        OPER_HELP_AKILL,-1,-1,-1,-1},                                                                                    	
    { "AKILL",      do_akill,      is_services_oper,
	OPER_HELP_AKILL, -1,-1,-1,-1 },
      /* Commands for Services roots=directores: */
    { "SET",        do_set,        is_services_root,
	OPER_HELP_SET, -1,-1,-1,-1 },
    { "SET READONLY",0,0,  OPER_HELP_SET_READONLY, -1,-1,-1,-1 },
    { "SET DEBUG",0,0,     OPER_HELP_SET_DEBUG, -1,-1,-1,-1 },
    { "JUPE",       do_jupe,       is_services_admin,
	OPER_HELP_JUPE, -1,-1,-1,-1 }, 
    { "RAW",        do_raw,        is_services_root,
	OPER_HELP_RAW, -1,-1,-1,-1 },
    { "UPDATE",     do_update,     is_services_root,
	OPER_HELP_UPDATE, -1,-1,-1,-1 },
    { "QUIT",       do_os_quit,    is_services_root,
	OPER_HELP_QUIT, -1,-1,-1,-1 },
    { "SHUTDOWN",   do_shutdown,   is_services_root,
	OPER_HELP_SHUTDOWN, -1,-1,-1,-1 },
    { "RESTART",    do_restart,    is_services_root,
	OPER_HELP_RESTART, -1,-1,-1,-1 },
     { "RELOAD",    do_reload,    is_services_root,
	OPER_HELP_RELOAD, -1,-1,-1,-1 },
    { "LISTIGNORE", do_listignore, is_services_root,
	-1,-1,-1,-1, -1 },	
    { "DEBUGSERV", do_debugserv, is_services_root,
	-1,-1,-1,-1, -1 },
   /* { "MATCHWILD",  do_matchwild,       is_services_root, -1,-1,-1,-1,-1 },*/
{ "ROTATELOG",  rotate_log,  is_services_root, -1,-1,-1,-1,
	OPER_HELP_ROTATELOG },
  
 
 /* Commands for Services Roots: */

#if defined(DEBUG_COMMANDS)

  //  { "LISTTIMERS", send_timeout_list,  is_services_root, -1,-1,-1,-1,-1 },

#endif

    /* Fencepost: */
    { NULL }
};

/*************************************************************************/
/*************************************************************************/

/* OperServ initialization. */

void os_init(void)
{
    Command *cmd;

    cmd = lookup_cmd(cmds, "GLOBAL");
    if (cmd)
	cmd->help_param1 = s_GlobalNoticer;
    cmd = lookup_cmd(cmds, "ADMIN");
    if (cmd)
	cmd->help_param1 = s_NickServ;
    cmd = lookup_cmd(cmds, "OPER");
    if (cmd)
	cmd->help_param1 = s_OperServ;

}

/*************************************************************************/

/* Main OperServ routine. */

void operserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	logeo("%s: user record for %s not found", s_OperServ, source);
	notice(s_OperServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    logeo("%s: %s: %s", s_OperServ, source, buf);

    cmd = strtok(buf, " ");
    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_OperServ, source, "\1PING %s", s);
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_OperServ, source, "\1VERSION %s %s -- %s\1",
               PNAME, s_OperServ, version_build);                
    } else {
	run_cmd(s_OperServ, u, cmds, cmd);
    }
}
/*************************************************************************/

/* Main XServ routine. */

/*void xserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	log("%s: user record for %s not found", s_XServ, source);
	notice(s_XServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    log("%s: %s: %s", s_XServ, source, buf);

    cmd = strtok(buf, " ");
    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_XServ, source, "\1PING %s", s);
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_XServ, source, "\1VERSION %s %s -- %s\1",
               PNAME, s_XServ, version_build);                
    } else {
	run_cmd(s_XServ, u, cmds, cmd);
    }
}*/
/*************************************************************************/
/**************************** Privilege checks ***************************/
/*************************************************************************/

/* Load OperServ data. */

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de escritura en %s", OperDBName);	\
	failed = 1;					\
	break;						\
    }							\
} while (0)

void load_os_dbase(void)
{
    dbFILE *f;
    int16 i, n, ver;
    char *s;
    int failed = 0;

    if (!(f = open_db(s_OperServ, OperDBName, "r")))
	return;
    switch (ver = get_file_version(f)) {
      case 9:
      case 8:
	SAFE(read_int16(&n, f));
	for (i = 0; i < n && !failed; i++) {
	    SAFE(read_string(&s, f));
	    if (s && i < MAX_SERVADMINS)
		services_admins[i] = findnick(s);
	    if (s)
		free(s);
	}
	if (ver >= 8) {
	   SAFE(read_int16(&n, f));
	   for (i = 0; i < n && !failed; i++) {
	       SAFE(read_string(&s, f));
	       if (s && i < MAX_SERVDEVELS)
	  	   services_devels[i] = findnick(s);
	       if (s)
	  	   free(s);
	   }
	    SAFE(read_int16(&n, f));
	   for (i = 0; i < n && !failed; i++) {
	       SAFE(read_string(&s, f));
	       if (s && i < MAX_SERVADMINS)
	  	   services_cregadmins[i] = findnick(s);
	       if (s)
	  	   free(s);
	   }
	}
	
	if (!failed)
	    SAFE(read_int16(&n, f));
	for (i = 0; i < n && !failed; i++) {
	    SAFE(read_string(&s, f));
	    if (s && i < MAX_SERVOPERS)
		services_opers[i] = findnick(s);
	    if (s)
		free(s);
	}
	if (ver >= 8) {
	   SAFE(read_int16(&n, f));
	   for (i = 0; i < n && !failed; i++) {
	       SAFE(read_string(&s, f));
	       if (s && i < MAX_SERVPATROCINAS)
		   services_patrocinas[i] = findnick(s);
	       if (s)
	  	   free(s);
	   }
	  
	}
	
	int32 tmp32;
 	SAFE(read_int32(&allow_ignore, f));
	SAFE(read_int32(&autogeoip, f));
	SAFE(read_int32(&maxusercnt, f));
	SAFE(read_int32(&notifinouts, f));
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

/* Save OperServ data. */

#define SAFE(x) do {						\
    if ((x) < 0) {						\
	restore_db(f);						\
	log_perror("Error de escritura en %s", OperDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {		\
	    canalopers(NULL, "Error de escritura en %s: %s", OperDBName,	\
			strerror(errno));			\
	    lastwarn = time(NULL);				\
	}							\
	return;							\
    }								\
} while (0)

void save_os_dbase(void)
{
    dbFILE *f;
    int16 i, count = 0;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_OperServ, OperDBName, "w")))
	return;
    
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_admins[i])
	    count++;
    }
    SAFE(write_int16(count, f));

    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_admins[i])
	    SAFE(write_string(services_admins[i]->nick, f));
    }
    count = 0;
    for (i = 0; i < MAX_SERVDEVELS; i++) {
	if (services_devels[i])
	    count++;
    }
     
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVDEVELS; i++) {
	if (services_devels[i])
	    SAFE(write_string(services_devels[i]->nick, f));
    }
    count = 0;
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_cregadmins[i])
	    count++;
    }
     
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_cregadmins[i])
	    SAFE(write_string(services_cregadmins[i]->nick, f));
    }
    count = 0;
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (services_opers[i])
	    count++;
    }
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (services_opers[i])
	    SAFE(write_string(services_opers[i]->nick, f));
    }
    count = 0;
    for (i = 0; i < MAX_SERVPATROCINAS; i++) {
	if (services_patrocinas[i])
	    count++;
    }
    SAFE(write_int16(count, f));
    for (i = 0; i < MAX_SERVPATROCINAS; i++) {
	if (services_patrocinas[i])
	    SAFE(write_string(services_patrocinas[i]->nick, f));
    }
    SAFE(write_int32(allow_ignore, f));
    SAFE(write_int32(autogeoip, f));
    SAFE(write_int32(maxusercnt, f));
     SAFE(write_int32(notifinouts, f));
    SAFE(write_int32(maxusertime, f));
    close_db(f);
}

#undef SAFE

/*************************************************************************/

/* Does the given user have Services root privileges? */

int is_services_root(User *u)
{
    int i;
    for (i = 0; i <RootNumber ; i++) {
    if (/*(u->mode & UMODE_O) && */stricmp(u->nick, ServicesRoots[i]) == 0 && nick_identified(u))
	return 1;
     }
    if (skeleton)
	return 1;
    return 0;
}

/*************************************************************************/

/* Does the given user have Services admin privileges? */

int is_services_admin(User *u)
{
    int i;

/**    if (!(u->mode & UMODE_O))
	return 0;     ***/
    if (is_services_root(u))
	return 1;
    if (skeleton)
	return 1;
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_admins[i] && u->ni == getlink(services_admins[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}
/*************************************************************************/

/* Does the given user have Services cregadmin-coadmin privileges? */

int is_services_cregadmin(User *u)
{
    int i;

    if (is_services_admin(u))
	return 1;
    if (skeleton)
	return 1;
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_cregadmins[i] && u->ni == getlink(services_cregadmins[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}

/*************************************************************************/

/* Does the given user have Services devel privileges? */

int is_services_devel(User *u)
{
    int i;

/**    if (!(u->mode & UMODE_O))
	return 0;     ***/
    if (is_services_cregadmin(u))
	return 1;
    if (skeleton)
	return 1;
    for (i = 0; i < MAX_SERVDEVELS; i++) {
	if (services_devels[i] && u->ni == getlink(services_devels[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}
/**************************************************************************/
/* Does the given user have Services oper privileges? */

int is_services_oper(User *u)
{
    int i;

/**    if (!(u->mode & UMODE_O))
	return 0; ***/
    if (is_services_devel(u))
	return 1;
    if (skeleton)
	return 1;
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (services_opers[i] && u->ni == getlink(services_opers[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}
/*************************************************************************/
/**************************************************************************/
/* Does the given user have Services patrocina privileges? */

int is_services_patrocina(User *u)
{
    int i;

/**    if (!(u->mode & UMODE_O))
	return 0; ***/
   if (is_services_oper(u))
	return 1;
    if (skeleton)
	return 1;

    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (services_patrocinas[i] && u->ni == getlink(services_patrocinas[i])) {
	    if (nick_identified(u))
		return 1;
	    return 0;
	}
    }
    return 0;
}
/*************************************************************************/

/*************************************************************************/
/* Es este usuario un bot? */

int is_a_service(char *nick)
{
   if ((stricmp(nick, s_NickServ) == 0) || (stricmp(nick, s_ChanServ) == 0) || (stricmp(nick, s_CregServ) == 0) || (stricmp(nick, s_MemoServ) == 0) || (stricmp(nick, s_OperServ) == 0) || (stricmp(nick, s_ShadowServ) == 0) || (stricmp(nick, s_BddServ) == 0) || (stricmp(nick, s_HelpServ) == 0) || (stricmp(nick, s_GlobalNoticer) == 0) || (stricmp(nick, s_NewsServ) == 0) || (stricmp(nick, s_EuskalIRCServ) == 0) || (stricmp(nick, s_SpamServ) == 0)|| (stricmp(nick, s_IpVirtual) == 0) || (stricmp(nick, s_GeoIP) == 0) || (stricmp(nick, s_XServ) == 0))
	return 1;
  else
  	return 0;
}
 


/*************************************************************************/

/* Listar los OPERS/ADMINS on-line */

void ircops(User *u)
{
    int i,j/*,valor*/;
    int online=0;
    int director=0;
    NickInfo *ni/*, *ni2*/;
    
     for (i = 0; i < MAX_SERVPATROCINAS; i++) {
         if (services_patrocinas[i]) {
             ni = findnick(services_patrocinas[i]->nick);
             if (ni && (ni->status & NS_IDENTIFIED)) {
#if defined(IRC_UNDERNET_P10)
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12PATROCINADOR de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12PATROCINADOR de 4%s y 4%s",
#endif
                     services_patrocinas[i]->nick, s_NickServ, s_ChanServ);
                 online++;
              }
         }     
    }
    
    
    for (i = 0; i < MAX_SERVOPERS; i++) {
         if (services_opers[i]) {
             ni = findnick(services_opers[i]->nick);
             if (ni && (ni->status & NS_IDENTIFIED)) {
#if defined(IRC_UNDERNET_P10)
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12OPER de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12OPER de 4%s y 4%s",
#endif
                     services_opers[i]->nick, s_NickServ, s_ChanServ);
                 online++;
              }
         }     
    }
for (i = 0; i < MAX_SERVADMINS; i++) {
         if (services_cregadmins[i]) {
             ni = findnick(services_cregadmins[i]->nick);
             if (ni && (ni->status & NS_IDENTIFIED)) {
#if defined(IRC_UNDERNET_P10)
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12COADMIN de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12COADMIN de 4%s y 4%s",
#endif
                     services_cregadmins[i]->nick, s_NickServ, s_ChanServ);
                 online++;
              }
         }     
    }

    for (i = 0; i < MAX_SERVDEVELS; i++) {
         if (services_devels[i]) {
             ni = findnick(services_devels[i]->nick);
             if (ni && (ni->status & NS_IDENTIFIED)) {
#if defined(IRC_UNDERNET_P10)
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12DEVEL de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12DEVEL de 4%s y 4%s",
#endif
                     services_devels[i]->nick, s_NickServ, s_ChanServ);
                 online++;
              }
         }     
    }

                                                                                      
   
		 for (i = 0; i < MAX_SERVADMINS; i++) {
	    if (services_admins[i]) {
                   ni = findnick(services_admins[i]->nick);
		    if (ni && (ni->status & NS_IDENTIFIED) && !nick_is_services_root(services_admins[i])) { 
		privmsg(s_ChanServ, u->nick, "%-10s es 12Admin de 4%s",ni->nick,Net);
		online++;	
		}	
           }
         }     
  

 for (j = 0; j <RootNumber ; j++) {
	ni = findnick(ServicesRoots[j]);
	if (ni && (ni->status & NS_IDENTIFIED) && stricmp(ni->nick,ServicesRoots[j]) ==0) {
	    privmsg(s_ChanServ, u->nick, "%-10s es 12DIRECTOR de 5%s",ni->nick,Net);
		director++;
		} 
}
   
#if defined(IRC_UNDERNET_P10)
    privmsg(s_ChanServ, u->numerico, "12%d REPRESENTANTES de RED on-line", online);
#else
    privmsg(s_ChanServ, u->nick, "12%d REPRESENTANTES de RED  on-line", online);
#endif    
#if defined(IRC_UNDERNET_P10)
      privmsg(s_ChanServ, u->numerico, "12%d DIRECTORES  on-line", director);
#else
      privmsg(s_ChanServ, u->nick, "12%d DIRECTORES  on-line", director);
#endif    


}
            


/*************************************************************************/

/* Is the given nick a Services admin/root nick? */

/* NOTE: Do not use this to check if a user who is online is a services admin
 * or root. This function only checks if a user has the ABILITY to be a 
 * services admin. Rather use is_services_admin(User *u). -TheShadow */
int nick_is_services_root(NickInfo *ni)
{
    int i;

    if (!ni)
	return 0;
	for (i = 0; i <RootNumber ; i++) {
    if (stricmp(ni->nick, ServicesRoots[i]) == 0)
	return 1;
      }
       return 0;
}
int nick_is_services_admin(NickInfo *ni)
{
    int i;

    if (!ni)
	return 0;
	
   /*for (i = 0; i <RootNumber ; i++) {
    if (stricmp(ni->nick, ServicesRoots[i]) == 0)
	return 1;
      }*/
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_admins[i] && getlink(ni) == getlink(services_admins[i]))
	    return 1;
    }
    return 0;
}
int nick_is_services_cregadmin(NickInfo *ni)
{
    int i;

    if (!ni)
	return 0;
	
   /*for (i = 0; i <RootNumber ; i++) {
    if (stricmp(ni->nick, ServicesRoots[i]) == 0)
	return 1;
      }*/
     if (nick_is_services_admin(ni))
       return 1;
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_cregadmins[i] && getlink(ni) == getlink(services_cregadmins[i]))
	    return 1;
    }
    return 0;
}
int nick_is_services_devel(NickInfo *ni)
{
    int i;

    if (!ni)
	return 0;
	
     if (nick_is_services_admin(ni))
       return 1;
     if (nick_is_services_cregadmin(ni))
       return 1;
    for (i = 0; i < MAX_SERVDEVELS; i++) {
	if (services_devels[i] && getlink(ni) == getlink(services_devels[i]))
	    return 1;
    }
    return 0;
}
/*************************************************************************/

/* El nick es Oper de los Services */

int nick_is_services_oper(NickInfo *ni)
{
   int i;
   
   if (!ni)
       return 0;
   /*if (stricmp(ni->nick, ServicesRoot) == 0)
       return 1;*/
   if (nick_is_services_admin(ni))
       return 1;
    if (nick_is_services_cregadmin(ni))
       return 1;
   if (nick_is_services_devel(ni))
       return 1;
   for (i = 0; i < MAX_SERVOPERS; i++) {
       if (services_opers[i] && getlink(ni) == getlink(services_opers[i]))
       return 1;
   }
   return 0;
}
  /*************************************************************************/

/* El nick es Patrocinador de los Services */

int nick_is_services_patrocina(NickInfo *ni)
{
   int i;
   
   if (!ni)
       return 0;
   /*if (stricmp(ni->nick, ServicesRoot) == 0)
       return 1;*/
   
   for (i = 0; i < MAX_SERVPATROCINAS; i++) {
       if (services_patrocinas[i] && getlink(ni) == getlink(services_patrocinas[i]))
       return 1;
   }
   return 0;
}


/*************************************************************************/


/* Expunge a deleted nick from the Services admin/oper lists. */

void os_remove_nick(const NickInfo *ni)
{
    int i;

    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_admins[i] == ni)
	    services_admins[i] = NULL;
    }
    for (i = 0; i < MAX_SERVADMINS; i++) {
	if (services_cregadmins[i] == ni)
	    services_cregadmins[i] = NULL;
    }
    for (i = 0; i < MAX_SERVDEVELS; i++) {
	if (services_devels[i] == ni)
	    services_devels[i] = NULL;
    }
    for (i = 0; i < MAX_SERVOPERS; i++) {
	if (services_opers[i] == ni)
	    services_opers[i] = NULL;
    }
    for (i = 0; i < MAX_SERVPATROCINAS; i++) {
	if (services_patrocinas[i] == ni)
	    services_patrocinas[i] = NULL;
    }
}

/*************************************************************************/
/*********************** OperServ command functions **********************/
/*************************************************************************/
static void do_credits(User *u)
{
    notice_lang(s_OperServ, u, SERVICES_CREDITS);
}
    
/***********************************************************************/
    

/* HELP command. */

static void do_help(User *u)
{
    const char *cmd = strtok(NULL, "");
	
    if (!cmd) {
	if (is_services_root(u))
	notice_help(s_OperServ, u, OPER_ROOT_HELP,Net,CanalAyuda,SpamUsers);
    	else if  (is_services_admin(u))
	notice_help(s_OperServ, u, OPER_ADMIN_HELP,Net,CanalAyuda,SpamUsers);
	else if  (is_services_cregadmin(u))
	notice_help(s_OperServ, u, OPER_COADMIN_HELP,Net,CanalAyuda,SpamUsers);
	else if  (is_services_devel(u))
	notice_help(s_OperServ, u, OPER_DEVEL_HELP,Net,CanalAyuda,SpamUsers);
	else if  (is_services_oper(u))
	notice_help(s_OperServ, u, OPER_HELP,Net,CanalAyuda,SpamUsers);
	} else {
	help_cmd(s_OperServ, u, cmds, cmd);
    }

}
void do_svsmodes(User *u)
{    
        char *nick, *modos;
        nick = strtok(NULL, " ");
        modos = strtok(NULL, " ");
       /* NickInfo *ni;*/
	User *u2;
     
    if (!nick) {
             	 privmsg(s_OperServ,u->nick, " 4Falta un Nick /msg %s 2MODOS 12<NICK> 5<Modos>",s_OperServ);
    	return;
    } /* else if (!(ni = findnick(nick))) {
	notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	return;*/  
/*	} permito que se pueda cambiar los modos a los nicks no registrados*/
	 else if (!(u2 = finduser(nick)))  {  
         
         privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",nick);
         return;
         }
   
        if (!modos) {
         
    	 privmsg(s_OperServ,u->nick, " 4Falta Modos  /msg %s 2MODOS 12<NICK> 5<Modos>",s_OperServ);
    	return;
    } 
   
      
send_cmd(ServerName, "SVSMODE %s %s", nick,modos);
canaladmins( s_OperServ,"4%s 3SVSMODE  a  5%s:4[%s] ",u->nick,nick,modos);

 
  
      
}
void do_svsjoinparts(User *u)
{    
        char *cmd, *nick, *canal;
        cmd = strtok(NULL, " ");
        nick = strtok(NULL, " ");
        canal = strtok(NULL, " ");
	User *u2;
	Channel *c = canal ? findchan(canal) : NULL;
	  struct c_userlist *usuario;
        /*NickInfo *ni;*/
        /*ChannelInfo *ci;*/
        int cont = 0;
    if (!c) {
	privmsg(s_OperServ, u->nick, "Canal %s no encontrado!",
		canal ? canal : "(null)");
	return;
    }

    if ((!cmd) || ((!stricmp(cmd, "ENTRADA") == 0) && (!stricmp(cmd, "SALIDA") == 0))) {
	if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
           }
       privmsg(s_OperServ,u->nick, " /msg %s 2FORZAR 12ENTRADA/SALIDA 5<NICK> 2<CANAL>",s_OperServ);
    	return;
    }
    
    if (!nick) {
         if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
           }
    	 privmsg(s_OperServ,u->nick, " 4Falta un Nick /msg %s 2FORZAR 12ENTRADA/SALIDA 5<NICK> 2<CANAL>",s_OperServ);
    	return;
    } /*else if (!(ni = findnick(nick))) {
	notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	return;*/
   
   /* } permito que se pueda forzar la entrada/salida a los nicks no registrados*/

      
      if (stricmp(cmd, "ENTRADA") == 0) {
	if (!is_services_cregadmin(u)) {
	    notice_lang( s_OperServ, u, PERMISSION_DENIED);
	    return;
	}  

 if (!(u2 = finduser(nick)) && (stricmp(cmd, "ENTRADA") == 0))  {
               				privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",nick);
					return;
					}

 if ((u2 = finduser(nick)) && (stricmp(cmd, "ENTRADA") == 0))  {
		  for (usuario = c->users; usuario; usuario = usuario->next)
			 if (stricmp(nick, usuario->user->nick) == 0) {
			privmsg(s_OperServ,u->nick, "El Nick 5%s Ya está en el  Canal %sl!!.",nick,canal);
			return;
				}
		
			else if  (stricmp(nick, usuario->user->nick) != 0) {
					cont++;
				}
    			     }
	}

if (cont > 0) {
	send_cmd(ServerName, "SVSJOIN %s %s", nick,canal);
	canaladmins( s_OperServ,"12REP 5%s 3SVSJOIN de  2%s al Canal 5%s",u->nick,nick,canal);
  }

     if (stricmp(cmd, "SALIDA") == 0) {
	if (!is_services_cregadmin(u)) {
	    notice_lang( s_OperServ, u, PERMISSION_DENIED);
	    return;
	}  else if ((u2 = finduser(nick)))  {
		
		 for (usuario = c->users; usuario; usuario = usuario->next)
			 if (stricmp(nick, usuario->user->nick) == 0) {
	          send_cmd(ServerName, "SVSPART %s %s", nick,canal);
	canaladmins( s_OperServ,"12REP 5%s 3SVSPART de  2%s del Canal 5%s",u->nick,nick,canal);
	return;
       }  else
		 privmsg(s_OperServ,u->nick, "El Nick 5%s No está en el  Canal %s.",nick,canal);
}

          else 
               privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",nick);

          }

      
}


/*************************************************************************/

/* Global privmsg sending via GlobalNoticer. */

static void do_global(User *u)
{
    char *msg = strtok(NULL, "");
if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
    }
    if (!msg) {
        syntax_error(s_OperServ, u, "GLOBAL", OPER_GLOBAL_SYNTAX);
        return;
    }
#if HAVE_ALLWILD_NOTICE
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.%s :%s", NETWORK_DOMAIN, msg);
                            
#else
# if defined(NETWORK_DOMAIN)
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.%s :%s", NETWORK_DOMAIN, msg);
# else
    /* Go through all common top-level domains.  If you have others,
     * add them here.
     */

    send_cmd(s_GlobalNoticer, "PRIVMSG $*.es :%s", msg);
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.com :%s", msg);
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.net :%s", msg);
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.org :%s", msg);
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.edu :%s", msg);
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.eus :%s", msg);
    send_cmd(s_GlobalNoticer, "PRIVMSG $*.tk :%s", msg);
                    
# endif
#endif
 /*   canalopers(s_GlobalNoticer, "12%s ha enviado el GLOBAL (%s)", u->nick, msg); */
}
                                          
/*************************************************************************/

/* Global notice sending via GlobalNoticer. */

static void do_globaln(User *u)
{
    char *msg = strtok(NULL, "");
   if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
          }
    if (!msg) {
	syntax_error(s_OperServ, u, "GLOBAL", OPER_GLOBAL_SYNTAX);
	return;
    }
#if HAVE_ALLWILD_NOTICE
    send_cmd(s_GlobalNoticer, "NOTICE $*.%s :%s", NETWORK_DOMAIN, msg);
    
#else
# if defined(NETWORK_DOMAIN)
    send_cmd(s_GlobalNoticer, "NOTICE $*.%s :%s", NETWORK_DOMAIN, msg);
# else
    /* Go through all common top-level domains.  If you have others,
     * add them here.
     */
     
    send_cmd(s_GlobalNoticer, "NOTICE $*.es :%s", msg);
    send_cmd(s_GlobalNoticer, "NOTICE $*.com :%s", msg);
    send_cmd(s_GlobalNoticer, "NOTICE $*.net :%s", msg);
    send_cmd(s_GlobalNoticer, "NOTICE $*.org :%s", msg);
    send_cmd(s_GlobalNoticer, "NOTICE $*.edu :%s", msg);
    send_cmd(s_GlobalNoticer, "NOTICE $*.eus :%s", msg);
    send_cmd(s_GlobalNoticer, "NOTICE $*.tk :%s", msg);
                
# endif
#endif
    /* canalopers(s_GlobalNoticer, "12%s ha enviado el GLOBALN (%s)", u->nick, msg); */
}


/*************************************************************************/

/* Op en un canal a traves del servidor */

static void do_os_op(User *u)
{
    char *chan = strtok(NULL, " ");
    /*char *op_params = strtok(NULL, " ");*/
     /*char *op_params;*/
    char *argv[3];
     int i = 0;
    Channel *c;
    char *destino;

    if (!chan /*|| !op_params*/) {
        syntax_error(s_OperServ, u, "OP", CHAN_OP_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else {
        /*char *destino;*/
        /*User *u2 =finduser(op_params);*/
         while ((destino = strtok(NULL, " ")) && (i++ < MAXPARAMS)) {
        if (is_on_chan(destino,chan)) { 
		 send_cmd(ServerName, "MODE %s +o %s", chan, destino); 
/*#ifdef IRC_UNDERNET_P10
            destino = u2->numerico;
#else
            destino = u2->nick;
#endif */                                   
           

            argv[0] = sstrdup(chan);
            argv[1] = sstrdup("+o");
            argv[2] = sstrdup(destino);
            do_cmode(s_OperServ, 3, argv);
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
        } else
	   if (is_a_service(destino)) 
    	notice_lang(s_OperServ, u, NICK_IS_A_SERVICE, destino);
           else if (!is_on_chan(destino,chan))
	   notice_lang(s_OperServ, u, NICK_X_NOT_IN_CHAN,destino);
     }
    }
}
/*************************************************************************/



/* deop en un canal a traves de server */

static void do_os_deop(User *u)
{
    char *chan = strtok(NULL, " ");
    /*char *deop_params = strtok(NULL, " ");*/
    char *argv[3];
      int i = 0;
    /*char *deop_params;*/
    char *destino;
    Channel *c;

    if (!chan /*|| !deop_params*/) {
        syntax_error(s_OperServ, u, "DEOP", CHAN_DEOP_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else {
 while ((destino = strtok(NULL, " ")) && (i++ < MAXPARAMS)) {
	     /*User *u2 =finduser(deop_params);*/

        if (is_on_chan(destino,chan)) { 
	    send_cmd(ServerName, "MODE %s -o %s", chan, destino);
/*#ifdef IRC_UNDERNET_P10
            destino = u2->numerico;
#else
            destino = u2->nick;
#endif*/

            argv[0] = sstrdup(chan);
            argv[1] = sstrdup("-o");
            argv[2] = sstrdup(destino);
            do_cmode(s_OperServ, 3, argv);
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
        } else
             if (is_a_service(destino)) 
    	notice_lang(s_OperServ, u, NICK_IS_A_SERVICE, destino);
           else if (!is_on_chan(destino,chan))
	   notice_lang(s_OperServ, u, NICK_X_NOT_IN_CHAN,destino);
    }
}
}

/*************************************************************************/

/* Voz en un canal a traves del servidor */

static void do_os_voice(User *u)
{
    char *chan = strtok(NULL, " ");
    /*char *op_params = strtok(NULL, " ");*/
     /*char *op_params;*/
    char *argv[3];
     int i = 0;
    Channel *c;
    char *destino;

    if (!chan /*|| !op_params*/) {
        syntax_error(s_OperServ, u, "VOICE", CHAN_VOICE_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else {
        /*char *destino;*/
        /*User *u2 =finduser(op_params);*/
         while ((destino = strtok(NULL, " ")) && (i++ < MAXPARAMS)) {
        if (is_on_chan(destino,chan)) { 
		 send_cmd(ServerName, "MODE %s +v %s", chan, destino); 
/*#ifdef IRC_UNDERNET_P10
            destino = u2->numerico;
#else
            destino = u2->nick;
#endif */                                   
           

            argv[0] = sstrdup(chan);
            argv[1] = sstrdup("+v");
            argv[2] = sstrdup(destino);
            do_cmode(s_OperServ, 3, argv);
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
        } else
	   if (is_a_service(destino)) 
    	notice_lang(s_OperServ, u, NICK_IS_A_SERVICE, destino);
           else if (!is_on_chan(destino,chan))
	   notice_lang(s_OperServ, u, NICK_X_NOT_IN_CHAN,destino);
     }
    }
}

/*************************************************************************/

/* Devoice en un canal a traves de server */

static void do_os_devoice(User *u)
{
    char *chan = strtok(NULL, " ");
    /*char *deop_params = strtok(NULL, " ");*/
    char *argv[3];
      int i = 0;
    /*char *deop_params;*/
    char *destino;
    Channel *c;

    if (!chan /*|| !deop_params*/) {
        syntax_error(s_OperServ, u, "DEVOICE", CHAN_DEVOICE_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else {
 while ((destino = strtok(NULL, " ")) && (i++ < MAXPARAMS)) {
	     /*User *u2 =finduser(deop_params);*/

        if (is_on_chan(destino,chan)) { 
	    send_cmd(ServerName, "MODE %s -v %s", chan, destino);
/*#ifdef IRC_UNDERNET_P10
            destino = u2->numerico;
#else
            destino = u2->nick;
#endif*/

            argv[0] = sstrdup(chan);
            argv[1] = sstrdup("-v");
            argv[2] = sstrdup(destino);
            do_cmode(s_OperServ, 3, argv);
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
        } else
             if (is_a_service(destino)) 
    	notice_lang(s_OperServ, u, NICK_IS_A_SERVICE, destino);
           else if (!is_on_chan(destino,chan))
	   notice_lang(s_OperServ, u, NICK_X_NOT_IN_CHAN,destino);
    }
}
}

/*************************************************************************/


/* Channel mode changing (MODE command). */

static void do_os_mode(User *u)
{
    int argc;
    char **argv;
    char *s = strtok(NULL, "");
    char *chan, *modes;
    Channel *c;

    if (!s) {
	syntax_error(s_OperServ, u, "MODE", OPER_MODE_SYNTAX);
	return;
    }
    chan = s;
    s += strcspn(s, " ");
    if (!*s) {
	syntax_error(s_OperServ, u, "MODE", OPER_MODE_SYNTAX);
	return;
    }
    *s = 0;
    modes = (s+1) + strspn(s+1, " ");
    if (!*modes) {
	syntax_error(s_OperServ, u, "MODE", OPER_MODE_SYNTAX);
	return;
    }
    if (!(c = findchan(chan))) {
	notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (c->bouncy_modes) {
#if defined(IRC_DALNET) || defined(IRC_UNDERNET)
	notice_lang(s_OperServ, u, OPER_BOUNCY_MODES_U_LINE);
#else
	notice_lang(s_OperServ, u, OPER_BOUNCY_MODES);
#endif
	return;
    } else {
	send_cmd(ServerName, "MODE %s %s", chan, modes);
	canalopers(s_OperServ, "12%s ha usado MODE %s en 12%s", u->nick, modes, chan); 
	*s = ' ';
	argc = split_buf(chan, &argv, 1);
	do_cmode(s_OperServ, argc, argv);
    }
}

/*************************************************************************/

/* Clear all modes from a channel. */

static void do_clearmodes(User *u)
{
    char *s;
    int i;
    char *argv[3];
    char *chan = strtok(NULL, " ");
    Channel *c;
    int all = 0;
    int count;		/* For saving ban info */
    char **bans;	/* For saving ban info */
    struct c_userlist *cu, *next;

    if (!chan) {
	syntax_error(s_OperServ, u, "CLEARMODES", OPER_CLEARMODES_SYNTAX);
    } else if (!(c = findchan(chan))) {
	notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (c->bouncy_modes) {
#if defined(IRC_DALNET) || defined(IRC_UNDERNET)
	notice_lang(s_OperServ, u, OPER_BOUNCY_MODES_U_LINE);
#else
	notice_lang(s_OperServ, u, OPER_BOUNCY_MODES);
#endif
	return;
    } else {
	s = strtok(NULL, " ");
	if (s) {
	    if (strcmp(s, "ALL") == 0) {
		all = 1;
	    } else {
		syntax_error(s_OperServ,u,"CLEARMODES",OPER_CLEARMODES_SYNTAX);
		return;
	    }
           }
        canalopers(s_OperServ, "12%s ha usado CLEARMODES %s en 12%s",
                      u->nick, all ? " ALL" : "", chan);
                        
	if (all) {
	    /* Clear mode +o */
	    for (cu = c->chanops; cu; cu = next) {
		next = cu->next;
		argv[0] = sstrdup(chan);
		argv[1] = sstrdup("-o");

#if defined(IRC_UNDERNET_P10)
		argv[2] = sstrdup(cu->user->numerico);
#else
                argv[2] = sstrdup(cu->user->nick);
#endif		
		send_cmd(MODE_SENDER(s_ChanServ), "MODE %s %s :%s",
			argv[0], argv[1], argv[2]);
		do_cmode(s_ChanServ, 3, argv);
		free(argv[2]);
		free(argv[1]);
		free(argv[0]);
	    }

	    /* Clear mode +v */
	    for (cu = c->voices; cu; cu = next) {
		next = cu->next;
		argv[0] = sstrdup(chan);
		argv[1] = sstrdup("-v");
#if defined(IRC_UNDERNET_P10)
		argv[2] = sstrdup(cu->user->numerico);
#else
                argv[2] = sstrdup(cu->user->nick);
#endif		
		send_cmd(MODE_SENDER(s_ChanServ), "MODE %s %s :%s",
			argv[0], argv[1], argv[2]);
		do_cmode(s_ChanServ, 3, argv);
		free(argv[2]);
		free(argv[1]);
		free(argv[0]);
	    }
	}

	/* Clear modes */
	send_cmd(MODE_SENDER(s_OperServ), "MODE %s -icklmnpstMR :%s",
		chan, c->key ? c->key : "");
	argv[0] = sstrdup(chan);
	argv[1] = sstrdup("-icklmnpstMR");
	argv[2] = c->key ? c->key : sstrdup("");
	do_cmode(s_OperServ, 2, argv);
	free(argv[0]);
	free(argv[1]);
	free(argv[2]);
	c->key = NULL;
	c->limit = 0;

	/* Clear bans */
	count = c->bancount;
	bans = smalloc(sizeof(char *) * count);
	for (i = 0; i < count; i++)
	    bans[i] = sstrdup(c->bans[i]);
	for (i = 0; i < count; i++) {
	    argv[0] = sstrdup(chan);
	    argv[1] = sstrdup("-b");
	    argv[2] = bans[i];
	    send_cmd(MODE_SENDER(s_OperServ), "MODE %s %s :%s",
			argv[0], argv[1], argv[2]);
	    do_cmode(s_OperServ, 3, argv);
	    free(argv[2]);
	    free(argv[1]);
	    free(argv[0]);
	}
	free(bans);
    }
}

/*************************************************************************/

/* Kick a user from a channel (KICK command). */

static void do_os_kick(User *u)
{
    char *argv[3];
    char *chan, *nick, *s, *destino;
    User *u2;
    Channel *c;

    chan = strtok(NULL, " ");
    nick = strtok(NULL, " ");
    s = strtok(NULL, "");
    if (!chan || !nick || !s) {
	syntax_error(s_OperServ, u, "KICK", OPER_KICK_SYNTAX);
	return;
    }
    if (!(c = findchan(chan))) {
	notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (c->bouncy_modes) {
#if defined(IRC_DALNET) || defined(IRC_UNDERNET)
	notice_lang(s_OperServ, u, OPER_BOUNCY_MODES_U_LINE);
#else
	notice_lang(s_OperServ, u, OPER_BOUNCY_MODES);
#endif
	return;
    }
    u2=finduser(nick);
    
    if (u2) {
#if defined(IRC_UNDERNET_P10)
        destino = u2->numerico;
#else
        destino = u2->nick;
#endif    
    
        send_cmd(ServerName, "KICK %s %s :%s (%s)", chan, destino, u->nick, s);
        canalopers(s_OperServ, "12%s ha usado KICK a 12%s en 12%s", u->nick, nick, chan);

        argv[0] = sstrdup(chan);
        argv[1] = sstrdup(destino);
        argv[2] = sstrdup(s);
        do_kick(s_OperServ, 3, argv);
        free(argv[2]);
        free(argv[1]);
        free(argv[0]);
    } else
        notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);                       
}

/***************************************************************************/

/* Quitar los modos y lo silencia */

static void do_apodera(User *u)
{
    char *chan = strtok(NULL, " ");

    Channel *c;

    if (!chan) {
         privmsg(s_OperServ, u->nick, "Sintaxis: 12APODERA <canal>");
         return;
    } else if (!(c = findchan(chan))) {
         privmsg(s_OperServ, u->nick, "El canal 12%s esta vacio.", chan);
    } else {
         char *av[2];
         struct c_userlist *cu, *next;
#if defined(IRC_UNDERNET_P10)
         send_cmd(s_ChanServ, "J %s", chan);
         send_cmd(ServerName, "M %s +o %s", chan, s_ChanServP10);
         send_cmd(s_ChanServ, "M %s :+tnsim", chan);                           
#else         
         send_cmd(s_ChanServ, "JOIN %s", chan);
         send_cmd(ServerName, "MODE %s +o %s", chan, s_ChanServ);
         send_cmd(s_ChanServ, "MODE %s :+tnsim", chan);
#endif         
         for (cu = c->users; cu; cu = next) {
              next = cu->next;
              av[0] = sstrdup(chan);
#if defined(IRC_UNDERNET_P10)
              av[1] = sstrdup(cu->user->numerico);
#else
              av[1] = sstrdup(cu->user->nick);
#endif              
              send_cmd(MODE_SENDER(s_ChanServ), "MODE %s -o %s",
                     av[0], av[1]);
              do_cmode(s_ChanServ, 3, av);
              free(av[1]);
              free(av[0]);
         }
         canalopers(s_OperServ, "12%s se APODERA de %s", u->nick, chan);
    }
}
                                                   
/**************************************************************************/

static void do_limpia(User *u)
{
    char *chan = strtok(NULL, " ");
    
    Channel *c;
    ChannelInfo *ci;
    if (!chan) {
        privmsg(s_OperServ, u->nick, "Sintaxis: 12LIMPIA <canal>");
        return;
    } else if (!(c = findchan(chan))) {
        privmsg(s_OperServ, u->nick, "El canal 12%s esta vacio.", chan);
    } else if (!(ci = cs_findchan(chan))) {
		notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else {
         if ((ci = cs_findchan(chan)) && (ci->flags & CI_AUTOLIMIT)) {
   privmsg(s_ChanServ, u->nick, "Debes Desactivar el AutoLimit del Canal %s", ci->name);
	return;
        }
        char *av[3];
        struct c_userlist *cu, *next;
        char buf[256];
       
        snprintf(buf, sizeof(buf), "No puedes permanecer en este canal");

#if defined(IRC_UNDERNET_P10)
        send_cmd(s_ChanServ, "J %s", chan);
        send_cmd(ServerName, "M %s +o %s", chan, s_ChanServP10);
        send_cmd(s_ChanServ, "M %s :+tnsim", chan);                       
#else
        send_cmd(s_ChanServ, "JOIN %s", chan);
        send_cmd(ServerName, "MODE %s +o %s", chan, s_ChanServ);
        send_cmd(s_ChanServ, "MODE %s :+tnsim", chan);
#endif        
        for (cu = c->users; cu; cu = next) {
             next = cu->next;
             av[0] = sstrdup(chan);
#if defined(IRC_UNDERNET_P10)
             av[1] = sstrdup(cu->user->numerico);
#else
             av[1] = sstrdup(cu->user->nick);
#endif             
             av[2] = sstrdup(buf);
             send_cmd(MODE_SENDER(s_ChanServ), "KICK %s %s :%s",
                       av[0], av[1], av[2]);
                                          
             do_kick(s_ChanServ, 3, av);
             free(av[2]);
             free(av[1]);
             free(av[0]);
        }
        canalopers(s_OperServ, "12%s ha LIMPIADO 12%s", u->nick, chan);
    }                                                                                                                                
}

static void do_vhost(User *u)
{
    char *nick = strtok(NULL, " ");
    char *mask = strtok(NULL, "");
  
    /*    User *u2 = NULL; */
    NickInfo *ni;
    
    if (!nick) {
    	syntax_error(s_OperServ, u, "VHOST", OPER_VHOST_SYNTAX);
    	return;
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	return;
    } else  if (!(ni->status & NI_ON_BDD)) {
        notice_lang(s_OperServ, u, NICK_MUST_BE_ON_BDD);
	return;
    }
    
    if (!mask) {
       #if defined(IRC_UNDERNET_P09)
    	do_write_bdd(nick, 2, "");
	#endif
	notice_lang(s_OperServ, u, OPER_VHOST_UNSET, nick);
	return;
    }
    if (strlen(mask) > 64) {
	 privmsg(s_OperServ, u->nick, "V-Host demasiado larga. Máximo 64 carácteres.");
	 return;
    }
    
     #if defined(IRC_UNDERNET_P09)
    do_write_bdd(nick, 2, mask);
   #endif
    notice_lang(s_OperServ, u, OPER_VHOST_SET, nick, mask);
}
/**************************************************************************/
/* Kill basico */

static void do_skill(User *u)
{
    char *nick = strtok(NULL, " ");
    char *text = strtok(NULL, "");
    User *u2 = NULL;
    
    
    
    if (!nick) {
        return;
    }
    
    u2 = finduser(nick);

    if (!u2) {
        notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
    } else if (is_a_service(nick)) {
        notice_lang(s_OperServ, u, PERMISSION_DENIED);
    } else if (!text) {
        send_cmd(ServerName, "KILL %s :Have a nice day!", nick);
	return;
    } else {
   	send_cmd(ServerName, "KILL %s :%s", nick, text);
    }
	canalopers(s_OperServ, "12%s utilizó KILL sobre 12%s", u->nick, nick);
}
/**************************************************************************/

/* Gline de 5 minutos */

static void do_block(User *u)
{
    char *nick = strtok(NULL, " ");
    char *text = strtok(NULL, "");
    User *u2 = NULL; 
    
    if (!text) {
        privmsg(s_OperServ, u->nick, "Sintaxis: 12BLOCK <nick> <motivo>");    
        return;
    }         
    
    u2 = finduser(nick);   
        
    if (!u2) {
        notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, nick);
    } else {
    
        send_cmd(ServerName, "GLINE * +*@%s 300 :%s", u2->host, text);
 
        privmsg(s_OperServ, u->nick, "Has cepillado a 12%s", nick);
        canalopers(s_OperServ, "12%s ha cepillado a 12%s (%s)", u->nick, nick, text);
    }
                                          
}


/**************************************************************************/

/* Quitar un block o gline */

static void do_unblock(User *u)
{
    char *mascara = strtok(NULL, " ");
    
    if (!mascara) {
#if defined(IRC_UNDERNET_P10)
        privmsg(s_OperServ, u->numerico, "Sintaxis: 12UNBLOCK/UNGLINE <*@host.es>");
#else
        privmsg(s_OperServ, u->nick, "Sintaxis: 12UNBLOCK/UNGLINE <*@host.es>");
#endif
    } else {
        send_cmd(ServerName ,"GLINE * -%s", mascara);
        canalopers(s_OperServ, "12%s ha usado UNBLOCK/UNGLINE en 12%s", u->nick, mascara);
        if (del_akill(mascara))           	                  
		notice_lang(s_OperServ, u, OPER_AKILL_REMOVED, mascara);
    }
}
                                        
/*************************************************************************/

/* Sincronizar la red en tiempo real */

static void do_settime(User *u)
{
    time_t tiempo = time(NULL);

    send_cmd(NULL, "SETTIME %lu", tiempo);
/*#if HAVE_ALLWILD_NOTICE
    send_cmd(s_OperServ, "NOTICE $*.%s :Sincronizando la RED...", NETWORK_DOMAIN);
    
#else
# ifdef NETWORK_DOMAIN
    send_cmd(s_OperServ, "NOTICE $*.%s :Sincronizando la RED...", NETWORK_DOMAIN);
# else
     * Go through all common top-level domains.  If you have others,
     * add them here.
     
    send_cmd(s_OperServ, "NOTICE $*.es :Sincronizando la RED...");
    send_cmd(s_OperServ, "NOTICE $*.com :Sincronizando la RED...");
    send_cmd(s_OperServ, "NOTICE $*.net :Sincronizando la RED...");
    send_cmd(s_OperServ, "NOTICE $*.org :Sincronizando la RED...");
    send_cmd(s_OperServ, "NOTICE $*.edu :Sincronizando la RED...");
# endif
#endif                                          
    canalopers(s_OperServ, "12%s ha usado SETTIME", u->nick); */
}
            
/*************************************************************************/

/* ROOT list viewing(solo listamos) porque estan dados de alta por services.conf*/

static void do_director(User *u)
{
    char *cmd/*, *nick*/;
    /*NickInfo *ni;*/
    int i/*,gid*/;

    /*if (skeleton) {
	notice_lang(s_OperServ, u, OPER_ADMIN_SKELETON);
	return;
    }*/
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    

    if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_OperServ, u, OPER_DIRECTOR_LIST_HEADER,Net);
	  for (i = 0; i <RootNumber ; i++) {
     privmsg(s_OperServ, u->nick, "%s",ServicesRoots[i]);
     }

    } else {
	syntax_error(s_OperServ, u, "DIRECTOR",OPER_DIRECTOR_SYNTAX);
    }
}


/*************************************************************************/

/* Services admin list viewing/modification. */

static void do_admin(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i/*,gid*/;

    if (skeleton) {
	notice_lang(s_OperServ, u, OPER_ADMIN_SKELETON);
	return;
    }
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if (!is_services_root(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_OperServ, u->nick, "No pueden añadirse nicks que no esten migrados a la BDD");
	       return;
            }
	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (!services_admins[i] || services_admins[i] == ni)
		    break;
	    }
	    if (services_admins[i] == ni) {
		notice_lang(s_OperServ, u, OPER_ADMIN_EXISTS, ni->nick);
	    } else if (i < MAX_SERVADMINS) {
		services_admins[i] = ni;
		notice_lang(s_OperServ, u, OPER_ADMIN_ADDED, ni->nick);
		canaladmins(s_OperServ, "12%s añade a 12%s como ADMIN", u->nick, ni->nick);
		#if defined(IRC_PATCHS_P09)
		if (nick_is_services_root(ni)) 
		do_write_bdd(ni->nick, 3, "khaAX");
		else
	    	do_write_bdd(ni->nick, 3, "khaX");
	    	#else
		do_write_bdd(ni->nick, 3, "10");
	    	#endif
            if (nick_is_services_root(ni)) {
		do_write_bdd(ni->nick, 27, "");
                char root[BUFSIZE];
                snprintf(root, sizeof(root), "%s%s.%s", RootColor,ni->nick,RootHost);
                ni->vhost=sstrdup(root);
               } else { do_write_bdd(ni->nick, 23, "");
			 char adm[BUFSIZE];
                         snprintf(adm, sizeof(adm), "%s%s.%s", AdminColor,ni->nick,AdminHost);
                         ni->vhost=sstrdup(adm);
                 }
		#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
if (nick_is_services_root(ni)) {
gid = 25;
} else {
  gid = 24;
  }

snprintf(modifica, sizeof(modifica), "update jos_users set gid='%d' where username ='%s';",gid,ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='%d' where aro_id  ='%s';",gid,row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));

if (nick_is_services_root(ni)) {
canaladmins(s_OperServ, "Añadido como 10Super-Administrador de la web",ni->nick);
} else {
canaladmins(s_OperServ, "Añadido como 10Administrador de la web",ni->nick);
  } 
 mysql_close(conn);

     #endif
send_cmd(NULL, "RENAME %s", ni->nick);
	    } else {
		notice_lang(s_OperServ, u, OPER_ADMIN_TOO_MANY, MAX_SERVADMINS);
	    }
	    if (readonly)
		notice_lang(s_OperServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_OperServ, u, "ADMIN", OPER_ADMIN_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_services_root(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	   	   	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (services_admins[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVADMINS) {
		services_admins[i] = NULL;
		notice_lang(s_OperServ, u, OPER_ADMIN_REMOVED, ni->nick);
		canaladmins(s_OperServ, "12%s borra a 12%s como ADMIN", u->nick, ni->nick);
		#if defined(IRC_UNDERNET_P09)
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
                ni->vhost=NULL;
		#endif
			#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='18' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='18' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Borrado como 10Administrador de la web",ni->nick);
 mysql_close(conn);

     #endif
		if (readonly)
		    notice_lang(s_OperServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_OperServ, u, OPER_ADMIN_NOT_FOUND, ni->nick);
	    }
	} else {
	    syntax_error(s_OperServ, u, "ADMIN", OPER_ADMIN_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_OperServ, u, OPER_ADMIN_LIST_HEADER);
	for (i = 0; i < MAX_SERVADMINS; i++) {
	    if (services_admins[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_OperServ, u->numerico, "%s", services_admins[i]->nick);
#else	    
		privmsg(s_OperServ, u->nick, "%s", services_admins[i]->nick);
#endif
	}

    } else {
	syntax_error(s_OperServ, u, "ADMIN", OPER_ADMIN_SYNTAX);
    }
}

/*************************************************************************/

/* Services cregadmin-coadmin list viewing/modification. */

static void do_coadmin(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i;

    if (skeleton) {
	notice_lang(s_OperServ, u, OPER_ADMIN_SKELETON, "COADMIN");
	return;
    }
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if (!is_services_root(u) && !is_services_admin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_OperServ, u->nick, "No pueden añadirse nicks que no esten migrados a la BDD");
	       return;
            }
	    
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (!services_cregadmins[i] || services_cregadmins[i] == ni)
		    break;
	    }
	    if (services_cregadmins[i] == ni) {
		notice_lang(s_OperServ, u, OPER_CREGADMIN_EXISTS, ni->nick, "CoAdmins");
	    } else if (i < MAX_SERVADMINS) {
		services_cregadmins[i] = ni;
		notice_lang(s_OperServ, u, OPER_CREGADMIN_ADDED, ni->nick, "CoAdmins");
		canaladmins(s_OperServ, "12%s añade a 12%s como COADMIN", u->nick, ni->nick);
		#if defined(IRC_PATCHS_P09)
		do_write_bdd(ni->nick, 3, "khcX"); //-->si lo añado a la tabla o y 10 para flag X,"en este parche es modo +c"
		#else 
		do_write_bdd(ni->nick, 3, "10");
		#endif
		do_write_bdd(ni->nick, 26, "");
                         char coadm[BUFSIZE];
                         snprintf(coadm, sizeof(coadm), "%s%s.%s", CoAdminColor,ni->nick,CoAdminHost);
                         ni->vhost=sstrdup(coadm);
		   	send_cmd(NULL, "RENAME %s", ni->nick);
			#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='23' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='23' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Añadido como 10Gestor-Mánager de la web",ni->nick);
 mysql_close(conn);

     #endif
	    } else {
		notice_lang(s_OperServ, u, OPER_CREGADMIN_TOO_MANY, MAX_SERVADMINS, "CoAdmins");
	    }
	    if (readonly)
		notice_lang(s_OperServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_OperServ, u, "COADMIN", OPER_CREGADMIN_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_services_root(u) && !is_services_admin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	    for (i = 0; i < MAX_SERVADMINS; i++) {
		if (services_cregadmins[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVADMINS) {
		services_cregadmins[i] = NULL;
		notice_lang(s_OperServ, u, OPER_CREGADMIN_REMOVED, ni->nick, "CoAdmins");
		canaladmins(s_OperServ, "12%s borra a 12%s como COADMIN", u->nick, ni->nick);
		#if defined(IRC_UNDERNET_P09)
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		ni->vhost=NULL;
		send_cmd(NULL, "RENAME %s", ni->nick);
		#endif
			#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='18' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='18' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Borrado como 10Gestor-Mánager de la web",ni->nick);
 mysql_close(conn);

     #endif
		if (readonly)
		    notice_lang(s_OperServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_OperServ, u, OPER_CREGADMIN_NOT_FOUND, ni->nick, "CoAdmins");
	    }
	} else {
	    syntax_error(s_OperServ, u, "COADMIN", OPER_CREGADMIN_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_OperServ, u, OPER_CREGADMIN_LIST_HEADER, "COAdmins");
	for (i = 0; i < MAX_SERVADMINS; i++) {
	    if (services_cregadmins[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_OperServ, u->numerico, "%s", services_cregadmins[i]->nick);
#else	    
		privmsg(s_OperServ, u->nick, "%s", services_cregadmins[i]->nick);
#endif
	}

    } else {
	syntax_error(s_OperServ, u, "COADMIN", OPER_CREGADMIN_SYNTAX);
    }
}

/*************************************************************************/

/* Services admin list viewing/modification. */

static void do_devel(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i;

    if (skeleton) {
	notice_lang(s_OperServ, u, OPER_ADMIN_SKELETON);
	return;
    }
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if  (!is_services_root(u) && !is_services_admin(u) && !is_services_cregadmin(u)) { 
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_OperServ, u->nick, "No pueden añadirse nicks que no esten migrados a la BDD");
	       return;
            }
	    
	    for (i = 0; i < MAX_SERVDEVELS; i++) {
		if (!services_devels[i] || services_devels[i] == ni)
		    break;
	    }
	    if (services_devels[i] == ni) {
		notice_lang(s_OperServ, u, OPER_DEVEL_EXISTS, ni->nick);
	    } else if (i < MAX_SERVDEVELS) {
		services_devels[i] = ni;
		notice_lang(s_OperServ, u, OPER_DEVEL_ADDED, ni->nick);
		canaladmins(s_OperServ, "12%s añade a 12%s como DEVEL", u->nick, ni->nick);
		#if defined(IRC_PATCHS_P09)
	        do_write_bdd(ni->nick, 3, "khDX");
	    	#else
		do_write_bdd(ni->nick, 3, "10");
	    	#endif
		do_write_bdd(ni->nick, 24, "");
		         char dev[BUFSIZE];
                         snprintf(dev, sizeof(dev), "%s%s.%s", DevelColor,ni->nick,DevelHost);
                         ni->vhost=sstrdup(dev);
	                 send_cmd(NULL, "RENAME %s", ni->nick);
			#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='21' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='21' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Añadido como 10Supervisor-Publisher de la web",ni->nick);
 mysql_close(conn);

     #endif
	    } else {
		notice_lang(s_OperServ, u, OPER_DEVEL_TOO_MANY, MAX_SERVDEVELS);
	    }
	    if (readonly)
		notice_lang(s_OperServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_OperServ, u, "DEVEL", OPER_DEVEL_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_services_root(u) && !is_services_admin(u) && !is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	    
	   
	    
	    for (i = 0; i < MAX_SERVDEVELS; i++) {
		if (services_devels[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVDEVELS) {
		services_devels[i] = NULL;
		notice_lang(s_OperServ, u, OPER_DEVEL_REMOVED, ni->nick);
		canaladmins(s_OperServ, "12%s borra a 12%s como DEVEL", u->nick, ni->nick);
		#if defined(IRC_UNDERNET_P09)
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		ni->vhost=NULL;
		send_cmd(NULL, "RENAME %s", ni->nick);
		#endif
		#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='18' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='18' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Borrado como 10Supervisor-Publisher de la web",ni->nick);
 mysql_close(conn);

     #endif
		if (readonly)
		    notice_lang(s_OperServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_OperServ, u, OPER_DEVEL_NOT_FOUND, ni->nick);
	    }
	} else {
	    syntax_error(s_OperServ, u, "DEVEL", OPER_DEVEL_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_OperServ, u, OPER_DEVEL_LIST_HEADER);
	for (i = 0; i < MAX_SERVDEVELS; i++) {
	    if (services_devels[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_OperServ, u->numerico, "%s", services_devels[i]->nick);
#else	    
		privmsg(s_OperServ, u->nick, "%s", services_devels[i]->nick);
#endif
	}

    } else {
	syntax_error(s_OperServ, u, "DEVEL", OPER_DEVEL_SYNTAX);
    }
}





/*************************************************************************/

/*************************************************************************/

/* Services oper list viewing/modification. */

static void do_oper(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i;

    if (skeleton) {
	notice_lang(s_OperServ, u, OPER_OPER_SKELETON);
	return;
    }
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if  (!is_services_root(u) && !is_services_admin(u) && !is_services_cregadmin(u) && !is_services_devel(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    
	    if (!(ni->status & NI_ON_BDD)) {
	        notice_lang(s_OperServ, u, NICK_MUST_BE_ON_BDD);
	        return;
            }
	    
	    for (i = 0; i < MAX_SERVOPERS; i++) {
		if (!services_opers[i] || services_opers[i] == ni)
		    break;
	    }
	    if (services_opers[i] == ni) {
		notice_lang(s_OperServ, u, OPER_OPER_EXISTS, ni->nick);
	    } else if (i < MAX_SERVOPERS) {
		services_opers[i] = ni;
		notice_lang(s_OperServ, u, OPER_OPER_ADDED, ni->nick);
		canaladmins(s_OperServ, "12%s añade a 12%s como OPER", u->nick, ni->nick);
		#if defined(IRC_PATCHS_P09)
		do_write_bdd(ni->nick, 3, "kh");
		#else
	    	do_write_bdd(ni->nick, 3, "5");
		#endif
		do_write_bdd(ni->nick, 22, "");
                         char op[BUFSIZE];
                         snprintf(op, sizeof(op), "%s%s.%s", OperColor,ni->nick,OperHost);
                         ni->vhost=sstrdup(op);
			send_cmd(NULL, "RENAME %s", ni->nick);
		#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='20' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='20' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Añadido como 10Editor de la web",ni->nick);
 mysql_close(conn);

     #endif
	    } else {
		notice_lang(s_OperServ, u, OPER_OPER_TOO_MANY, MAX_SERVOPERS);
	    }
	    if (readonly)
		notice_lang(s_OperServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_OperServ, u, "OPER", OPER_OPER_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_services_root(u) && !is_services_admin(u) && !is_services_cregadmin(u) && !is_services_devel(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	    for (i = 0; i < MAX_SERVOPERS; i++) {
		if (services_opers[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVOPERS) {
		services_opers[i] = NULL;
		notice_lang(s_OperServ, u, OPER_OPER_REMOVED, ni->nick);
		canaladmins(s_OperServ, "12%s borra a 12%s como OPER", u->nick, ni->nick);
		#if defined(IRC_UNDERNET_P09)
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		ni->vhost=NULL;
		send_cmd(NULL, "RENAME %s", ni->nick);	
		#endif	
#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='18' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='18' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Borrado como 10Editor de la web",ni->nick);
 mysql_close(conn);

     #endif
		if (readonly)
		    notice_lang(s_OperServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_OperServ, u, OPER_OPER_NOT_FOUND, ni->nick);
	    }
	} else {
	    syntax_error(s_OperServ, u, "OPER", OPER_OPER_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_OperServ, u, OPER_OPER_LIST_HEADER);
	for (i = 0; i < MAX_SERVOPERS; i++) {
	    if (services_opers[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_OperServ, u->numerico, "%s", services_opers[i]->nick);
#else	    
		privmsg(s_OperServ, u->nick, "%s", services_opers[i]->nick);
#endif
	}

    } else {
	syntax_error(s_OperServ, u, "OPER", OPER_OPER_SYNTAX);
    }
}

/*************************************************************************/
static void do_patrocina(User *u)
{
    char *cmd, *nick;
    NickInfo *ni;
    int i;

    if (skeleton) {
	notice_lang(s_OperServ, u, OPER_OPER_SKELETON);
	return;
    }
    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if  (!is_services_root(u) && !is_services_admin(u) && !is_services_cregadmin(u) && !is_services_devel(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    } 
	    
	    if (!(ni->status & NI_ON_BDD)) {
	        notice_lang(s_OperServ, u, NICK_MUST_BE_ON_BDD);
	        return;
            }
	    
	    for (i = 0; i < MAX_SERVPATROCINAS; i++) {
		if (!services_opers[i] || services_opers[i] == ni)
		    break;
	    }
	    if (services_patrocinas[i] == ni) {
		notice_lang(s_OperServ, u, OPER_PATROCINA_EXISTS, ni->nick);
	    } else if (i < MAX_SERVPATROCINAS) {
		services_patrocinas[i] = ni;
		notice_lang(s_OperServ, u, OPER_PATROCINA_ADDED, ni->nick);
		canaladmins(s_OperServ, "12%s añade a 12%s como PATROCINADOR", u->nick, ni->nick);
                #if defined(IRC_PATCHS_P09)
		do_write_bdd(ni->nick, 3, "kp"); /*-->En este parche,es modo +p de Patrocinador*/
		#else
	    	do_write_bdd(ni->nick, 3, ""); /*-->No lo añado a la tabla o*/
		#endif
		do_write_bdd(ni->nick, 25, "");
                         char pat[BUFSIZE];
                         snprintf(pat, sizeof(pat), "%s%s.%s", PatrocinaColor,ni->nick,PatrocinaHost);
                         ni->vhost=sstrdup(pat);
		send_cmd(NULL, "RENAME %s", ni->nick);
		#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='19' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='19' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Añadido como 10Autor de la web",ni->nick);
 mysql_close(conn);

     #endif
	    } else {
		notice_lang(s_OperServ, u, OPER_PATROCINA_TOO_MANY, MAX_SERVOPERS);
	    }
	    if (readonly)
		notice_lang(s_OperServ, u, READ_ONLY_MODE);
	} else {
	    syntax_error(s_OperServ, u, "PATROCINA", OPER_PATROCINA_ADD_SYNTAX);
	}

    } else if (stricmp(cmd, "DEL") == 0) {
	if (!is_services_root(u) && !is_services_admin(u) && !is_services_cregadmin(u) && !is_services_devel(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}
	nick = strtok(NULL, " ");
	if (nick) {
	    if (!(ni = findnick(nick))) {
		notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
		return;
	    }
	    for (i = 0; i < MAX_SERVPATROCINAS; i++) {
		if (services_patrocinas[i] == ni)
		    break;
	    }
	    if (i < MAX_SERVPATROCINAS) {
		services_patrocinas[i] = NULL;
		notice_lang(s_OperServ, u, OPER_PATROCINA_REMOVED, ni->nick);
		canaladmins(s_OperServ, "12%s borra a 12%s como PATROCINADOR", u->nick, ni->nick);
		#if defined(IRC_UNDERNET_P09)
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		ni->vhost=NULL;
		send_cmd(NULL, "RENAME %s", ni->nick);	
		#endif	
	#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
 MYSQL_RES *res;
 MYSQL_ROW row;
char modifica[BUFSIZE],consulta[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set gid='18' where username ='%s';",ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
snprintf(consulta, sizeof(consulta), "select id,gid from jos_users where username ='%s' limit 1",ni->nick);
if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));
	         res = mysql_use_result(conn);
                 row = mysql_fetch_row(res);			
//canaladmins(s_OperServ, "id es %s y gid es %s",row[0],row[1]);   
snprintf(consulta, sizeof(consulta), "select id  from  jos_core_acl_aro where value ='%s' limit 1",row[0]);
 mysql_free_result(res);
    if (mysql_query(conn,consulta)) 
      	canaladmins(s_NickServ, "%s\n", mysql_error(conn));   
 res = mysql_use_result(conn);
 row = mysql_fetch_row(res);	
//canaladmins(s_OperServ, "el valor es %s",row[0]);
snprintf(modifica, sizeof(modifica), "update jos_core_acl_groups_aro_map set group_id ='18' where aro_id  ='%s';",row[0]); // es el  aro_id 
 mysql_free_result(res);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
          
canaladmins(s_OperServ, "Borrado como 10Autor de la web",ni->nick);
 mysql_close(conn);

     #endif
	
		if (readonly)
		    notice_lang(s_OperServ, u, READ_ONLY_MODE);
	    } else {
		notice_lang(s_OperServ, u, OPER_PATROCINA_NOT_FOUND, ni->nick);
	    }
	} else {
	    syntax_error(s_OperServ, u, "PATROCINA", OPER_PATROCINA_DEL_SYNTAX);
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_OperServ, u, OPER_PATROCINA_LIST_HEADER);
	for (i = 0; i < MAX_SERVPATROCINAS; i++) {
	    if (services_patrocinas[i])
#if defined(IRC_UNDERNET_P10)
                privmsg(s_OperServ, u->numerico, "%s", services_patrocinas[i]->nick);
#else	    
		privmsg(s_OperServ, u->nick, "%s", services_patrocinas[i]->nick);
#endif
	}

    } else {
	syntax_error(s_OperServ, u, "PATROCINA", OPER_PATROCINA_SYNTAX);
    }
}

/*************************************************************************/

/*************************************************************************/

/* Set various Services runtime options. */

static void do_set(User *u)
{
    char *option = strtok(NULL, " ");
    char *setting = strtok(NULL, " ");

    if (!option || !setting) {
	syntax_error(s_OperServ, u, "SET", OPER_SET_SYNTAX);

    } else if (stricmp(option, "IGNORE") == 0) {
	if (stricmp(setting, "on") == 0) {
	    allow_ignore = 1;
	    notice_lang(s_OperServ, u, OPER_SET_IGNORE_ON);
	} else if (stricmp(setting, "off") == 0) {
	    allow_ignore = 0;
	    notice_lang(s_OperServ, u, OPER_SET_IGNORE_OFF);
	} else if (stricmp(setting, "status") == 0) {
	    if (allow_ignore) 
             privmsg(s_OperServ, u->nick, "Protección Flood Servicios 2Activado");
         else
               privmsg(s_OperServ, u->nick, "Protección Flood Servicios 5DESACTIVADO");
	return;
	} else {
	    notice_lang(s_OperServ, u, OPER_SET_IGNORE_ERROR);
	}
  } else if (stricmp(option, "DESNOTIFY") == 0) {
	if (stricmp(setting, "on") == 0) {
	    notifinouts = 1;
	    notice_lang(s_OperServ, u, X_SET_NOTIFY_ON);
	} else if (stricmp(setting, "off") == 0) {
	   notifinouts = 0;
	    notice_lang(s_OperServ, u, X_SET_NOTIFY_OFF);
	} else if (stricmp(setting, "status") == 0) {
	    if (notifinouts) 
	      privmsg(s_OperServ, u->nick, "Notificación De Servicios 5DESACTIVADO");
             
         else
               privmsg(s_OperServ, u->nick, "Notificación De Servicios 2Activado");
	return;
	} else {
	    notice_lang(s_OperServ, u, X_SET_NOTIFY_ERROR);
	}



    } else if (stricmp(option, "READONLY") == 0) {
	if (stricmp(setting, "on") == 0) {
	    readonly = 1;
	    logeo("Read-only mode activated");
	    close_log();
	    notice_lang(s_OperServ, u, OPER_SET_READONLY_ON);
	} else if (stricmp(setting, "off") == 0) {
	    readonly = 0;
	    open_log();
	    logeo("Read-only mode deactivated");
	    notice_lang(s_OperServ, u, OPER_SET_READONLY_OFF);
	} else {
	    notice_lang(s_OperServ, u, OPER_SET_READONLY_ERROR);
	}

    } else if (stricmp(option, "DEBUG") == 0) {
	if (stricmp(setting, "on") == 0) {
	    debug = 1;
	    logeo("Debug mode activated");
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_ON);
	} else if (stricmp(setting, "off") == 0 ||
				(*setting == '0' && atoi(setting) == 0)) {
	    logeo("Debug mode deactivated");
	    debug = 0;
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_OFF);
	} else if (isdigit(*setting) && atoi(setting) > 0) {
	    debug = atoi(setting);
	    logeo("Debug mode activated (level %d)", debug);
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_LEVEL, debug);
	} else {
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_ERROR);
	}

    } else {
	notice_lang(s_OperServ, u, OPER_SET_UNKNOWN_OPTION, option);
    }
}

/*************************************************************************/

static void do_jupe(User *u)
{
    char *jserver = strtok(NULL, " ");
    char *reason = strtok(NULL, "");
    char buf[NICKMAX+16];
    Server *server;

#if defined(IRC_UNDERNET_P10)
    char *destino=u->numerico;
#else
    char *destino=u->nick;
#endif

    if (!jserver) 
	syntax_error(s_OperServ, u, "JUPE", OPER_JUPE_SYNTAX);
    else 
	if (!reason) {
	    snprintf(buf, sizeof(buf), "Jupeado por %s", u->nick);
	    reason = buf;
	}	
    server = find_servername(jserver);
    
    if (!server) {
        privmsg(s_OperServ, destino, "No encuentro el server 12%s para JUPEarlo", jserver);
        return;
    } else {
                
#if defined(IRC_UNDERNET_P09)
        send_cmd(NULL, "SQUIT %s 0 :%s", jserver, reason);
        send_cmd(NULL, "SERVER %s 2 %lu %lu P10 :%s",
                   jserver, time(NULL), time(NULL), reason);               
        /*canalopers(s_OperServ, "argv[0] ya! y completado!");  */
     
#elif defined (IRC_UNDERNET_P10)
        send_cmd(NULL, "%c SQ %s 0 :%s", convert2y[ServerNumerico], jserver, reason);
        send_cmd(NULL, "%c S %s 2 %lu %lu J10 %s 0 :%s",
            convert2y[ServerNumerico], jserver, time(NULL), time(NULL), server->numerico, reason);                                
#else
	send_cmd(NULL, "SERVER %s 2 :%s", jserver, reason);                        	
#endif
        privmsg(s_OperServ, destino, "Has JUPEado el servidor 12%s", jserver);
        canalopers(s_OperServ, "12JUPE  de %s por 12%s.",
                               jserver, u->nick);        
        logeo("%s: %s!%s@%s ha JUPEado a %s (%s)", s_OperServ, u->nick, u->realname, u->host,
            jserver, reason);
                                
    }
}


static void do_raw(User *u)
{
    char *text = strtok(NULL, "");

    if (!text) {
	syntax_error(s_OperServ, u, "RAW", OPER_RAW_SYNTAX);
    } else {
 	send_cmd(NULL, "%s", text);
/*	canaladmins(s_OperServ, "12%s ha usado 12RAW para: %s.", 
	  u->nick, text);*/
    }
}

/*************************************************************************/

static void do_update(User *u)
{
    notice_lang(s_OperServ, u, OPER_UPDATING);
    save_data = 1;
    canaladmins(s_OperServ, "12%s ejecuta UPDATE. 4Actualizando bases de datos..", u->nick);
}

/*************************************************************************/

static void do_os_quit(User *u)
{
    quitmsg = malloc(50 + strlen(u->nick));
    if (!quitmsg)
	quitmsg = "Aieee! QUITing Services...!";
    else
	sprintf(quitmsg, "Aieee! Services ha recibido una orden de QUIT de %s", u->nick);
/*    canaladmins(s_OperServ, "%s", quitmsg);*/
    quitting = 1;
}

/*************************************************************************/

static void do_shutdown(User *u)
{
    quitmsg = malloc(54 + strlen(u->nick));
    if (!quitmsg)
    
	quitmsg = "Cerrando Services...!";
    else
	sprintf(quitmsg, "Services ha recibido una orden de SHUTDOWN de %s", u->nick);
/*   canaladmins(s_OperServ, "%s", quitmsg);*/
    save_data = 1;
    delayed_quit = 1;
}
static void do_reload(User *u)
{

if (!read_config()) {
	canaladmins(s_OperServ, "4Error Recargando el %s",SERVICES_CONF);
         } 
    else {
	canaladmins(s_OperServ, "4Reloading %s12",SERVICES_CONF);
       }

 privmsg(s_OperServ, u->nick, "12Reloading Hecho");

/* uso stricmp cuando no importa case sensitive*/

if (!RootHostold)
RootHostold ="";
if (strcmp(RootHost, RootHostold) != 0) {
 canaladmins(s_OperServ, "12RootHost Cambiando 4%s a 2%s",RootHostold,RootHost);
RootHostold =RootHost;
}

if (!AdminHostold)
AdminHostold ="";
if (strcmp(AdminHost, AdminHostold) != 0) {
 canaladmins(s_OperServ, "12AdminHost Cambiando 4%s a 2%s",AdminHostold,AdminHost);
AdminHostold =AdminHost;
}
if (!CoAdminHostold)
CoAdminHostold ="";
if (strcmp(CoAdminHost,CoAdminHostold) != 0) {
 canaladmins(s_OperServ, "12CoAdminHost Cambiando 4%s a 2%s",CoAdminHostold,CoAdminHost);
CoAdminHostold =CoAdminHost;
}
if (!DevelHostold)
DevelHostold ="";
if (strcmp(DevelHost, DevelHostold) != 0) {
 canaladmins(s_OperServ, "12DevelHost Cambiando 4%s a 2%s",DevelHostold,DevelHost);
DevelHostold =DevelHost;
}
if (!OperHostold)
OperHostold ="";
if (strcmp(OperHost, OperHostold) != 0) {
 canaladmins(s_OperServ, "12OperHost Cambiando 4%s a 2%s",OperHostold,OperHost);
OperHostold =OperHost;
}
if (!PatrocinaHostold)
PatrocinaHostold ="";
if (strcmp(PatrocinaHost, PatrocinaHostold) != 0) {
 canaladmins(s_OperServ, "12PatrocinaHost Cambiando 4%s a 2%s",PatrocinaHostold,PatrocinaHost);
PatrocinaHostold =PatrocinaHost;
}
if (!s_NickServold)
s_NickServold ="";
if (strcmp(s_NickServ, s_NickServold) != 0) {
 canaladmins(s_OperServ, "12s_NickServ Cambiando 4%s a 2%s",s_NickServold,s_NickServ);
send_cmd(NULL, ":%s NICK %s %lu", s_NickServold,s_NickServ,time(NULL));
s_NickServold =s_NickServ;
}
if (!s_ChanServold)
s_ChanServold ="";
if (strcmp(s_ChanServ, s_ChanServold) != 0) {
 canaladmins(s_OperServ, "12s_ChanServ Cambiando 4%s a 2%s",s_ChanServold,s_ChanServ);
send_cmd(NULL, ":%s NICK %s %lu", s_ChanServold,s_ChanServ,time(NULL));
s_ChanServold =s_ChanServ;
}
if (!s_MemoServold)
s_MemoServold ="";
if (strcmp(s_MemoServ, s_MemoServold) != 0) {
 canaladmins(s_OperServ, "12s_MemoServ Cambiando 4%s a 2%s",s_MemoServold,s_MemoServ);
send_cmd(NULL, ":%s NICK %s %lu", s_MemoServold,s_MemoServ,time(NULL));
s_MemoServold =s_MemoServ;
}
if (!s_HelpServold)
s_HelpServold ="";
if (strcmp(s_HelpServ, s_HelpServold) != 0) {
 canaladmins(s_OperServ, "12s_HelpServ Cambiando 4%s a 2%s",s_HelpServold,s_HelpServ);
send_cmd(NULL, ":%s NICK %s %lu", s_HelpServold,s_HelpServ,time(NULL));
s_MemoServold =s_MemoServ;
}
if (!s_OperServold)
s_OperServold ="";
if (strcmp(s_OperServ, s_OperServold) != 0) {
 canaladmins(s_OperServ, "12s_OperServ Cambiando 4%s a 2%s",s_OperServold,s_OperServ);
send_cmd(NULL, ":%s NICK %s %lu",s_OperServold,s_OperServ,time(NULL));
s_OperServold =s_OperServ;
}
if (!s_CregServold)
s_CregServold ="";
if (strcmp(s_CregServ, s_CregServold) != 0) {
 canaladmins(s_OperServ, "12s_CregServ Cambiando 4%s a 2%s",s_CregServold,s_CregServ);
send_cmd(NULL, ":%s NICK %s %lu",s_CregServold,s_CregServ,time(NULL));
s_CregServold =s_CregServ;
}
if (!s_SpamServold)
s_SpamServold ="";
if (strcmp(s_SpamServ, s_SpamServold) != 0) {
 canaladmins(s_OperServ, "12s_SpamServ Cambiando 4%s a 2%s",s_SpamServold,s_SpamServ);
send_cmd(NULL, ":%s NICK %s %lu",s_SpamServold,s_SpamServ,time(NULL));
s_SpamServold =s_SpamServ;
}
if (!s_StatServold)
s_StatServold ="";
if (strcmp(s_StatServ, s_StatServold) != 0) {
 canaladmins(s_OperServ, "12s_StatServ Cambiando 4%s a 2%s",s_StatServold,s_StatServ);
send_cmd(NULL, ":%s NICK %s %lu", s_StatServold,s_StatServ,time(NULL));
s_StatServold =s_StatServ;
}
if (!s_EuskalIRCServold)
s_EuskalIRCServold ="";
if (strcmp(s_EuskalIRCServ, s_EuskalIRCServold) != 0) {
 canaladmins(s_OperServ, "12s_EuskalIRCServ Cambiando 4%s a 2%s",s_EuskalIRCServold,s_EuskalIRCServ);
send_cmd(NULL, ":%s NICK %s %lu",s_EuskalIRCServold,s_EuskalIRCServ,time(NULL));
s_EuskalIRCServold =s_EuskalIRCServ;
}
if (!s_GlobalNoticerold)
s_GlobalNoticerold ="";
if (strcmp(s_GlobalNoticer, s_GlobalNoticerold) != 0) {
 canaladmins(s_OperServ, "12s_GlobalNoticer Cambiando 4%s a 2%s",s_GlobalNoticerold,s_GlobalNoticer);
send_cmd(NULL, ":%s NICK %s %lu",s_GlobalNoticerold,s_GlobalNoticer,time(NULL));
s_GlobalNoticerold =s_GlobalNoticer;
}
if (!s_NewsServold)
s_NewsServold ="";
if (strcmp(s_NewsServ, s_NewsServold) != 0) {
 canaladmins(s_OperServ, "12s_NewsServ Cambiando 4%s a 2%s",s_NewsServold,s_NewsServ);
send_cmd(NULL, ":%s NICK %s %lu",s_NewsServold,s_NewsServ,time(NULL));
s_NewsServold =s_NewsServ;
}
if (!s_IrcIIHelpold)
s_IrcIIHelpold ="";
if (strcmp(s_IrcIIHelp, s_IrcIIHelpold) != 0) {
 canaladmins(s_OperServ, "12s_IrcIIHelp Cambiando 4%s a 2%s",s_IrcIIHelpold,s_IrcIIHelp);
send_cmd(NULL, ":%s NICK %s %lu",s_IrcIIHelpold,s_IrcIIHelp,time(NULL));
s_IrcIIHelpold =s_IrcIIHelp;
}
if (!s_mIRCHelpold)
s_mIRCHelpold ="";
if (strcmp(s_mIRCHelp, s_mIRCHelpold) != 0) {
 canaladmins(s_OperServ, "12s_mIRCHelp Cambiando 4%s a 2%s",s_mIRCHelpold,s_mIRCHelp);
send_cmd(NULL, ":%s NICK %s %lu",s_mIRCHelpold,s_mIRCHelp,time(NULL));
s_mIRCHelpold =s_mIRCHelp;
}
if (!s_BddServold)
s_BddServold ="";
if (strcmp(s_BddServ, s_BddServold) != 0) {
 canaladmins(s_OperServ, "12s_BddServ Cambiando 4%s a 2%s",s_BddServold,s_BddServ);
send_cmd(NULL, ":%s NICK %s %lu",s_BddServold,s_BddServ,time(NULL));
s_BddServold =s_BddServ;
}
if (!s_ShadowServold)
s_ShadowServold ="";
if (strcmp(s_ShadowServ, s_ShadowServold) != 0) {
 canaladmins(s_OperServ, "12s_ShadowServ Cambiando 4%s a 2%s",s_ShadowServold,s_ShadowServ);
send_cmd(NULL, ":%s NICK %s %lu",s_ShadowServold,s_ShadowServ,time(NULL));
s_ShadowServold =s_ShadowServ;
}
if (!s_IpVirtualold)
s_IpVirtualold ="";
if (strcmp(s_IpVirtual, s_IpVirtualold) != 0) {
 canaladmins(s_OperServ, "12s_IpVirtual Cambiando 4%s a 2%s",s_IpVirtualold,s_IpVirtual);
send_cmd(NULL, ":%s NICK %s %lu",s_IpVirtualold,s_IpVirtual,time(NULL));
s_IpVirtualold =s_IpVirtual;
}
if (!s_GeoIPold)
s_GeoIPold ="";
if (strcmp(s_GeoIP, s_GeoIPold) != 0) {
 canaladmins(s_OperServ, "12s_GeoIP Cambiando 4%s a 2%s",s_GeoIPold,s_GeoIP);
send_cmd(NULL, ":%s NICK %s %lu",s_GeoIPold,s_GeoIP,time(NULL));
s_GeoIPold =s_GeoIP;
}
if (!s_JokuServold)
s_JokuServold ="";
if (strcmp(s_JokuServ, s_JokuServold) != 0) {
 canaladmins(s_OperServ, "12s_JokuServ Cambiando 4%s a 2%s",s_JokuServold,s_JokuServ);
send_cmd(NULL, ":%s NICK %s %lu",s_JokuServold,s_JokuServ,time(NULL));
s_JokuServold =s_JokuServ;
}
if (!DEntryMsgold)
DEntryMsgold ="";
if (strcmp(DEntryMsg, DEntryMsgold) != 0) {
 canaladmins(s_OperServ, "12DEntryMsg Cambiando 4%s a 2%s",DEntryMsgold,DEntryMsg);
DEntryMsgold =DEntryMsg;
}
if (!CregApoyosold)
CregApoyosold =0;
if (CregApoyos !=CregApoyosold) {
 canaladmins(s_OperServ, "12CregApoyos Cambiando 4 %d  a 2 %d ",CregApoyosold,CregApoyos);
CregApoyosold=CregApoyos;
}

if (!SpamUsersold)
SpamUsersold =0;
if (SpamUsers !=SpamUsersold) {
 canaladmins(s_OperServ, "12SpamUsers Cambiando 4 %d  a 2 %d ",SpamUsersold,SpamUsers);
SpamUsersold=SpamUsers;
}
#if defined(REG_NICK_MAIL)
#if defined(SENDMAIL)
if (!SendMailPatchold)
SendMailPatchold ="";
if (strcmp(SendMailPatch, SendMailPatchold) != 0) {
 canaladmins(s_OperServ, "12SendMailPatch Cambiando 4%s a 2%s",SendMailPatchold,SendMailPatch);
SendMailPatchold =SendMailPatch;
}
#endif
#if defined(SMTP)
if (!ServerSMTPold)
ServerSMTPold ="";
if (strcmp(ServerSMTP, ServerSMTPold) != 0) {
 canaladmins(s_OperServ, "12ServerSMTP Cambiando 4%d a 2%d",ServerSMTPold,ServerSMTP);
ServerSMTPold =ServerSMTP;
}
if (!PortSMTPold)
PortSMTPold =0;
if (PortSMTP !=PortSMTPold) {
 canaladmins(s_OperServ, "12PortSMTP Cambiando 4 %d  a 2 %d ",PortSMTPold,PortSMTP);
PortSMTPold=PortSMTP;
}

#endif
if (!NicksMailold)
NicksMailold =0;
if (NicksMail !=NicksMailold) {
 canaladmins(s_OperServ, "12NicksMail Cambiando 4 %d  a 2 %d ",NicksMailold,NicksMail);
NicksMailold =NicksMail;
}
if (!SendFromold)
SendFromold ="";
if (strcmp(SendFrom, SendFromold) != 0) {
 canaladmins(s_OperServ, "12SendFrom Cambiando 4%s a 2%s",SendFromold,SendFrom);
SendFromold =SendFrom;
}
if (!WebNetworkold)
WebNetworkold ="";
if (strcmp(WebNetwork, WebNetworkold) != 0) {
 canaladmins(s_OperServ, "12WebNetwork Cambiando 4%s a 2%s",WebNetworkold,WebNetwork);
WebNetworkold =WebNetwork;
}
#endif
int NSExpireoldval,NSExpireval;
if (!NSExpireold)
NSExpireold =0;
if (NSExpire >= 86400)
		NSExpireval = (NSExpire/86400);
	if (NSExpireold >= 86400)
		NSExpireoldval = (NSExpireold/86400);

if (NSExpire != NSExpireold) {
 canaladmins(s_OperServ, "12NSExpire Cambiando 4 %d  a 2 %d ",NSExpireoldval,NSExpireval);
NSExpireold =NSExpire;
}
int NSRegMailoldval,NSRegMailval;
if (!NSRegMailold)
NSRegMailold =0;
if (NSRegMail >= 86400)
		NSRegMailval = (NSRegMail/86400);
	if (NSRegMailold >= 86400)
		NSRegMailoldval = (NSRegMailold/86400);

if (NSRegMail != NSRegMailold) {
 canaladmins(s_OperServ, "12NSRegMail Cambiando 4 %d  a 2 %d ",NSRegMailoldval,NSRegMailval);
NSRegMailold =NSRegMail;
}

if (!NSListMaxold)
NSListMaxold =0;
if (NSListMax != NSListMaxold) {
 canaladmins(s_OperServ, "12NSListMax Cambiando 4 %d  a 2 %d ",NSListMaxold,NSListMax);
NSListMaxold =NSListMax;
}
if (!NSSecureAdminsold)
NSSecureAdminsold =0;
if (NSSecureAdmins !=NSSecureAdminsold)  {
 canaladmins(s_OperServ, "12NSSecureAdmins Cambiando 4 %d  a 2 %d ",NSSecureAdminsold,NSSecureAdmins);
NSSecureAdminsold =NSSecureAdmins;
}
if (!CSMaxRegold)
CSMaxRegold =0;
if (CSMaxReg !=CSMaxRegold) {
 canaladmins(s_OperServ, "12CSMaxReg Cambiando 4 %d  a 2 %d ",CSMaxRegold,CSMaxReg);
CSMaxRegold =CSMaxReg;
}
int CSExpireoldval,CSExpireval;
if (CSExpire >= 86400)
		CSExpireval = (CSExpire/86400);
	if (CSExpireold >= 86400)
		CSExpireoldval = (CSExpireold/86400);
if (!CSExpireold)
CSExpireold =0;
if (CSExpire !=CSExpireold)  {
 canaladmins(s_OperServ, "12CSExpire Cambiando 4 %d  a 2 %d ",CSExpireoldval,CSExpireval);
CSExpireold =CSExpire;
}
if (!CSAccessMaxold)
CSAccessMaxold =0;
if (CSAccessMax !=CSAccessMaxold) {
 canaladmins(s_OperServ, "12CSAccessMax Cambiando 4 %d  a 2 %d ",CSAccessMaxold,CSAccessMax);
CSAccessMaxold =CSAccessMax;
}
if (!CSAutokickMaxold)
CSAutokickMaxold =0;
if (CSAutokickMax !=CSAutokickMaxold) {
 canaladmins(s_OperServ, "12CSAutokickMax Cambiando 4 %d  a 2 %d ",CSAutokickMaxold,CSAutokickMax);
CSAutokickMaxold =CSAutokickMax;
}
if (!CSAutokickReasonold)
CSAutokickReasonold ="";
if (strcmp(CSAutokickReason, CSAutokickReasonold) != 0) {
 canaladmins(s_OperServ, "12CSAutokickReason Cambiando 4%s a 2%s",CSAutokickReasonold,CSAutokickReason);
CSAutokickReasonold =CSAutokickReason;
}
if (!CSListMaxold)
CSListMaxold =0;
if (CSListMax !=CSListMaxold) {
 canaladmins(s_OperServ, "12 CSListMax  Cambiando 4 %d  a 2 %d ",CSListMaxold,CSListMax);
CSListMaxold =CSListMax;
}
const char * pseudoservs[] ={s_NickServ,s_ChanServ,s_CregServ,s_OperServ,s_StatServ,s_EuskalIRCServ,s_HelpServ,s_IrcIIHelp,s_mIRCHelp,s_SpamServ,s_GlobalNoticer,s_NewsServ,s_BddServ,s_ShadowServ,s_IpVirtual,s_GeoIP,s_MemoServ,s_JokuServ};
int i;
if (!CanalAdminsold)
CanalAdminsold ="";
if (strcmp(CanalAdmins, CanalAdminsold) != 0) {
 canaladmins(s_OperServ, "12CanalAdmins Cambiando 4%s a 2%s",CanalAdminsold,CanalAdmins);
for (i = 0; i < 18; i++) {
send_cmd(pseudoservs[i], "JOIN #%s", CanalAdmins);
send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, pseudoservs[i]);
send_cmd(s_OperServ, "MODE #%s +ntsil 1", CanalAdmins);
send_cmd(pseudoservs[i], "PART #%s", CanalAdminsold);
}
CanalAdminsold =CanalAdmins;

}

if (!CanalOpersold)
CanalOpersold ="";
if (strcmp(CanalOpers, CanalOpersold) != 0) {
 canaladmins(s_OperServ, "12CanalOpers Cambiando 4%s a 2%s",CanalOpersold,CanalOpers);
for (i = 0; i < 6; i++) {
send_cmd(pseudoservs[i], "JOIN #%s", CanalOpers);
send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, pseudoservs[i]);
send_cmd(s_OperServ, "MODE #%s +ntsil 1", CanalOpers);
send_cmd(pseudoservs[i], "PART #%s", CanalOpersold);
}

CanalOpersold =CanalOpers;
}
if (!CanalCybersold)
CanalCybersold ="";
if (strcmp(CanalCybers, CanalCybersold) != 0) {
 canaladmins(s_OperServ, "12CanalCybers Cambiando 4%s a 2%s",CanalCybersold,CanalCybers);
CanalCybersold =CanalCybers;
}
if (!CanalAyudaold)
CanalAyudaold ="";
if (strcmp(CanalAyuda, CanalAyudaold) != 0) {
 canaladmins(s_OperServ, "12CanalAyuda Cambiando 4%s a 2%s",CanalAyudaold,CanalAyuda);
for (i = 5; i < 9; i++) {
send_cmd(pseudoservs[i], "JOIN #%s", CanalAyuda);
send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAyuda, pseudoservs[i]);
send_cmd(s_EuskalIRCServ, "MODE #%s +ntsil 1", CanalAyuda);
send_cmd(pseudoservs[i], "PART #%s", CanalAyudaold);
}
CanalAyudaold =CanalAyuda;
}
if (!CanalSpamersold)
CanalSpamersold="";
if (strcmp(CanalSpamers, CanalSpamersold) != 0) {
 canaladmins(s_OperServ, "12CanalSpamers Cambiando 4%s a 2%s",CanalSpamersold,CanalSpamers);
CanalSpamersold =CanalSpamers;
}
if (!RootNumberold)
RootNumberold =0;
if (RootNumber !=RootNumberold) {
 canaladmins(s_OperServ, "12RootNumber Cambiando 4 %d  a 2 %d ",RootNumberold,RootNumber);

RootNumberold =RootNumber;
}

if (!LogMaxUsersold)
LogMaxUsersold =0;
if (LogMaxUsers !=LogMaxUsersold) {
 canaladmins(s_OperServ, "12LogMaxUsers Cambiando 4 %d  a 2 %s ",LogMaxUsersold,LogMaxUsers);
LogMaxUsersold =LogMaxUsers;
}

}
/*************************************************************************/

static void do_restart(User *u)
{
#if defined(SERVICES_BIN)
    quitmsg = malloc(53 + strlen(u->nick));
    if (!quitmsg) {
	quitmsg = "Reiniciando Services...!";
   } else {
   sprintf(quitmsg, "Services ha recibido una orden de RESTART de %s", u->nick);
   }
  canaladmins(s_OperServ, "%s", quitmsg);
    save_data = 1;
    canaladmins(s_OperServ, "Actualizando bases de datos..");
  save_data = -2;
   /*raise(SIGHUP);*/
 #else
    notice_lang(s_OperServ, u, OPER_CANNOT_RESTART);
#endif
}

/*************************************************************************/

/* XXX - this function is broken!! */




static void do_listignore(User *u)
{
    char *pattern = strtok(NULL, " ");
    /*char buf[BUFSIZE];*/
    int sent_header = 0;
    IgnoreData *id;
    int nnicks, i;
    /*int is_director = is_services_root(u);*/
  struct tm *tm;
 char timebuf[64];
    if (!pattern) {
	pattern = "*";
	nnicks = 0;
    } else {
	nnicks = 0;
}
    for (i = 0; i < aliases; i++) {
	for (id = ignore[i]; id; id = id->next) {
		if (stricmp(pattern, id->who) == 0 ||
					match_wild_nocase(pattern, id->who)) {
			if (stricmp(u->nick, id->who) == 0)
				continue;
			 if (++nnicks <= NSListMax) {
	    if (!sent_header) {
		notice_lang(s_OperServ, u, OPER_IGNORE_LIST);
		sent_header = 1;
	    }
	  tm = localtime(&id->time);
          strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
	  notice(s_OperServ, u->nick, "2 %s  4 %s 12 %s  3( %s )", timebuf, id->who,id->servicio,id->inbuf);
	}
    }
    
}
}

if (!sent_header)
	notice_lang(s_OperServ, u, OPER_IGNORE_LIST_EMPTY);
}
/*************************************************************************/

static void do_debugserv(User *u)
{
    char *pattern = strtok(NULL, " ");
    /*char buf[BUFSIZE];*/
    int sent_header = 0;
    DebugData *id;
    int nnicks, i;
    /*int is_director = is_services_root(u);*/
  struct tm *tm;
 char timebuf[64];
    if (!pattern) {
	pattern = "*";
	nnicks = 0;
    } else {
	nnicks = 0;
}
    for (i = 0; i < aliases; i++) {
	for (id = debugserv[i]; id; id = id->next) {
		if (stricmp(pattern, id->who) == 0 ||
					match_wild_nocase(pattern, id->who)) {
			if (stricmp(u->nick, id->who) == 0)
				continue;
			 if (++nnicks <= NSListMax) {
	    if (!sent_header) {
		notice_lang(s_OperServ, u, OPER_IGNORE_LIST);
		sent_header = 1;
	    }
	  tm = localtime(&id->time);
          strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
	  notice(s_OperServ, u->nick, "2 %s  4 %s 12 %s  3( %s )", timebuf, id->who,id->servicio,id->inbuf);
	}
    }
    
}
}

if (!sent_header)
	notice_lang(s_OperServ, u, OPER_IGNORE_LIST_EMPTY);
}
/*************************************************************************/

static void do_matchwild(User *u)
{
    char *pat = strtok(NULL, " ");
    char *str = strtok(NULL, " ");
    if (pat && str)
	notice(s_OperServ, u->nick, "%d", match_wild(pat, str));
    else
	notice(s_OperServ, u->nick, "Syntax error.");
}

/* DEBUG_COMMANDS */
