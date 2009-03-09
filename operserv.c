/* OperServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"
#include "pseudo.h"


/*************************************************************************/

/* Services admin list */
static NickInfo *services_admins[MAX_SERVADMINS];

/* Services devel list */
static NickInfo *services_devels[MAX_SERVDEVELS];


/* Services operator list */
static NickInfo *services_opers[MAX_SERVOPERS];

/* Services patrocinadores list */
static NickInfo *services_patrocinas[MAX_SERVPATROCINAS];


/* Services bots list */
static NickInfo *services_bots[MAX_SERVOPERS];
/* Services cregadmins-coadmins list */
static NickInfo *services_cregadmins[MAX_SERVADMINS];


/*************************************************************************/
static void do_credits(User *u);
static void do_help(User *u);
static void do_global(User *u);
static void do_globaln(User *u);
static void do_stats(User *u);
static void do_admin(User *u);
static void do_devel(User *u);
static void do_patrocina(User *u);
static void do_oper(User *u);
static void do_coadmin(User *u);
static void do_os_op(User *u);
static void do_os_deop(User *u);
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
static void do_listignore(User *u);
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
    { "STATS",      do_stats,      NULL,  OPER_HELP_STATS,      -1,-1,-1,-1 },
    { "UPTIME",     do_stats,      NULL,  OPER_HELP_STATS,      -1,-1,-1,-1 },
    { "SPAM",       do_spam,       NULL,  -1,      -1,-1,-1,-1 },
    { "DUDA",	    do_euskal,	   is_services_oper,   OPER_HELP,   -1,-1,-1,-1 },
	/*Para el Bot EuskalIRC*/
    { "FORZAR",	    do_svsjoinparts,	   is_services_oper,   OPER_HELP,   -1,-1,-1,-1 },
    { "MODOS",	    do_svsmodes,	   is_services_admin,   OPER_HELP,   -1,-1,-1,-1 }, /*is_services_admin*/
	/* --donostiarra(2009)--
        con este ejemplo,ahorramos codigo en "MODOS" no asi en "FORZAR",
	Que queriendo restringir mas aun el acceso,
	con  <CregAdmin>*,nos obliga a declarar mas condicionales*/

    { "SERVERS",    do_servers,    NULL,  -1,                   -1,-1,-1,-1 },

    /* Anyone can use the LIST option to the ADMIN and OPER commands; those
     * routines check privileges to ensure that only authorized users
     * modify the list. */
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

    /* Commands for Services admins: */
    { "SET",        do_set,        is_services_admin,
	OPER_HELP_SET, -1,-1,-1,-1 },
    { "SET READONLY",0,0,  OPER_HELP_SET_READONLY, -1,-1,-1,-1 },
    { "SET DEBUG",0,0,     OPER_HELP_SET_DEBUG, -1,-1,-1,-1 },
    { "JUPE",       do_jupe,       is_services_admin,
	OPER_HELP_JUPE, -1,-1,-1,-1 }, 
    { "RAW",        do_raw,        is_services_admin,
	OPER_HELP_RAW, -1,-1,-1,-1 },
    { "UPDATE",     do_update,     is_services_admin,
	OPER_HELP_UPDATE, -1,-1,-1,-1 },
    { "QUIT",       do_os_quit,    is_services_admin,
	OPER_HELP_QUIT, -1,-1,-1,-1 },
    { "SHUTDOWN",   do_shutdown,   is_services_admin,
	OPER_HELP_SHUTDOWN, -1,-1,-1,-1 },
    { "RESTART",    do_restart,    is_services_admin,
	OPER_HELP_RESTART, -1,-1,-1,-1 },
    /*{ "LISTIGNORE", do_listignore, is_services_admin,
	-1,-1,-1,-1, -1 },*/	
   // { "MATCHWILD",  do_matchwild,       is_services_root, -1,-1,-1,-1,-1 },

    /* Commands for Services CoAdmins: */

  /* de channels.c */
{ "CHANLIST",  send_channel_list,  is_services_cregadmin, -1,-1,-1,-1,-1 },
{ "CHANUSERS",   send_channel_users, is_services_cregadmin, -1,-1,-1,-1,-1 },
/*de users.c*/
{ "USERLIST",  send_user_list,     is_services_cregadmin, -1,-1,-1,-1,-1 },
 { "USERINFO",   send_user_info,     is_services_cregadmin, -1,-1,-1,-1,-1 },

 /* Commands for Services Roots: */

 { "ROTATELOG",  rotate_log,  is_services_root, -1,-1,-1,-1,
	OPER_HELP_ROTATELOG },

#ifdef DEBUG_COMMANDS

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
	cmd->help_param1 = s_NickServ;
    if (cmd)
	cmd->help_param1 = s_NickServ;
    cmd = lookup_cmd(cmds, "DEVEL");
    if (cmd)
	cmd->help_param1 = s_NickServ;
    cmd = lookup_cmd(cmds, "PATROCINA");
    if (cmd)
	cmd->help_param1 = s_NickServ;
}

/*************************************************************************/

/* Main OperServ routine. */

void operserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	log("%s: user record for %s not found", s_OperServ, source);
	notice(s_OperServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    log("%s: %s: %s", s_OperServ, source, buf);

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
    SAFE(write_int32(maxusercnt, f));
    SAFE(write_int32(maxusertime, f));
    close_db(f);
}

#undef SAFE

/*************************************************************************/

/* Does the given user have Services root privileges? */

int is_services_root(User *u)
{
    
    if (!(u->mode & UMODE_O) || stricmp(u->nick, ServicesRoot) != 0)
	return 0;
    if (skeleton || nick_identified(u))
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

/*************************************************************************/
/* Es este usuario un bot? */

int is_a_service(char *nick)
{
   if ((stricmp(nick, s_NickServ) == 0) || (stricmp(nick, s_ChanServ) == 0) || (stricmp(nick, s_CregServ) == 0) || (stricmp(nick, s_MemoServ) == 0) || (stricmp(nick, s_OperServ) == 0) || (stricmp(nick, s_ShadowServ) == 0) || (stricmp(nick, s_BddServ) == 0) || (stricmp(nick, s_HelpServ) == 0) || (stricmp(nick, s_GlobalNoticer) == 0) || (stricmp(nick, s_NewsServ) == 0) || (stricmp(nick, s_EuskalIRCServ) == 0) || (stricmp(nick, s_SpamServ) == 0)|| (stricmp(nick, s_IpVirtual) == 0))
	return 1;
  else
  	return 0;
}
 


/*************************************************************************/

/* Listar los OPERS/ADMINS on-line */

void ircops(User *u)
{
    int i;
    int online = 0;
    NickInfo *ni, *ni2;
    
     for (i = 0; i < MAX_SERVPATROCINAS; i++) {
         if (services_patrocinas[i]) {
             ni = findnick(services_patrocinas[i]->nick);
             if (ni && (ni->status & NS_IDENTIFIED)) {
#ifdef IRC_UNDERNET_P10
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
#ifdef IRC_UNDERNET_P10
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
#ifdef IRC_UNDERNET_P10
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12COADMINISTRADOR de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12COADMINISTRADOR de 4%s y 4%s",
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
#ifdef IRC_UNDERNET_P10
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12DESARROLLADOR de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12DESARROLLADOR de 4%s y 4%s",
#endif
                     services_devels[i]->nick, s_NickServ, s_ChanServ);
                 online++;
              }
         }     
    }
                                                                                      
    for (i = 0; i < MAX_SERVADMINS; i++) {
         if (services_admins[i]) {
             ni2 = findnick(services_admins[i]->nick);
             if (ni2 && (ni2->status & NS_IDENTIFIED)) {             
#ifdef IRC_UNDERNET_P10
                 privmsg(s_ChanServ, u->numerico, "%-10s es 12ADMIN de 4%s y 4%s",
#else
                 privmsg(s_ChanServ, u->nick, "%-10s es 12ADMIN de 4%s y 4%s",
#endif
                     services_admins[i]->nick, s_NickServ, s_ChanServ);
                 online++;
             }
         }    
    }
    
   
#ifdef IRC_UNDERNET_P10
    privmsg(s_ChanServ, u->numerico, "12%d REPRESENTANTES de RED on-line", online);
#else
    privmsg(s_ChanServ, u->nick, "12%d REPRESENTANTES de RED  on-line", online);
#endif    
}
            


/*************************************************************************/

/* Is the given nick a Services admin/root nick? */

/* NOTE: Do not use this to check if a user who is online is a services admin
 * or root. This function only checks if a user has the ABILITY to be a 
 * services admin. Rather use is_services_admin(User *u). -TheShadow */

int nick_is_services_admin(NickInfo *ni)
{
    int i;

    if (!ni)
	return 0;
	
    if (stricmp(ni->nick, ServicesRoot) == 0)
	return 1;
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
	
    if (stricmp(ni->nick, ServicesRoot) == 0)
	return 1;
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
   if (stricmp(ni->nick, ServicesRoot) == 0)
       return 1;
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
	notice_help(s_OperServ, u, OPER_HELP,SpamUsers);
    } else {
	help_cmd(s_OperServ, u, cmds, cmd);
    }
}
void do_svsmodes(User *u)
{    
        char *nick, *modos;
        nick = strtok(NULL, " ");
        modos = strtok(NULL, " ");
        NickInfo *ni;
     
    if (!nick) {
             	 privmsg(s_OperServ,u->nick, " 4Falta un Nick /msg %s 2MODOS 12<NICK> 5<Modos>",s_OperServ);
    	return;
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	return;
	}
	 else if (ni && (ni->status & !NS_IDENTIFIED)) { 
         
         privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
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
        NickInfo *ni;
        ChannelInfo *ci;

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
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	return;
   
    }
     if (!canal) {
         if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
           }
    	 privmsg(s_OperServ,u->nick, " 4Falta un Canal /msg %s 2FORZAR 12ENTRADA/SALIDA 5<NICK> 2<CANAL>",s_OperServ);
    	return;
    } /*else if  (!(ci = cs_findchan(canal))) { 
	notice_lang(s_OperServ, u, CHAN_X_NOT_REGISTERED, canal);
	return; }  */ /*No nos preguntamos*/
   
    else if (ni && (ni->status & !NS_IDENTIFIED)) { 
         if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
           }
         privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
         return;
         }
      
      if (stricmp(cmd, "ENTRADA") == 0) {
	if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	} else if (ni && (ni->status & NS_IDENTIFIED)) {           
         send_cmd(ServerName, "SVSJOIN %s %s", nick,canal);
	canaladmins( s_OperServ,"12OPER 4%s 3SVSJOIN de  2%s al Canal 5%s",u->nick,nick,canal);
           }
             else 
               privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);

          }
       else if (stricmp(cmd, "SALIDA") == 0) {
	if (!is_services_cregadmin(u)) {
	    notice_lang( s_OperServ, u, PERMISSION_DENIED);
	    return;
	}  else if (ni && (ni->status & NS_IDENTIFIED)) {
          send_cmd(ServerName, "SVSPART %s %s", nick,canal);
	canaladmins( s_OperServ,"12OPER 4%s 3SVSPART de  2%s del Canal 5%s",u->nick,nick,canal);
	  }
          else 
               if (!is_services_cregadmin(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
           }
               privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);

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
# ifdef NETWORK_DOMAIN
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
# ifdef NETWORK_DOMAIN
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
                
# endif
#endif
    /* canalopers(s_GlobalNoticer, "12%s ha enviado el GLOBALN (%s)", u->nick, msg); */
}

/*************************************************************************/

/* STATS command. */

static void do_stats(User *u)
{
    time_t uptime = time(NULL) - start_time;
    char *extra = strtok(NULL, "");
    int days = uptime/86400, hours = (uptime/3600)%24,
        mins = (uptime/60)%60, secs = uptime%60;
    struct tm *tm;
    char timebuf[64];

    if (extra && stricmp(extra, "ALL") != 0) {
	if (stricmp(extra, "AKILL") == 0) {
	    int timeout = AutokillExpiry+59;
	    notice_lang(s_OperServ, u, OPER_STATS_AKILL_COUNT, num_akills());
	    if (timeout >= 172800)
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_DAYS,
			timeout/86400);
	    else if (timeout >= 86400)
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_DAY);
	    else if (timeout >= 7200)
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_HOURS,
			timeout/3600);
	    else if (timeout >= 3600)
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_HOUR);
	    else if (timeout >= 120)
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_MINS,
			timeout/60);
	    else if (timeout >= 60)
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_MIN);
	    else
		notice_lang(s_OperServ, u, OPER_STATS_AKILL_EXPIRE_NONE);
	    return;
	} else {
	    notice_lang(s_OperServ, u, OPER_STATS_UNKNOWN_OPTION,
			strupper(extra));
	}
    }

    notice_lang(s_OperServ, u, OPER_STATS_CURRENT_USERS, usercnt, opcnt);
    tm = localtime(&maxusertime);
    strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
    notice_lang(s_OperServ, u, OPER_STATS_MAX_USERS, maxusercnt, timebuf);
    if (days > 1) {
	notice_lang(s_OperServ, u, OPER_STATS_UPTIME_DHMS,
		days, hours, mins, secs);
    } else if (days == 1) {
	notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1DHMS,
		days, hours, mins, secs);
    } else {
	if (hours > 1) {
	    if (mins != 1) {
		if (secs != 1) {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_HMS,
				hours, mins, secs);
		} else {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_HM1S,
				hours, mins, secs);
		}
	    } else {
		if (secs != 1) {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_H1MS,
				hours, mins, secs);
		} else {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_H1M1S,
				hours, mins, secs);
		}
	    }
	} else if (hours == 1) {
	    if (mins != 1) {
		if (secs != 1) {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1HMS,
				hours, mins, secs);
		} else {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1HM1S,
				hours, mins, secs);
		}
	    } else {
		if (secs != 1) {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1H1MS,
				hours, mins, secs);
		} else {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1H1M1S,
				hours, mins, secs);
		}
	    }
	} else {
	    if (mins != 1) {
		if (secs != 1) {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_MS,
				mins, secs);
		} else {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_M1S,
				mins, secs);
		}
	    } else {
		if (secs != 1) {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1MS,
				mins, secs);
		} else {
		    notice_lang(s_OperServ, u, OPER_STATS_UPTIME_1M1S,
				mins, secs);
		}
	    }
	}
    }

    if (extra && stricmp(extra, "ALL") == 0 && is_services_admin(u)) {
	long count, mem, count2, mem2;

	notice_lang(s_OperServ, u, OPER_STATS_BYTES_READ, total_read / 1024);
	notice_lang(s_OperServ, u, OPER_STATS_BYTES_WRITTEN, 
			total_written / 1024);
        get_server_stats(&count, &mem);
        notice_lang(s_OperServ, u, OPER_STATS_SERVER_MEM,
                        count, (mem+512) / 1024);
	get_user_stats(&count, &mem);
	notice_lang(s_OperServ, u, OPER_STATS_USER_MEM,
			count, (mem+512) / 1024);
	get_channel_stats(&count, &mem);
	notice_lang(s_OperServ, u, OPER_STATS_CHANNEL_MEM,
			count, (mem+512) / 1024);
	get_nickserv_stats(&count, &mem);
	notice_lang(s_OperServ, u, OPER_STATS_NICKSERV_MEM,
			count, (mem+512) / 1024);
	get_chanserv_stats(&count, &mem);
	notice_lang(s_OperServ, u, OPER_STATS_CHANSERV_MEM,
			count, (mem+512) / 1024);
	count = 0;
	get_akill_stats(&count2, &mem2);
	count += count2;
	mem += mem2;
	get_news_stats(&count2, &mem2);
	count += count2;
	mem += mem2;
	notice_lang(s_OperServ, u, OPER_STATS_OPERSERV_MEM,
			count, (mem+512) / 1024);

    }
}

/*************************************************************************/

/* Op en un canal a traves del servidor */

static void do_os_op(User *u)
{
    char *chan = strtok(NULL, " ");
    char *op_params = strtok(NULL, " ");
    char *argv[3];
    
    Channel *c;

    if (!chan || !op_params) {
        syntax_error(s_OperServ, u, "OP", CHAN_OP_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else {
        char *destino;
        User *u2 =finduser(op_params);
               
        if (u2) {
#ifdef IRC_UNDERNET_P10
            destino = u2->numerico;
#else
            destino = u2->nick;
#endif                                    
            send_cmd(ServerName, "MODE %s +o %s", chan, destino); 

            argv[0] = sstrdup(chan);
            argv[1] = sstrdup("+o");
            argv[2] = sstrdup(destino);
            do_cmode(s_OperServ, 3, argv);
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
        } else
            notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, op_params);                                                                                                            
    }
}
                                                 
/*************************************************************************/

/* deop en un canal a traves de server */

static void do_os_deop(User *u)
{
    char *chan = strtok(NULL, " ");
    char *deop_params = strtok(NULL, " ");
    char *argv[3];
    
    Channel *c;

    if (!chan || !deop_params) {
        syntax_error(s_OperServ, u, "DEOP", CHAN_DEOP_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_OperServ, u, CHAN_X_NOT_IN_USE, chan);
    } else {
        char *destino;
        User *u2 =finduser(deop_params);
        
        if (u2) {
#ifdef IRC_UNDERNET_P10
            destino = u2->numerico;
#else
            destino = u2->nick;
#endif
                            
            send_cmd(ServerName, "MODE %s -o %s", chan, destino);
                                      
            argv[0] = sstrdup(chan);
            argv[1] = sstrdup("-o");
            argv[2] = sstrdup(destino);
            do_cmode(s_OperServ, 3, argv);
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
        } else        
            notice_lang(s_OperServ, u, NICK_X_NOT_IN_USE, deop_params);    
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

#ifdef IRC_UNDERNET_P10
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
#ifdef IRC_UNDERNET_P10		
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
	send_cmd(MODE_SENDER(s_OperServ), "MODE %s -iklmnpstMR :%s",
		chan, c->key ? c->key : "");
	argv[0] = sstrdup(chan);
	argv[1] = sstrdup("-iklmnpstMR");
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
#ifdef IRC_UNDERNET_P10
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
#ifdef IRC_UNDERNET_P10
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
#ifdef IRC_UNDERNET_P10              
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
    
    if (!chan) {
        privmsg(s_OperServ, u->nick, "Sintaxis: 12LIMPIA <canal>");
        return;
    } else if (!(c = findchan(chan))) {
        privmsg(s_OperServ, u->nick, "El canal 12%s esta vacio.", chan);
    } else {
        char *av[3];
        struct c_userlist *cu, *next;
        char buf[256];
       
        snprintf(buf, sizeof(buf), "No puedes permanecer en este canal");

#ifdef IRC_UNDERNET_P10
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
#ifdef IRC_UNDERNET_P10             
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
       #ifdef IRC_UNDERNET_P09
    	do_write_bdd(nick, 2, "");
	#endif
	notice_lang(s_OperServ, u, OPER_VHOST_UNSET, nick);
	return;
    }
    if (strlen(mask) > 64) {
	 privmsg(s_OperServ, u->nick, "V-Host demasiado larga. Máximo 64 carácteres.");
	 return;
    }
    
     #ifdef IRC_UNDERNET_P09
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
#ifdef IRC_UNDERNET_P10
        privmsg(s_OperServ, u->numerico, "Sintaxis: 12UNBLOCK/UNGLINE <*@host.es>");
#else
        privmsg(s_OperServ, u->nick, "Sintaxis: 12UNBLOCK/UNGLINE <*@host.es>");
#endif
    } else {
        send_cmd(ServerName ,"GLINE * -%s", mascara);
        canalopers(s_OperServ, "12%s ha usado UNBLOCK/UNGLINE en 12%s", u->nick, mascara);
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

/* Services admin list viewing/modification. */

static void do_admin(User *u)
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
		#ifdef IRC_UNDERNET_P09
	    	do_write_bdd(ni->nick, 3, "10");
	    	do_write_bdd(ni->nick, 23, "");
	        send_cmd(NULL, "RENAME %s", ni->nick);
		#endif
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
		#ifdef IRC_UNDERNET_P09
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
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
#ifdef IRC_UNDERNET_P10
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
		#ifdef IRC_UNDERNET_P09
		do_write_bdd(ni->nick, 3, "10"); //-->si lo añado a la tabla o y 10 para flag X
		do_write_bdd(ni->nick, 26, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
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
		#ifdef IRC_UNDERNET_P09
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
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
#ifdef IRC_UNDERNET_P10
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
		#ifdef IRC_UNDERNET_P09
	    	do_write_bdd(ni->nick, 3, "10");
	    	do_write_bdd(ni->nick, 24, "");
	        send_cmd(NULL, "RENAME %s", ni->nick);
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
		#ifdef IRC_UNDERNET_P09
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
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
#ifdef IRC_UNDERNET_P10
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
		#ifdef IRC_UNDERNET_P09
	    	do_write_bdd(ni->nick, 3, "5");
		do_write_bdd(ni->nick, 22, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
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
		#ifdef IRC_UNDERNET_P09
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		send_cmd(NULL, "RENAME %s", ni->nick);	
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
#ifdef IRC_UNDERNET_P10
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
		#ifdef IRC_UNDERNET_P09
	    	do_write_bdd(ni->nick, 3, ""); //-->No lo añado a la tabla o
		do_write_bdd(ni->nick, 25, "");
		send_cmd(NULL, "RENAME %s", ni->nick);
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
		#ifdef IRC_UNDERNET_P09
		do_write_bdd(ni->nick, 3, "");
		do_write_bdd(ni->nick, 2, "");
		send_cmd(NULL, "RENAME %s", ni->nick);	
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
#ifdef IRC_UNDERNET_P10
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
	} else {
	    notice_lang(s_OperServ, u, OPER_SET_IGNORE_ERROR);
	}

    } else if (stricmp(option, "READONLY") == 0) {
	if (stricmp(setting, "on") == 0) {
	    readonly = 1;
	    log("Read-only mode activated");
	    close_log();
	    notice_lang(s_OperServ, u, OPER_SET_READONLY_ON);
	} else if (stricmp(setting, "off") == 0) {
	    readonly = 0;
	    open_log();
	    log("Read-only mode deactivated");
	    notice_lang(s_OperServ, u, OPER_SET_READONLY_OFF);
	} else {
	    notice_lang(s_OperServ, u, OPER_SET_READONLY_ERROR);
	}

    } else if (stricmp(option, "DEBUG") == 0) {
	if (stricmp(setting, "on") == 0) {
	    debug = 1;
	    log("Debug mode activated");
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_ON);
	} else if (stricmp(setting, "off") == 0 ||
				(*setting == '0' && atoi(setting) == 0)) {
	    log("Debug mode deactivated");
	    debug = 0;
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_OFF);
	} else if (isdigit(*setting) && atoi(setting) > 0) {
	    debug = atoi(setting);
	    log("Debug mode activated (level %d)", debug);
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_LEVEL, debug);
	} else {
	    notice_lang(s_OperServ, u, OPER_SET_DEBUG_ERROR);
	}

    } else {
	notice_lang(s_OperServ, u, OPER_SET_UNKNOWN_OPTION, option);
    }
}

/*************************************************************************/

/*static void do_jupe(User *u)
{
    char *jserver = strtok(NULL, " ");
    char *reason = strtok(NULL, "");
    char buf[NICKMAX+16];
    Server *server;

#ifdef IRC_UNDERNET_P10
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
                
#ifdef IRC_UNDERNET_P09
        send_cmd(NULL, "SQUIT %s 0 :%s", jserver, reason);
        send_cmd(NULL, "SERVER %s 1 %lu %lu P10 :%s",
                   jserver, time(NULL), time(NULL), reason);               
        canalopers(s_OperServ, "argv[0] ya! y completado!");        
     
#elif defined (IRC_UNDERNET_P10)
        send_cmd(NULL, "%c SQ %s 0 :%s", convert2y[ServerNumerico], jserver, reason);
        send_cmd(NULL, "%c S %s 1 %lu %lu J10 %s 0 :%s",
            convert2y[ServerNumerico], jserver, time(NULL), time(NULL), server->numerico, reason);                                
#else
	send_cmd(NULL, "SERVER %s 2 :%s", jserver, reason);                        	
#endif
        privmsg(s_OperServ, destino, "Has JUPEado el servidor 12%s", jserver);
        canalopers(s_OperServ, "12JUPE  de %s por 12%s.",
                               jserver, u->nick);        
        log("%s: %s!%s@%s ha JUPEado a %s (%s)", s_OperServ, u->nick, u->realname, u->host,
            jserver, reason);
                                
    }
}
*/
static void do_jupe(User *u)
{
 char *jserver = strtok(NULL, " ");
 char *reason = strtok(NULL, "");
 int n=3;

 if ((!jserver) || (!reason)) {
	syntax_error(s_OperServ, u, "JUPE", OPER_RAW_SYNTAX);
    } else {
#ifdef IRC_UNDERNET_P10
 send_cmd(NULL, "SERVER %s %d 0 %ld J10 %cD] :%s",
             ServerName, n++, start_time, convert2y[ServerNumerico], reason); 
#endif
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
//    canaladmins(s_OperServ, "%s", quitmsg);
    quitting = 1;
}

/*************************************************************************/

static void do_shutdown(User *u)
{
    quitmsg = malloc(54 + strlen(u->nick));
    if (!quitmsg)
    
	quitmsg = "Aieee! SHUTDOWNing Services...!";
    else
	sprintf(quitmsg, "Aieee! Services ha recibido una orden de SHUTDOWN de %s", u->nick);
//    canaladmins(s_OperServ, "%s", quitmsg);
    save_data = 1;
    delayed_quit = 1;
}

/*************************************************************************/

static void do_restart(User *u)
{
#ifdef SERVICES_BIN
    quitmsg = malloc(53 + strlen(u->nick));
    if (!quitmsg)
	quitmsg = "Aieee! RESTARTing Services...!";
    else
	sprintf(quitmsg, "Aieee! Services ha recibido una orden de RESTART de %s", u->nick);
  canaladmins(s_OperServ, "%s", quitmsg);
   raise(SIGHUP);
 #else
    notice_lang(s_OperServ, u, OPER_CANNOT_RESTART);
#endif
}

/*************************************************************************/

/* XXX - this function is broken!! */

static void do_listignore(User *u)
{
    int sent_header = 0;
    IgnoreData *id;
    int i;

    notice(s_OperServ, u->nick, "Command disabled - it's broken.");
    return;
    
    for (i = 0; i < 256; i++) {
	for (id = ignore[i]; id; id = id->next) {
	    if (!sent_header) {
		notice_lang(s_OperServ, u, OPER_IGNORE_LIST);
		sent_header = 1;
	    }
	    notice(s_OperServ, u->nick, "%ld %s", id->time, id->who);
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
