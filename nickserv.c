/* NickServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

/*
 * Note that with the addition of nick links, most functions in Services
 * access the "effective nick" (u->ni) to determine privileges and such.
 * The only functions which access the "real nick" (u->real_ni) are:
 *	various functions which set/check validation flags
 *	    (validate_user, cancel_user, nick_{identified,recognized},
 *	     do_identify)
 *	validate_user (adding a collide timeout on the real nick)
 *	cancel_user (deleting a timeout on the real nick)
 *	do_register (checking whether the real nick is registered)
 *	do_drop (dropping the real nick)
 *	do_link (linking the real nick to another)
 *	do_unlink (unlinking the real nick)
 *	chanserv.c/do_register (setting the founder to the real nick)
 * plus a few functions in users.c relating to nick creation/changing.
 */

#include "services.h"
#include "pseudo.h"
#if defined(SOPORTE_MYSQL)
#include <mysql.h>
#endif
/*************************************************************************/
#define aliases 6000
static NickInfo *nicklists[aliases];	/* One for each initial character */

int numkills = 10;
#define TO_COLLIDE   0			/* Collide the user with this nick */
#define TO_RELEASE   1			/* Release a collided nick */

/*************************************************************************/

static int is_on_access(User *u, NickInfo *ni);
static void alpha_insert_nick(NickInfo *ni);
static NickInfo *makenick(const char *nick);
static int delnick(NickInfo *ni);
static void remove_links(NickInfo *ni);
static void delink(NickInfo *ni);

static void collide(NickInfo *ni, int from_timeout);
static void release(NickInfo *ni, int from_timeout);
static void add_ns_timeout(NickInfo *ni, int type, time_t delay);
static void del_ns_timeout(NickInfo *ni, int type);

static void do_credits(User *u);
static void do_help(User *u);
static void do_register(User *u);
static void do_identify(User *u);
static void do_drop(User *u);
static void do_validar(User *u);
static void do_confirm(User *u);
  /*donostiarra 2009*/
static void do_marcar(User *u);
static void do_desmarcar(User *u);

static void do_set(User *u);
static void do_set_password(User *u, NickInfo *ni, char *param);
static void do_set_language(User *u, NickInfo *ni, char *param);
static void do_set_url(User *u, NickInfo *ni, char *param);
static void do_set_email(User *u, NickInfo *ni, char *param);
static void do_set_timeregister(User *u, NickInfo *ni, char *param);
/*static void do_set_kill(User *u, NickInfo *ni, char *param);
static void do_set_secure(User *u, NickInfo *ni, char *param);*/
static void do_set_private(User *u, NickInfo *ni, char *param);
static void do_set_hide(User *u, NickInfo *ni, char *param);
static void do_set_noexpire(User *u, NickInfo *ni, char *param);
static void do_set_bdd(User *u, NickInfo *ni, char *param);
static void do_set_vhost(User *u, NickInfo *ni, char *param);
static void do_access(User *u);
/*static void do_link(User *u);
static void do_unlink(User *u);
static void do_listlinks(User *u);*/
static void do_info(User *u);
static void do_userip(User *u);
static void do_list(User *u);
/*static void do_recover(User *u);
static void do_release(User *u);*/
static void do_ghost(User *u);
static void do_status(User *u);
static void do_sendpass(User *u);
static void do_sendclave(User *u);
static void do_getpass(User *u);
static void do_getclave(User *u);
static void do_suspend(User *u);
static void do_unsuspend(User *u);
static void do_forbid(User *u);
static void do_unforbid(User *u);
static void do_rename(User *u);
/*static void do_getnewpass(User *u);*/

/*************************************************************************/

static Command cmds[] = {
    { "CREDITS",  do_credits,  NULL,  -1,                     -1,-1,-1,-1 },
    { "CREDITOS", do_credits,  NULL,  -1,                     -1,-1,-1,-1 },        
    { "HELP",     do_help,     NULL,  -1,                     -1,-1,-1,-1 },
    { "AYUDA",    do_help,     NULL,  -1,                     -1,-1,-1,-1 },
   /* { "GETNEWPASS",  do_getnewpass,  NULL,  -1,                     -1,-1,-1,-1 },*/
    { "SHOWCOMMANDS",    do_help,   NULL,  -1,                -1,-1,-1,-1 },    
    { ":?",       do_help,     NULL,  -1,                     -1,-1,-1,-1 },
    { "?",        do_help,     NULL,  -1,                     -1,-1,-1,-1 },            
    { "REGISTER", do_register, NULL,  NICK_HELP_REGISTER,     -1,-1,-1,-1 },
    { "IDENTIFY", do_identify, NULL,  NICK_HELP_IDENTIFY,     -1,-1,-1,-1 },
    { "AUTH",     do_identify, NULL,  NICK_HELP_IDENTIFY,     -1,-1,-1,-1 },    
    { "USERIP",	  do_userip,   is_services_devel,  NICK_SERVADMIN_HELP_USERIP,         -1,-1,-1,-1 }, 
    
    { "DROP",     do_drop,     is_services_cregadmin,  -1,
		NICK_HELP_DROP, NICK_SERVADMIN_HELP_DROP,
		NICK_SERVADMIN_HELP_DROP, NICK_SERVADMIN_HELP_DROP },
    { "VALIDAR",     do_validar,     is_services_admin,  -1,
		NICK_HELP_VALIDAR, NICK_SERVADMIN_HELP_VALIDAR,
		NICK_SERVADMIN_HELP_VALIDAR, NICK_SERVADMIN_HELP_VALIDAR },
    { "CONFIRM", do_confirm, NULL,  NICK_HELP_CONFIRM,     -1,-1,-1,-1 },
     /*para seguimiento de nicks*/
    { "MARCAR",     do_marcar,    is_services_devel,-1,-1,-1,-1 },
     { "DESMARCAR",     do_desmarcar,    is_services_devel,-1,-1,-1,-1 },

    { "ACCESS",   do_access,   NULL,  NICK_HELP_ACCESS,       -1,-1,-1,-1 },
    /*{ "LINK",     do_link,     NULL,  NICK_HELP_LINK,         -1,-1,-1,-1 },
    { "UNLINK",   do_unlink,   NULL,  NICK_HELP_UNLINK,       -1,-1,-1,-1 },*/
    { "SET",      do_set,      NULL,  NICK_HELP_SET,
		-1, NICK_SERVADMIN_HELP_SET,
		NICK_SERVADMIN_HELP_SET, NICK_SERVADMIN_HELP_SET },
    { "MODUSER",  do_set,      NULL,  NICK_HELP_SET,
                -1, NICK_SERVADMIN_HELP_SET,
                NICK_SERVADMIN_HELP_SET, NICK_SERVADMIN_HELP_SET },
                                    		
    { "SET PASSWORD", NULL,    NULL,  NICK_HELP_SET_PASSWORD, -1,-1,-1,-1 },
    { "SET PASS",     NULL,    NULL,  NICK_HELP_SET_PASSWORD, -1,-1,-1,-1 },    
    { "SET URL",      NULL,    NULL,  NICK_HELP_SET_URL,      -1,-1,-1,-1 },
    { "SET EMAIL",    NULL,    NULL,  NICK_HELP_SET_EMAIL,    -1,-1,-1,-1 },
    { "SET KILL",     NULL,    NULL,  NICK_HELP_SET_KILL,     -1,-1,-1,-1 },
    { "SET SECURE",   NULL,    NULL,  NICK_HELP_SET_SECURE,   -1,-1,-1,-1 },
    { "SET PRIVATE",  NULL,    NULL,  NICK_HELP_SET_PRIVATE,  -1,-1,-1,-1 },
    { "SET HIDE",     NULL,    NULL,  NICK_HELP_SET_HIDE,     -1,-1,-1,-1 },
    { "SET NOEXPIRE", NULL,    is_services_root,  -1, -1,
		NICK_SERVADMIN_HELP_SET_NOEXPIRE,
		NICK_SERVADMIN_HELP_SET_NOEXPIRE,
		NICK_SERVADMIN_HELP_SET_NOEXPIRE },
     { "SET TIMEREG", NULL,    is_services_root,  -1, -1,
		NICK_SERVADMIN_HELP_SET_TIMEREG,
		NICK_SERVADMIN_HELP_SET_TIMEREG,
		NICK_SERVADMIN_HELP_SET_TIMEREG },
    { "SET BDD",      NULL,    NULL,  -1,                     -1,-1,-1,-1 },
    { "SET VHOST", NULL,       NULL,  NICK_HELP_SET_VHOST,    -1,-1,-1,-1 },
    /*{ "RECOVER",  do_recover,  NULL,  NICK_HELP_RECOVER,      -1,-1,-1,-1 },
    { "RELEASE",  do_release,  NULL,  NICK_HELP_RELEASE,      -1,-1,-1,-1 },
    { "FREE",     do_release,  NULL,  NICK_HELP_RELEASE,      -1,-1,-1,-1 },    */
    { "GHOST",    do_ghost,    NULL,  NICK_HELP_GHOST,        -1,-1,-1,-1 },
    { "INFO",     do_info,     NULL,  NICK_HELP_INFO,
		-1, NICK_HELP_INFO, NICK_SERVADMIN_HELP_INFO,
		NICK_SERVADMIN_HELP_INFO },
    { "LIST",     do_list,     NULL,  -1,
		NICK_HELP_LIST, NICK_SERVADMIN_HELP_LIST,
		NICK_SERVADMIN_HELP_LIST, NICK_SERVADMIN_HELP_LIST },
    { "STATUS",   do_status,   NULL,  NICK_HELP_STATUS,       -1,-1,-1,-1 },
    /*{ "LISTLINKS",do_listlinks,is_services_admin, -1,
		-1, NICK_SERVADMIN_HELP_LISTLINKS,
		NICK_SERVADMIN_HELP_LISTLINKS, NICK_SERVADMIN_HELP_LISTLINKS },*/
    { "SENDPASS",  do_sendpass,  is_services_oper,  -1,
                -1, NICK_SERVADMIN_HELP_SENDPASS,
                NICK_SERVADMIN_HELP_SENDPASS, NICK_SERVADMIN_HELP_SENDPASS },
     { "SENDCLAVE",  do_sendclave,  is_services_oper,  -1,
                -1, NICK_SERVADMIN_HELP_SENDCLAVE,
                NICK_SERVADMIN_HELP_SENDCLAVE, NICK_SERVADMIN_HELP_SENDCLAVE },
    
    { "RENAME",  do_rename, is_services_cregadmin,   -1,	      -1,-1,-1,-1 }, 

    { "GETPASS",  do_getpass,  is_services_devel,  -1,
                -1, NICK_SERVADMIN_HELP_GETPASS,
                NICK_SERVADMIN_HELP_GETPASS, NICK_SERVADMIN_HELP_GETPASS },
    { "GETCLAVE",  do_getclave,  is_services_root,  -1,
                -1, NICK_SERVADMIN_HELP_GETPASS,
                NICK_SERVADMIN_HELP_GETCLAVE, NICK_SERVADMIN_HELP_GETCLAVE },
    { "SUSPEND",  do_suspend,  is_services_devel,  -1,
                -1, NICK_SERVADMIN_HELP_SUSPEND,
                NICK_SERVADMIN_HELP_SUSPEND, NICK_SERVADMIN_HELP_SUSPEND },
    { "UNSUSPEND",  do_unsuspend,  is_services_devel,  -1,
                -1, NICK_SERVADMIN_HELP_UNSUSPEND,
                NICK_SERVADMIN_HELP_UNSUSPEND, NICK_SERVADMIN_HELP_UNSUSPEND },
    { "FORBID",   do_forbid,   is_services_cregadmin,  -1,
                -1, NICK_SERVADMIN_HELP_FORBID,
                NICK_SERVADMIN_HELP_FORBID, NICK_SERVADMIN_HELP_FORBID },
    { "UNFORBID",   do_unforbid,   is_services_cregadmin,  -1,
                -1, NICK_SERVADMIN_HELP_UNFORBID,
                NICK_SERVADMIN_HELP_UNFORBID, NICK_SERVADMIN_HELP_UNFORBID },
    { NULL }
};

/*************************************************************************/
/*************************************************************************/

/* Display total number of registered nicks and info about each; or, if
 * a specific nick is given, display information about that nick (like
 * /msg NickServ INFO <nick>).  If count_only != 0, then only display the
 * number of registered nicks (the nick parameter is ignored).
 */

void listnicks(int count_only, const char *nick)
{
    int count = 0;
    NickInfo *ni;
    int i;
    char *end;

    if (count_only) {

	for (i = 0; i < aliases; i++) {
	    for (ni = nicklists[i]; ni; ni = ni->next)
		count++;
	}
	printf("%d Nicks Registrados.\n", count);

    } else if (nick) {

	struct tm *tm;
	char buf[512];
	static const char commastr[] = ", ";
	int need_comma = 0;

	if (!(ni = findnick(nick))) {
	    printf("%s no est� registrado.\n", nick);
	    return;
	} else if (ni->status & NS_VERBOTEN) {
	    printf("%s est� FORBIDeado.\n", nick);
	    return;
	}
	if (ni->status & NS_SUSPENDED)
          printf("SUSPENDIDO:%s\n", ni->last_quit);
	printf("Realname de %s es %s\n", nick, ni->last_realname);
	printf("�ltima direcci�n: %s\n", ni->last_usermask);
	tm = localtime(&ni->time_registered);
	strftime(buf, sizeof(buf), getstring((NickInfo *)NULL,STRFTIME_DATE_TIME_FORMAT), tm);
	printf("Hora de registro: %s\n", buf);
	tm = localtime(&ni->last_seen);
	strftime(buf, sizeof(buf), getstring((NickInfo *)NULL,STRFTIME_DATE_TIME_FORMAT), tm);
	printf("�ltima hora por la red: %s\n", buf);
	tm = localtime(&ni->expira_min);
	strftime(buf, sizeof(buf), getstring((NickInfo *)NULL,STRFTIME_DATE_TIME_FORMAT), tm);
	printf("Tiempo M�nimo Expiraci�n: %s\n", buf);
        
	if (ni->url)
	    printf("URL: %s\n", ni->url);
	if (ni->email)
	    printf("E-mail: %s\n", ni->email);
	*buf = 0;
	end = buf;
	if (ni->flags & NI_KILLPROTECT) {
	    end += snprintf(end, sizeof(buf)-(end-buf), "Protecci�n de Kill");
	    need_comma = 1;
	}
	if (ni->flags & NI_SECURE) {
	    end += snprintf(buf, sizeof(buf)-(end-buf), "%sSeguro",
			need_comma ? commastr : "");
	    need_comma = 1;
	}
	if (ni->flags & NI_PRIVATE) {
	    end += snprintf(buf, sizeof(buf)-(end-buf), "%sPrivado",
			need_comma ? commastr : "");
	    need_comma = 1;
	}
	if (ni->estado & NS_MARCADO) {
	    end += snprintf(buf, sizeof(buf)-(end-buf), "%sMarcado",
			need_comma ? commastr : "");
	    need_comma = 1;
	}
	if (ni->status & NS_NO_EXPIRE) {
	    end += snprintf(buf, sizeof(buf)-(end-buf), "%sNo Expira",
			need_comma ? commastr : "");
	    need_comma = 1;
	}
	if (ni->env_mail & MAIL_REC) {
	    end += snprintf(buf, sizeof(buf)-(end-buf), "%sRecibi� mail recordatorio",
			need_comma ? commastr : "");
	    need_comma = 1;
	}
	if (ni->status & NI_ON_BDD) {
	    end += snprintf(buf, sizeof(buf)-(end-buf), "%sMigrado a BDD",
	    		need_comma ? commastr : "");
	    need_comma = 1;
	}
	
	printf("Opciones: %s\n", *buf ? buf : "Ninguna");

    } else {

	for (i = 0; i < aliases; i++) {
	    for (ni = nicklists[i]; ni; ni = ni->next) {
		printf("    %s %-20s  %s\n", 
			    ni->status & NS_NO_EXPIRE ? "!" : " ", 
			    ni->nick, ni->status & NS_VERBOTEN ? 
				     "Inactivo (FORBID)" : ni->last_usermask);
		count++;
	    }
	}
	printf("%d Nicks registrados.\n", count);

    }
}

/*************************************************************************/

/* Return information on memory use.  Assumes pointers are valid. */

void get_nickserv_stats(long *nrec, long *memuse)
{
    long count = 0, mem = 0;
    int i, j;
    NickInfo *ni;
    char **accptr;

    for (i = 0; i < aliases; i++) {
	for (ni = nicklists[i]; ni; ni = ni->next) {
	    count++;
	    mem += sizeof(*ni);
	    if (ni->url)
		mem += strlen(ni->url)+1;
	    if (ni->email)
		mem += strlen(ni->email)+1;
	    if (ni->vhost)
		mem += strlen(ni->vhost)+1;
	    if (ni->last_usermask)
		mem += strlen(ni->last_usermask)+1;
	    if (ni->last_realname)
		mem += strlen(ni->last_realname)+1;
	    if (ni->last_quit)
		mem += strlen(ni->last_quit)+1;
            if (ni->suspendby)
                mem += strlen(ni->suspendby)+1;
	    if (ni->nickoper)
                mem += strlen(ni->nickoper)+1;
            if (ni->suspendreason)
                mem += strlen(ni->suspendreason)+1;
	   if (ni->motivo)
                mem += strlen(ni->motivo)+1;
	 
            if (ni->forbidby)
                mem += strlen(ni->forbidby)+1;
            if (ni->forbidreason)
                mem += strlen(ni->forbidreason)+1;		
	    mem += sizeof(char *) * ni->accesscount;
	    for (accptr=ni->access, j=0; j < ni->accesscount; accptr++, j++) {
		if (*accptr)
		    mem += strlen(*accptr)+1;
	    }
	    mem += ni->memos.memocount * sizeof(Memo);
	    for (j = 0; j < ni->memos.memocount; j++) {
		if (ni->memos.memos[j].text)
		    mem += strlen(ni->memos.memos[j].text)+1;
	    }
	}
    }
    *nrec = count;
    *memuse = mem;
}

/*************************************************************************/
/*************************************************************************/

/* NickServ initialization. */

void ns_init(void)
{
}

/*************************************************************************/

/* Main NickServ routine. */

void nickserv(const char *source, char *buf)
{
    char *cmd, *s;
    User *u = finduser(source);

    if (!u) {
	log("%s: user record for %s not found", s_NickServ, source);
#ifndef IRC_UNDERNET_P10
	privmsg(s_NickServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
#endif		
	return;
    }

    cmd = strtok(buf, " ");

    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
#if defined(IRC_UNDERNET_P10)
        notice(s_NickServ, u->numerico, "\1PING %s", s);
#else
	notice(s_NickServ, source, "\1PING %s", s);
#endif	
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_NickServ, source, "\1VERSION %s %s -- %s\1",
                PNAME, s_NickServ, version_build);
                               
    } else if (skeleton) {
	notice_lang(s_NickServ, u, SERVICE_OFFLINE, s_NickServ);
    } else {
	run_cmd(s_NickServ, u, cmds, cmd);
    }

}

/*************************************************************************/

/* Load/save data files. */


#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Error de lectura en %s", NickDBName);	\
	failed = 1;					\
	break;						\
    }							\
} while (0)


static void load_old_ns_dbase(dbFILE *f, int ver)
{
    struct nickinfo_ {
	NickInfo *next, *prev;
	char nick[NICKMAX];
	char pass[PASSMAX];
	char clave[PASSMAX];
	char *last_usermask;
	char *last_realname;
	time_t time_registered;
	time_t last_seen;
	time_t expira_min;
	long accesscount;
	char **access;
	long flags;
	long active;
	long estado;
	time_t id_timestamp;
	unsigned short memomax;
	unsigned short channelcount;
	char *url;
	char *email;
	char *vhost;
    } old_nickinfo;

    int i, j, c;
    NickInfo *ni, **last, *prev;
    int failed = 0;

    for (i = 33; i < aliases && !failed; i++) {
	last = &nicklists[i];
	prev = NULL;
	while ((c = getc_db(f)) != 0) {
	    if (c != 1)
		fatal("Invalid format in %s", NickDBName);
	    SAFE(read_variable(old_nickinfo, f));
	    if (debug >= 3)
		log("debug: load_old_ns_dbase read nick %s", old_nickinfo.nick);
	    ni = scalloc(1, sizeof(NickInfo));
	    *last = ni;
	    last = &ni->next;
	    ni->prev = prev;
	    prev = ni;
	    strscpy(ni->nick, old_nickinfo.nick, NICKMAX);
	    strscpy(ni->pass, old_nickinfo.pass, PASSMAX);
	    strscpy(ni->clave, old_nickinfo.clave, PASSMAX);
	    ni->time_registered = old_nickinfo.time_registered;
	    ni->last_seen = old_nickinfo.last_seen;
	   ni->expira_min = old_nickinfo.expira_min;
	    ni->accesscount = old_nickinfo.accesscount;
	    ni->flags = old_nickinfo.flags;
	       ni->active = old_nickinfo.active;
	     ni->estado = old_nickinfo.estado;
	    if (ver < 3)	/* Memo max field created in ver 3 */
		ni->memos.memomax = MSMaxMemos;
	    else if (old_nickinfo.memomax)
		ni->memos.memomax = old_nickinfo.memomax;
	    else
		ni->memos.memomax = -1;  /* Unlimited is now -1 */
	    /* Reset channel count because counting was broken in old
	     * versions; load_old_cs_dbase() will calculate the count */
	    ni->channelcount = 0;
	    ni->channelmax = CSMaxReg;
	    ni->language = DEF_LANGUAGE;
	    /* ENCRYPTEDPW and VERBOTEN moved from ni->flags to ni->status */
	    if (ni->flags & 4)
		ni->status |= NS_VERBOTEN;
	    if (ni->flags & 8)
		ni->status |= NS_ENCRYPTEDPW;
	    ni->flags &= ~0xE000000C;
#if defined(USE_ENCRYPTION)
	    if (!(ni->status & (NS_ENCRYPTEDPW | NS_VERBOTEN))) {
		if (debug)
		    log("debug: %s: encrypting password for `%s' on load",
				s_NickServ, ni->nick);
		if (encrypt_in_place(ni->pass, PASSMAX) < 0)
		    fatal("%s: Can't encrypt `%s' nickname password!",
				s_NickServ, ni->nick);
		ni->status |= NS_ENCRYPTEDPW;
	    }
#else
	    if (ni->status & NS_ENCRYPTEDPW) {
		/* Bail: it makes no sense to continue with encrypted
		 * passwords, since we won't be able to verify them */
		fatal("%s: load database: password for %s encrypted "
		          "but encryption disabled, aborting",
		          s_NickServ, ni->nick);
	    }
#endif
	    if (old_nickinfo.url)
		SAFE(read_string(&ni->url, f));
	    if (old_nickinfo.email)
		SAFE(read_string(&ni->email, f));
	    if (old_nickinfo.vhost)
		SAFE(read_string(&ni->vhost, f));
	    SAFE(read_string(&ni->last_usermask, f));
	    if (!ni->last_usermask)
		ni->last_usermask = sstrdup("@");
	    SAFE(read_string(&ni->last_realname, f));
	    if (!ni->last_realname)
		ni->last_realname = sstrdup("");
	    if (ni->accesscount) {
		char **access, *s;
		if (ni->accesscount > NSAccessMax)
		    ni->accesscount = NSAccessMax;
		access = smalloc(sizeof(char *) * ni->accesscount);
		ni->access = access;
		for (j = 0; j < ni->accesscount; j++, access++)
		    SAFE(read_string(access, f));
		while (j < old_nickinfo.accesscount) {
		    SAFE(read_string(&s, f));
		    if (s)
			free(s);
		    j++;
		}
	    }
	    ni->id_timestamp = 0;
	    if (ver < 3) {
		ni->flags |= NI_MEMO_SIGNON | NI_MEMO_RECEIVE;
	    } else if (ver == 3) {
		if (!(ni->flags & (NI_MEMO_SIGNON | NI_MEMO_RECEIVE)))
		    ni->flags |= NI_MEMO_SIGNON | NI_MEMO_RECEIVE;
	    }
	} /* while (getc_db(f) != 0) */
	*last = NULL;
    } /* for (i) */
    if (debug >= 2)
	log("debug: load_old_ns_dbase(): loading memos");
    load_old_ms_dbase();
}


void load_ns_dbase(void)
{
    dbFILE *f;
    int ver, i, j, c;
    NickInfo *ni, **last, *prev;
    int failed = 0;

    if (!(f = open_db(s_NickServ, NickDBName, "r")))
	return;

    switch (ver = get_file_version(f)) {
      
      case 8:
      case 7:
      case 6:
      case 5:
	for (i = 0; i < aliases && !failed; i++) {
	    int32 tmp32;
	    last = &nicklists[i];
	    prev = NULL;
	    while ((c = getc_db(f)) == 1) {
		if (c != 1)
		    fatal("Invalid format in %s", NickDBName);
		ni = scalloc(sizeof(NickInfo), 1);
		*last = ni;
		last = &ni->next;
		ni->prev = prev;
		prev = ni;
		SAFE(read_buffer(ni->nick, f));
		SAFE(read_buffer(ni->pass, f));
		SAFE(read_buffer(ni->clave, f));
		SAFE(read_string(&ni->url, f));
             
		SAFE(read_string(&ni->email, f));
		SAFE(read_string(&ni->vhost, f));
		SAFE(read_string(&ni->last_usermask, f));
		if (!ni->last_usermask)
		    ni->last_usermask = sstrdup("@");
		SAFE(read_string(&ni->last_realname, f));
		if (!ni->last_realname)
		    ni->last_realname = sstrdup("");
		SAFE(read_string(&ni->last_quit, f));
		SAFE(read_int32(&tmp32, f));
		ni->time_registered = tmp32;
		SAFE(read_int32(&tmp32, f));
		ni->last_seen = tmp32;
		SAFE(read_int32(&tmp32, f));
		ni->expira_min = tmp32;
		SAFE(read_int16(&ni->status, f));
		ni->status &= ~NS_TEMPORARY;
		SAFE(read_int16(&ni->env_mail, f));  /*mail recordando que se le expira el nick*/
		#if defined(USE_ENCRYPTION)
		if (!(ni->status & (NS_ENCRYPTEDPW | NS_VERBOTEN))) {
		    if (debug)
			log("debug: %s: encrypting password for `%s' on load",
				s_NickServ, ni->nick);
		    if (encrypt_in_place(ni->pass, PASSMAX) < 0)
			fatal("%s: Can't encrypt `%s' nickname password!",
				s_NickServ, ni->nick);
		    ni->status |= NS_ENCRYPTEDPW;
		}
#else
		if (ni->status & NS_ENCRYPTEDPW) {
		    /* Bail: it makes no sense to continue with encrypted
		     * passwords, since we won't be able to verify them */
		    fatal("%s: load database: password for %s encrypted "
		          "but encryption disabled, aborting",
		          s_NickServ, ni->nick);
		}
#endif
                if (ver >= 8) {
                    SAFE(read_string(&ni->suspendby, f));
			  SAFE(read_string(&ni->nickactoper, f));
		       SAFE(read_string(&ni->nickoper, f));
                    SAFE(read_string(&ni->suspendreason, f));
		    SAFE(read_string(&ni->motivo, f));
                    SAFE(read_int32(&tmp32, f));
                    ni->time_suspend = tmp32;
			SAFE(read_int32(&tmp32, f));
                    ni->time_motivo = tmp32;
                    SAFE(read_int32(&tmp32, f));
                    ni->time_expiresuspend = tmp32;
		    
                    SAFE(read_string(&ni->forbidby, f));
                    SAFE(read_string(&ni->forbidreason, f));
                } else {
                    ni->suspendby = NULL;
		       ni->nickactoper = NULL;
		    ni->nickoper = NULL;
                    ni->suspendreason = NULL;
		       ni->motivo = NULL;
                    ni->time_suspend = 0;
		     ni->time_motivo = 0;
                    ni->time_expiresuspend = 0;
		    
                    ni->forbidby = NULL;
                    ni->forbidreason = NULL;
                }    
		/* Store the _name_ of the link target in ni->link for now;
		 * we'll resolve it after we've loaded all the nicks */
		SAFE(read_string((char **)&ni->link, f));
		SAFE(read_int16(&ni->linkcount, f));
		if (ni->link) {
		    SAFE(read_int16(&ni->channelcount, f));
		    /* No other information saved for linked nicks, since
		     * they get it all from their link target */
		    ni->flags = 0;
		     ni->active = 0;
		    ni->estado = 0;
		    ni->accesscount = 0;
		    ni->access = NULL;
		    ni->memos.memocount = 0;
		    ni->memos.memomax = MSMaxMemos;
		    ni->memos.memos = NULL;
		    ni->channelmax = CSMaxReg;
		    ni->language = DEF_LANGUAGE;
		} else {
		    SAFE(read_int32(&ni->flags, f));
		     SAFE(read_int32(&ni->active, f));
		    SAFE(read_int32(&ni->estado, f));
		    if (!NSAllowKillImmed)
			ni->flags &= ~NI_KILL_IMMED;
		    SAFE(read_int16(&ni->accesscount, f));
		    if (ni->accesscount) {
			char **access;
			access = smalloc(sizeof(char *) * ni->accesscount);
			ni->access = access;
			for (j = 0; j < ni->accesscount; j++, access++)
			    SAFE(read_string(access, f));
		    }
		    SAFE(read_int16(&ni->memos.memocount, f));
		    SAFE(read_int16(&ni->memos.memomax, f));
		    if (ni->memos.memocount) {
			Memo *memos;
			memos = smalloc(sizeof(Memo) * ni->memos.memocount);
			ni->memos.memos = memos;
			for (j = 0; j < ni->memos.memocount; j++, memos++) {
			    SAFE(read_int32(&memos->number, f));
			    SAFE(read_int16(&memos->flags, f));
			    SAFE(read_int32(&tmp32, f));
			    memos->time = tmp32;
			    SAFE(read_buffer(memos->sender, f));
			    SAFE(read_string(&memos->text, f));
			}
		    }
		    SAFE(read_int16(&ni->channelcount, f));
		    SAFE(read_int16(&ni->channelmax, f));
		    if (ver == 5) {
			/* Fields not initialized properly for new nicks */
			/* These will be updated by load_cs_dbase() */
			ni->channelcount = 0;
			ni->channelmax = CSMaxReg;
		    }
		    SAFE(read_int16(&ni->language, f));
		}
		ni->id_timestamp = 0;
	    } /* while (getc_db(f) != 0) */
	    *last = NULL;
	} /* for (i) */

	/* Now resolve links */
	for (i = 0; i < aliases; i++) {
	    for (ni = nicklists[i]; ni; ni = ni->next) {
		if (ni->link)
		    ni->link = findnick((char *)ni->link);
	    }
	}

	break;

      case 4:
      case 3:
      case 2:
      case 1:
	load_old_ns_dbase(f, ver);
	break;

      default:
	fatal("Unsupported version number (%d) on %s", ver, NickDBName);

    } /* switch (version) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {						\
    if ((x) < 0) {						\
	restore_db(f);						\
	log_perror("Write error on %s", NickDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {		\
	    canalopers(NULL, "Write error on %s: %s", NickDBName,	\
			strerror(errno));			\
	    lastwarn = time(NULL);				\
	}							\
	return;							\
    }								\
} while (0)

void save_ns_dbase(void)
{
    dbFILE *f;
    int i, j;
    NickInfo *ni;
    char **access;
    Memo *memos;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_NickServ, NickDBName, "w")))
	return;
    for (i = 0; i < aliases; i++) {
	for (ni = nicklists[i]; ni; ni = ni->next) {
	    SAFE(write_int8(1, f));
	    SAFE(write_buffer(ni->nick, f));
	    SAFE(write_buffer(ni->pass, f));
	    SAFE(write_buffer(ni->clave, f));
	    SAFE(write_string(ni->url, f));
	    SAFE(write_string(ni->email, f));
	     SAFE(write_string(ni->vhost, f));
	    SAFE(write_string(ni->last_usermask, f));
	    SAFE(write_string(ni->last_realname, f));
	    SAFE(write_string(ni->last_quit, f));
	    SAFE(write_int32(ni->time_registered, f));
	    SAFE(write_int32(ni->last_seen, f));
	    SAFE(write_int32(ni->expira_min, f));
	    SAFE(write_int16(ni->status, f));
	   SAFE(write_int16(ni->env_mail, f));
            SAFE(write_string(ni->suspendby, f));
	     SAFE(write_string(ni->nickactoper, f));
	    SAFE(write_string(ni->nickoper, f));
            SAFE(write_string(ni->suspendreason, f));
	    SAFE(write_string(ni->motivo, f));
            SAFE(write_int32(ni->time_suspend, f));
	  SAFE(write_int32(ni->time_motivo, f));
            SAFE(write_int32(ni->time_expiresuspend, f));
	
            SAFE(write_string(ni->forbidby, f));
            SAFE(write_string(ni->forbidreason, f));                                                              
	    if (ni->link) {
		SAFE(write_string(ni->link->nick, f));
		SAFE(write_int16(ni->linkcount, f));
		SAFE(write_int16(ni->channelcount, f));
	    } else {
		SAFE(write_string(NULL, f));
		SAFE(write_int16(ni->linkcount, f));
		SAFE(write_int32(ni->flags, f));
		   SAFE(write_int32(ni->active, f));
	         SAFE(write_int32(ni->estado, f));
		SAFE(write_int16(ni->accesscount, f));
		for (j=0, access=ni->access; j<ni->accesscount; j++, access++)
		    SAFE(write_string(*access, f));
		SAFE(write_int16(ni->memos.memocount, f));
		SAFE(write_int16(ni->memos.memomax, f));
		memos = ni->memos.memos;
		for (j = 0; j < ni->memos.memocount; j++, memos++) {
		    SAFE(write_int32(memos->number, f));
		    SAFE(write_int16(memos->flags, f));
		    SAFE(write_int32(memos->time, f));
		    SAFE(write_buffer(memos->sender, f));
		    SAFE(write_string(memos->text, f));
		}
		SAFE(write_int16(ni->channelcount, f));
		SAFE(write_int16(ni->channelmax, f));
		SAFE(write_int16(ni->language, f));
	    }
	} /* for (ni) */
	SAFE(write_int8(0, f));
    } /* for (i) */
    close_db(f);
}

#undef SAFE

/*************************************************************************/

/* Check whether a user is on the access list of the nick they're using, or
 * if they're the same user who last identified for the nick.  If not, send
 * warnings as appropriate.  If so (and not NI_SECURE), update last seen
 * info.  Return 1 if the user is valid and recognized, 0 otherwise (note
 * that this means an NI_SECURE nick will return 0 from here unless the
 * user's timestamp matches the last identify timestamp).  If the user's
 * nick is not registered, 0 is returned.
 */

int validate_user(User *u)
{
    NickInfo *ni;
    int on_access;

    if (!(ni = u->real_ni))
	return 0;
	
    if (ni->status & NS_IDENTIFIED)
        return 1;
    if (!notifinouts) 
      if (ni->status & NI_ON_BDD && ((ni->active & ACTIV_CONFIRM) || (ni->active & ACTIV_FORZADO))) {
      	privmsg(s_NickServ, u->nick, " Has sido reconocido como propietario del nick. Recuerda que para una mayor seguridad, debes cambiar periodicamente tu clave con el comando 2 /msg nick set password. Si necesitas ayuda, contacta con nosotros en el canal 4#%s",CanalAyuda);
	privmsg(s_NickServ, u->nick, "Recuerda: Puedes encontrar cualquier tipo de ayuda sobre la Red,as� como foros de discusi�n y tutoriales en nuestra web 12%s",WebNetwork);
        return 1;
           }
     if (notifinouts) 
      if (ni->status & NI_ON_BDD && ((ni->active & ACTIV_CONFIRM) || (ni->active & ACTIV_FORZADO))) {
            return 1;
           }
if (ni->status & NI_ON_BDD && !(ni->active & ACTIV_CONFIRM) && !(ni->active & ACTIV_FORZADO) ) {
	struct tm *t1,*t2,*tm;
	int valor=0;
	time_t fechreg = ni->time_registered;
	time_t ns = NSRegMail; 
	time_t now = fechreg + ns;
	char buf[BUFSIZE];
	     tm = localtime(&now);
            strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
       privmsg(s_NickServ, u->nick, "Has sido reconocido por nick. Recuerda que a�n debes confirmar tu registro siguiendo las instrucciones enviadas a tu correo y que para ello tienes como fecha l�mite 2 %s. Si necesitas ayuda, contacta con nosotros en el canal 5#%s",buf,CanalAyuda);
 return 1; 
}

    if (ni->status & NS_VERBOTEN) {
	notice_lang(s_NickServ, u, NICK_MAY_NOT_BE_USED);
#if defined (IRC_HISPANO) || defined (IRC_TERRA)
	if (NSForceNickChange)
	    notice_lang(s_NickServ, u, FORCENICKCHANGE_IN_1_MINUTE);
	else
#endif
	    notice_lang(s_NickServ, u, DISCONNECT_IN_1_MINUTE);
	add_ns_timeout(ni, TO_COLLIDE, 60);
	return 0;
    }

    if (ni->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_SUSPENDED, ni->suspendreason);
        return 0;
    }
#if defined(OBSOLETO)
    if (!NoSplitRecovery) {
	/* XXX: This code should be checked to ensure it can't be fooled */
	if (ni->id_timestamp != 0 && u->signon == ni->id_timestamp) {
	    char buf[256];
	    snprintf(buf, sizeof(buf), "%s@%s", u->username, u->host);
	    if (strcmp(buf, ni->last_usermask) == 0) {
		ni->status |= NS_IDENTIFIED;
		return 1;
	    }
	}
    }   
#endif

    on_access = is_on_access(u, u->ni);
    if (on_access)
	ni->status |= NS_ON_ACCESS;

    if (!(u->ni->flags & NI_SECURE) && on_access) {
	ni->status |= NS_RECOGNIZED;
	ni->last_seen = time(NULL);
      	ni->expira_min = ni->last_seen + NSExpire;
	if (ni->last_usermask)
	    free(ni->last_usermask);
	ni->last_usermask = smalloc(strlen(u->username)+strlen(u->host)+2);
	sprintf(ni->last_usermask, "%s@%s", u->username, u->host);
	if (ni->last_realname)
	    free(ni->last_realname);
	ni->last_realname = sstrdup(u->realname);
	return 1;
    }

    if (on_access || !(u->ni->flags & NI_KILL_IMMED)) {
	if (u->ni->flags & NI_SECURE)
	    notice_lang(s_NickServ, u, NICK_IS_SECURE, s_NickServ);
	else
	    notice_lang(s_NickServ, u, NICK_IS_REGISTERED, s_NickServ);
    }

    if ((u->ni->flags & NI_KILLPROTECT) && !on_access) {
	if (u->ni->flags & NI_KILL_IMMED) {
	    collide(ni, 0);
	} else if (u->ni->flags & NI_KILL_QUICK) {
#if defined (IRC_HISPANO) || defined (IRC_TERRA)
	    if (NSForceNickChange)
	    	notice_lang(s_NickServ, u, FORCENICKCHANGE_IN_20_SECONDS);
	    else
#endif
	    	notice_lang(s_NickServ, u, DISCONNECT_IN_20_SECONDS);
	    add_ns_timeout(ni, TO_COLLIDE, 20);
	} else {
#if defined (IRC_HISPANO) || defined (IRC_TERRA)
	    if (NSForceNickChange)
	    	notice_lang(s_NickServ, u, FORCENICKCHANGE_IN_1_MINUTE);
	    else
#endif
	    	notice_lang(s_NickServ, u, DISCONNECT_IN_1_MINUTE);
	    add_ns_timeout(ni, TO_COLLIDE, 60);
	}
    }

    return 0;
}

/*************************************************************************/

/* Cancel validation flags for a nick (i.e. when the user with that nick
 * signs off or changes nicks).  Also cancels any impending collide. */

void cancel_user(User *u)
{
    NickInfo *ni = u->real_ni;
    if (ni) {

#if defined(IRC_TERRA)
	if (ni->status & NS_GUESTED) {
	    send_cmd(NULL, "NICK %s %ld 1 %s %s %s :%s Enforcement",
			u->nick, time(NULL), NSEnforcerUser, NSEnforcerHost, 
			ServerName, s_NickServ);
	    add_ns_timeout(ni, TO_RELEASE, NSReleaseTimeout);
	    ni->status &= ~NS_TEMPORARY;
	    ni->status |= NS_KILL_HELD;
	} else {
#endif
	    ni->status &= ~NS_TEMPORARY;
#if defined(IRC_TERRA)
	}
#endif
	del_ns_timeout(ni, TO_COLLIDE);
    }
}

/*************************************************************************/

/* Return whether a user has identified for their nickname. */

int nick_identified(User *u)
{
    return u->real_ni && (u->real_ni->status & NS_IDENTIFIED);
}

/*************************************************************************/

/* Return whether a user is recognized for their nickname. */

int nick_recognized(User *u)
{
    return u->real_ni && (u->real_ni->status & (NS_IDENTIFIED | NS_RECOGNIZED));
}
/*************************************************************************/

/* Nick suspendido. */

int nick_suspendied(User *u)
{
    return u->real_ni && (u->real_ni->status & NS_SUSPENDED);
}
    

/*************************************************************************/

/* Remove all nicks which have expired.  Also update last-seen time for all
 * nicks.
 */


void expire_nicks()
{
    User *u;
    NickInfo *ni, *next;
    int i;
    time_t now = time(NULL);
   time_t aviso,margen;
   aviso = dotime("5d"); /*cuanto debe faltar para mandarle un aviso*/
   margen = dotime("10d"); /* debe ser menor que nsexpire y mayor que el aviso*/

    /* Assumption: this routine takes less than NSExpire seconds to run.
     * If it doesn't, some users may end up with invalid user->ni pointers. */
    for (u = firstuser(); u; u = nextuser()) {
	if (u->real_ni) {
	    if (debug >= 2)
		log("debug: NickServ: updating last seen time for %s", u->nick);
	    u->real_ni->last_seen = time(NULL);
	}
    }
    if (!NSExpire)
	return;
    for (i = 0; i < aliases; i++) {
	for (ni = nicklists[i]; ni; ni = next) {
	    next = ni->next;
		

/*
*tratamiento del envio de mails a los nicks que est�n a punto de expirar
* (2009) donostiarra http://euskalirc.wordpress.com 
*/
		
             if (now - ni->last_seen >  (NSExpire -margen) &&  now - ni->last_seen <  (NSExpire -aviso)
			&& !(ni->status & (NS_VERBOTEN | NS_NO_EXPIRE | NS_SUSPENDED)) && !(ni->env_mail & ( MAIL_REC))) {
                      ni->env_mail |= MAIL_REC ;
             } else if (now - ni->last_seen >=  (NSExpire -aviso)
			&& !(ni->status & (NS_VERBOTEN | NS_NO_EXPIRE | NS_SUSPENDED)) && (ni->env_mail & ( MAIL_REC))) {
		canalopers(s_NickServ, "Queda menos de 1 semana para expirar  12%s (Le Enviamos email %s)", ni->nick,ni->email);
                char *buf;
                 char subject[BUFSIZE];
                  ni->env_mail &= ~MAIL_REC ;
                  //ni->env_mail |= MAIL_REC ;
              if (fork()==0) {
				 buf = smalloc(sizeof(char *) * 1024);
               sprintf(buf,"\n   Hola  NiCK: %s\n"
				"Queda  menos de una semana para expirar su nick.\n"
				"De no entrar con su alias identificado, en uno de nuestros servidores de Chat\n"
				"Nos Veremos Obligados a dar de baja su registro ,con lo que perder�a la antiguedad,\n"
				"As� como todos los recursos que ven�an aparejados al nick registrado.\n"
				"No nos olvides y sigue entrando en nuestra red de chat,le esperamos\n"
				"Disponemos de servicios exclusivos y un selecto grupo de canales.\n"
				"Le recordamos los datos que disponemos de su registro:\n\n" 
                             "Password: %s\n\n"
                             "Para identificarse   -> /nick %s:%s\n"
                             "Para cambio de clave -> /msg %s SET PASSWORD nueva_password\n\n"
                             "P�gina de Informaci�n %s\n",
                       ni->nick, ni->pass, ni->nick, ni->pass, s_NickServ, WebNetwork);
             
       
               snprintf(subject, sizeof(subject), "Recordatorio del NiCK '%s'", ni->nick);       
              enviar_correo(ni->email, subject, buf);
             exit(0);
         }
       }
            else if (now - ni->last_seen >=  (NSExpire)
			&& !(ni->status & (NS_VERBOTEN | NS_NO_EXPIRE | NS_SUSPENDED)) && !(ni->env_mail & ( MAIL_REC))) {
      		log("Expirando Nick %s", ni->nick);
		canalopers(s_NickServ, "El nick 12%s ha expirado", ni->nick);
		delnick(ni);
		#if defined(SOPORTE_MYSQL)
 MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_users where username ='%s';", ni->nick);

if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

     #endif

	    } else if (now - ni->time_registered >=  (NSRegMail) && !(ni->active & (ACTIV_CONFIRM | ACTIV_FORZADO))) { 
                      canalopers(s_NickServ, "El nick 12%s ha expirado por no activar 5%s", ni->nick,ni->email);
			 privmsg(s_NickServ, ni->nick ? ni->nick : u->nick, "Tu Nick 12%s ha expirado por no CONFIRMar 5%s ", ni->nick ? ni->nick : u->nick,ni->email);
			send_cmd(NULL, "RENAME %s", ni->nick ? ni->nick : u->nick);
		      delnick(ni); 
                }
	    /* AQUI EXPIRACION NICKS SUSPENDIDOS */
		}
}
}

/*************************************************************************/
/*************************************************************************/

/* Return the NickInfo structure for the given nick, or NULL if the nick
 * isn't registered. */

NickInfo *findnick(const char *nick)
{
    NickInfo *ni;

#if defined(IRC_UNDERNET)
/* A?adido soporte toLower, toUpper y strCasecmp para evitar conflictos
 * con nicks, debido a la arquitectura de undernet, que considera
 * como equivalentes los nicks [zoltan] y {zoltan} entre otros signos
 * As? los Services, lo considerar? como el mismo nick, impidiendo que
 * se pueda registrar {zoltan} existiendo [zoltan]
 *
 * Signos equivalentes
 * Min?sculas == May?sculas (esto ya estaba antes con tolower)
 * [ == {
 * ] == }
 * ^ == ~
 * \ == |
 *
 * Copiado codigo common.c y common.h del ircu de Undernet
 *
 * zoltan 1/11/2000
 */
    for (ni = nicklists[toLower(*nick)]; ni; ni = ni->next) {
        if (strCasecmp(ni->nick, nick) == 0)
            return ni;
    }

    for (ni = nicklists[toUpper(*nick)]; ni; ni = ni->next) {
        if (strCasecmp(ni->nick, nick) == 0)
            return ni;
    }
#else

    for (ni = nicklists[tolower(*nick)]; ni; ni = ni->next) {
	if (stricmp(ni->nick, nick) == 0)
	    return ni;
    }
#endif
    
    return NULL;
}

/*************************************************************************/

/* Return the "master" nick for the given nick; i.e., trace the linked list
 * through the `link' field until we find a nickname with a NULL `link'
 * field.  Assumes `ni' is not NULL.
 *
 * Note that we impose an arbitrary limit of 512 nested links.  This is to
 * prevent infinite loops in case someone manages to create a circular
 * link.  If we pass this limit, we arbitrarily cut off the link at the
 * initial nick.
 */

NickInfo *getlink(NickInfo *ni)
{
    NickInfo *orig = ni;
    int i = 0;

    while (ni->link && ++i < 512)
	ni = ni->link;
    if (i >= 512) {
	log("%s: Infinite loop(?) found at nick %s for nick %s, cutting link",
		s_NickServ, ni->nick, orig->nick);
	orig->link = NULL;
	ni = orig;
	/* FIXME: we should sanitize the data fields */
    }
    return ni;
}

/*************************************************************************/
/*********************** NickServ private routines ***********************/
/*************************************************************************/

/* Is the given user's address on the given nick's access list?  Return 1
 * if so, 0 if not. */

static int is_on_access(User *u, NickInfo *ni)
{
    int i;
    char *buf;

    if (ni->accesscount == 0)
	return 0;
    i = strlen(u->username);
    buf = smalloc(i + strlen(u->host) + 2);
    sprintf(buf, "%s@%s", u->username, u->host);
    strlower(buf+i+1);
    for (i = 0; i < ni->accesscount; i++) {
	if (match_wild_nocase(ni->access[i], buf)) {
	    free(buf);
	    return 1;
	}
    }
    free(buf);
    return 0;
}

/*************************************************************************/

/* Insert a nick alphabetically into the database. */

static void alpha_insert_nick(NickInfo *ni)
{
    NickInfo *ptr, *prev;
    char *nick = ni->nick;

    for (prev = NULL, ptr = nicklists[tolower(*nick)];
			ptr && stricmp(ptr->nick, nick) < 0;
			prev = ptr, ptr = ptr->next)
	;
    ni->prev = prev;
    ni->next = ptr;
    if (!prev)
	nicklists[tolower(*nick)] = ni;
    else
	prev->next = ni;
    if (ptr)
	ptr->prev = ni;
}

/*************************************************************************/

/* Add a nick to the database.  Returns a pointer to the new NickInfo
 * structure if the nick was successfully registered, NULL otherwise.
 * Assumes nick does not already exist.
 */

static NickInfo *makenick(const char *nick)
{
    NickInfo *ni;

    ni = scalloc(sizeof(NickInfo), 1);
    strscpy(ni->nick, nick, NICKMAX);
    alpha_insert_nick(ni);
    ni->status |= NI_ON_BDD;
    return ni;
   
}

/*************************************************************************/

/* Remove a nick from the NickServ database.  Return 1 on success, 0
 * otherwise.  Also deletes the nick from any ChanServ/OperServ lists it is
 * on.
 */

static int delnick(NickInfo *ni)
{
    int i;
    cs_remove_nick(ni);
    os_remove_nick(ni);

    if (ni->linkcount)
	remove_links(ni);
    if (ni->link)
	ni->link->linkcount--;
    if (ni->next)
	ni->next->prev = ni->prev;
    if (ni->prev)
	ni->prev->next = ni->next;
    else
	nicklists[tolower(*ni->nick)] = ni->next;
    if (ni->last_usermask)
	free(ni->last_usermask);
    if (ni->last_realname)
	free(ni->last_realname);
    if (ni->email)
        free (ni->email);	
    if (ni->vhost)
        free (ni->vhost);
    if (ni->url)
        free (ni->url);    
    if (ni->suspendby)
        free (ni->suspendby);
      if (ni->nickactoper)
        free (ni->nickactoper);
    if (ni->nickoper)
       free (ni->nickoper);
    if (ni->suspendreason)
        free (ni->suspendreason); 
 if (ni->motivo)
        free (ni->motivo); 
    
    if (ni->forbidby)
        free (ni->forbidby);        
    if (ni->forbidreason)
        free (ni->forbidreason);        
    if (ni->access) {
	for (i = 0; i < ni->accesscount; i++) {
	    if (ni->access[i])
		free(ni->access[i]);
	}
	free(ni->access);
    }
    if (ni->memos.memos) {
	for (i = 0; i < ni->memos.memocount; i++) {
	    if (ni->memos.memos[i].text)
		free(ni->memos.memos[i].text);
	}
	free(ni->memos.memos);
    }
    if (ni->status & NI_ON_BDD) {
          
           do_write_bdd(ni->nick, 15, "");
	do_write_bdd(ni->nick, 2, "");
	do_write_bdd(ni->nick, 3, "");
	do_write_bdd(ni->nick, 4, "");
    
    }

    free(ni);
    return 1;
}

/*************************************************************************/

/* Remove any links to the given nick (i.e. prior to deleting the nick).
 * Note this is currently linear in the number of nicks in the database--
 * that's the tradeoff for the nice clean method of keeping a single parent
 * link in the data structure.
 */

static void remove_links(NickInfo *ni)
{
    int i;
    NickInfo *ptr;

    for (i = 0; i < aliases; i++) {
	for (ptr = nicklists[i]; ptr; ptr = ptr->next) {
	    if (ptr->link == ni) {
		if (ni->link) {
		    ptr->link = ni->link;
		    ni->link->linkcount++;
		} else
		    delink(ptr);
	    }
	}
    }
}

/*************************************************************************/

/* Break a link from the given nick to its parent. */

static void delink(NickInfo *ni)
{
    NickInfo *link;

    link = ni->link;
    ni->link = NULL;
    do {
	link->channelcount -= ni->channelcount;
	if (link->link)
	    link = link->link;
    } while (link->link);
    ni->status = link->status;
    link->status &= ~NS_TEMPORARY;
    ni->flags = link->flags;
    ni->channelmax = link->channelmax;
    ni->memos.memomax = link->memos.memomax;
    ni->language = link->language;
    if (link->accesscount > 0) {
	char **access;
	int i;

	ni->accesscount = link->accesscount;
	access = smalloc(sizeof(char *) * ni->accesscount);
	ni->access = access;
	for (i = 0; i < ni->accesscount; i++, access++)
	    *access = sstrdup(link->access[i]);
    }
    link->linkcount--;
}

/*************************************************************************/
/*************************************************************************/

/* Collide a nick. 
 *
 * When connected to a network using DALnet servers, version 4.4.15 and above, 
 * Services is now able to force a nick change instead of killing the user. 
 * The new nick takes the form "Guest######". If a nick change is forced, we
 * do not introduce the enforcer nick until the user's nick actually changes. 
 * This is watched for and done in cancel_user(). -TheShadow 
 */

static void collide(NickInfo *ni, int from_timeout)
{
    User *u;
#if defined(IRC_UNDERNET_P10)
    int numeroa, numerob;    
#endif
    u = finduser(ni->nick);


#if defined(IRC_UNDERNET_P10)
    numkills++;
    
    if (numkills > 4095)
        numkills = 10;
    numeroa = (numkills / 64);
    numerob = numkills - (64 * numeroa);
#endif

    if (!from_timeout)
	del_ns_timeout(ni, TO_COLLIDE);

#if defined(IRC_TERRA)
    if (NSForceNickChange) {
	struct timeval tv;
	char guestnick[NICKMAX];

	gettimeofday(&tv, NULL);
	snprintf(guestnick, sizeof(guestnick), "%s%ld%ld", NSGuestNickPrefix,
			tv.tv_usec / 10000, tv.tv_sec % (60*60*24));

        notice_lang(s_NickServ, u, FORCENICKCHANGE_NOW, guestnick);

	send_cmd(NULL, "SVSNICK %s %s :%ld", ni->nick, guestnick, time(NULL));
	ni->status |= NS_GUESTED;
    } else {
#elif defined (IRC_HISPANO)
    if (NSForceNickChange) {
        send_cmd(ServerName, "RENAME %s", ni->nick);
    } else {
#endif                
	notice_lang(s_NickServ, u, DISCONNECT_NOW);
#if defined(IRC_UNDERNET_P10)
        
        kill_user(s_NickServ, u->numerico, "Protecci�n de Nick Registrado");
    	send_cmd(NULL, "%c NICK %s 1 %lu  %s %s AAAAAA %c%c%c :%s protegiendo a %s",
           convert2y[ServerNumerico], ni->nick, time(NULL), NSEnforcerUser,
           ServiceHost, convert2y[ServerNumerico], convert2y[numeroa],
           convert2y[numerob], s_NickServ, ni->nick); 
#else
        kill_user(s_NickServ, ni->nick, "Protecci�n de Nick Registrado");
        send_cmd(NULL, "NICK %s %ld 1 %s %s %s :%s protegiendo a %s",
                ni->nick, time(NULL), NSEnforcerUser, NSEnforcerHost,
                ServerName, s_NickServ, ni->nick);
#endif		
	ni->status |= NS_KILL_HELD;
	add_ns_timeout(ni, TO_RELEASE, NSReleaseTimeout);
#if defined (IRC_TERRA) || defined (IRC_HISPANO)
    }
#endif
}

/*************************************************************************/

/* Release hold on a nick. */

static void release(NickInfo *ni, int from_timeout)
{
    if (!from_timeout)
	del_ns_timeout(ni, TO_RELEASE);
    send_cmd(ni->nick, "QUIT");
    ni->status &= ~NS_KILL_HELD;
}

/*************************************************************************/
/*************************************************************************/

static struct my_timeout {
    struct my_timeout *next, *prev;
    NickInfo *ni;
    Timeout *to;
    int type;
} *my_timeouts;

/*************************************************************************/

/* Remove a collide/release timeout from our private list. */

static void rem_ns_timeout(NickInfo *ni, int type)
{
    struct my_timeout *t, *t2;

    t = my_timeouts;
    while (t) {
	if (t->ni == ni && t->type == type) {
	    t2 = t->next;
	    if (t->next)
		t->next->prev = t->prev;
	    if (t->prev)
		t->prev->next = t->next;
	    else
		my_timeouts = t->next;
	    free(t);
	    t = t2;
	} else {
	    t = t->next;
	}
    }
}

/*************************************************************************/

/* Collide a nick on timeout. */

static void timeout_collide(Timeout *t)
{
    NickInfo *ni = t->data;
    User *u;

    rem_ns_timeout(ni, TO_COLLIDE);
    /* If they identified or don't exist anymore, don't kill them. */
    if ((ni->status & NS_IDENTIFIED)
		|| !(u = finduser(ni->nick))
		|| u->my_signon > t->settime)
	return;
    /* The RELEASE timeout will always add to the beginning of the
     * list, so we won't see it.  Which is fine because it can't be
     * triggered yet anyway. */
    collide(ni, 1);
}

/*************************************************************************/

/* Release a nick on timeout. */

static void timeout_release(Timeout *t)
{
    NickInfo *ni = t->data;

    rem_ns_timeout(ni, TO_RELEASE);
    release(ni, 1);
}

/*************************************************************************/

/* Add a collide/release timeout. */

void add_ns_timeout(NickInfo *ni, int type, time_t delay)
{
    Timeout *to;
    struct my_timeout *t;
    void (*timeout_routine)(Timeout *);

    if (type == TO_COLLIDE)
	timeout_routine = timeout_collide;
    else if (type == TO_RELEASE)
	timeout_routine = timeout_release;
    else {
	log("NickServ: unknown timeout type %d!  ni=%p (%s), delay=%ld",
		type, ni, ni->nick, delay);
	return;
    }
    to = add_timeout(delay, timeout_routine, 0);
    to->data = ni;
    t = smalloc(sizeof(*t));
    t->next = my_timeouts;
    my_timeouts = t;
    t->prev = NULL;
    t->ni = ni;
    t->to = to;
    t->type = type;
}

/*************************************************************************/

/* Delete a collide/release timeout. */

static void del_ns_timeout(NickInfo *ni, int type)
{
    struct my_timeout *t, *t2;

    t = my_timeouts;
    while (t) {
	if (t->ni == ni && t->type == type) {
	    t2 = t->next;
	    if (t->next)
		t->next->prev = t->prev;
	    if (t->prev)
		t->prev->next = t->next;
	    else
		my_timeouts = t->next;
	    del_timeout(t->to);
	    free(t);
	    t = t2;
	} else {
	    t = t->next;
	}
    }
}

/*************************************************************************/
/*********************** NickServ command routines ***********************/
/*************************************************************************/

static void do_credits(User *u)
{
    notice_lang(s_NickServ, u, SERVICES_CREDITS);
}    

/*************************************************************************/    

/* Return a help message. */

static void do_help(User *u)
{
    char *cmd = strtok(NULL, "");

    if (!cmd) {
	if (NSExpire >= 86400)
	    notice_help(s_NickServ, u, NICK_HELP,Net,NSExpire/86400);
	else
	    notice_help(s_NickServ, u,NICK_HELP_EXPIRE_ZERO,Net);
	if (is_services_root(u))
	notice_help(s_NickServ, u, NICK_SERVDIRECTOR_HELP);
    	else if  (is_services_admin(u))
	notice_help(s_NickServ, u, NICK_SERVADMIN_HELP);
	else if  (is_services_cregadmin(u))
	notice_help(s_NickServ, u, NICK_SERVCOADMIN_HELP);
	else if  (is_services_devel(u))
	notice_help(s_NickServ, u, NICK_SERVDEVEL_HELP);
	else if  (is_services_oper(u))
	    notice_help(s_NickServ, u, NICK_SERVOPER_HELP);
           
	} else if (stricmp(cmd, "SET LANGUAGE") == 0) {
	int i;
	notice_help(s_NickServ, u, NICK_HELP_SET_LANGUAGE);
	for (i = 0; i < NUM_LANGS && langlist[i] >= 0; i++) {
#if defined(IRC_UNDERNET_P10)
	    privmsg(s_NickServ, u->numerico, "    %2d) %s",
#else
            privmsg(s_NickServ, u->nick, "    %2d) %s",
#endif	    
			i+1, langnames[langlist[i]]);
	}
    } else {
	help_cmd(s_NickServ, u, cmds, cmd);
    }
}
#if defined(SOPORTE_MYSQL)
void mirar_tablas() {
//Registrado (Registered), Autor (Autor), Editor (Editor) y Supervisor (Publisher).
// M�nager, Administrador y S�per-Administrador
static const char karak[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_{}[]";
   MYSQL *conn;
   MYSQL_RES *res,*resid,*residaro;
   MYSQL_ROW row,rowid,rowidaro;
   NickInfo *ni, *ni2;
   int i;
   char *cadena,*block;
   char borra[BUFSIZE];
   conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }

   /* send SQL query */
/*
      row[0] name
      row[1] username=nick
      row[2] password
      row[3] email
      row[4] gid
      row[5] block	*/


   if (mysql_query(conn, "select name,username,password,email,gid,block from jos_users")) {
      canaladmins( s_NickServ,"%s\n", mysql_error(conn));
     }

   res = mysql_use_result(conn);

   /* output table name */
/*Registramos los nicks de la web a IRC*/
   while ((row = mysql_fetch_row(res)) != NULL) {
	 if (!(ni = findnick(row[1])) && (strcmp("0", row[5]) == 0)) { 
		cadena = row[1];
		block=  row[5];
	
	/*chequeo aliases no permitidos*/
		if (cadena[strspn(cadena, karak)]) {
 canaladmins(s_NickServ, "El Nick 2%s Contiene Palabras No Permitidas ,No se registra y Se Borra De La Web",cadena);



 MYSQL *check;
char modifica[BUFSIZE];
 check = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(check, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_users where username ='%s';",cadena);
//canalopers(s_NickServ, "5%s eliminadose de tablas", nick);
if (mysql_query(check,modifica))  {
canaladmins(s_NickServ, "%s\n", mysql_error(check));
 mysql_close(check);
mysql_close(conn);
 return;
 }
return;
}
/* REG_NICK_MAIL */
         
		ni = makenick(row[1]);
		if (ni) {
	   	ni->status |= NI_ON_BDD;
		do_write_bdd(ni->nick, 1, row[2]);
		canalopers( s_NickServ,"5Registrado:12%s2(%s)",row[1], row[3]);
		 privmsg(s_NickServ, row[1], "El nick %s acaba de ser registrado en %s/index.php?option=com_user&task=register",row[1],WebNetwork);
		 privmsg(s_NickServ, row[1], "Si es su leg�timo propietario,identif�quese de nuevo");
		ni = findnick(ni->nick);
       ni->active &= ~ ACTIV_PROCESO;
		 ni->active |= ACTIV_FORZADO;
		ni->nickactoper = sstrdup("WEB");
	   send_cmd(NULL, "RENAME %s", row[1]);
	
	    ni->flags = 0;
	    ni->estado = 0;
             strscpy(ni->pass, row[2], PASSMAX);
               ni->email = sstrdup(row[3]);
	    if (NSDefKill)
		ni->flags |= NI_KILLPROTECT;
	    if (NSDefKillQuick)
		ni->flags |= NI_KILL_QUICK;
	    if (NSDefSecure)
		ni->flags |= NI_SECURE;
	    if (NSDefPrivate)
		ni->flags |= NI_PRIVATE;
	    if (NSDefHideEmail)
		ni->flags |= NI_HIDE_EMAIL;
	    /*if (NSDefHideUsermask)
		ni->flags |= NI_HIDE_MASK;*/
	    if (NSDefHideQuit)
		ni->flags |= NI_HIDE_QUIT;
	    if (NSDefMemoSignon)
		ni->flags |= NI_MEMO_SIGNON;
	    if (NSDefMemoReceive)
		ni->flags |= NI_MEMO_RECEIVE;
	    ni->memos.memomax = MSMaxMemos;
	    ni->channelcount = 0;
	    ni->channelmax = CSMaxReg;
	    ni->last_usermask = smalloc(strlen(row[1]));
	    sprintf(ni->last_usermask, "%s@%s", "webchat","*");
	    ni->last_realname = sstrdup(row[0]);
	    ni->time_registered = ni->last_seen = time(NULL);
	    ni->expira_min = ni->last_seen+NSExpire;
	    ni->language = DEF_LANGUAGE;
	    ni->link = NULL;
	
	    }
	}
     if (ni = findnick(row[1])) {

      /*
      row[0] name
      row[1] username=nick
      row[2] password
      row[3] email
      row[4] gid
			*/
	
         

         if (strcmp(ni->pass, row[2]) != 0) {
	canalopers( s_NickServ,"12WEBCHAT:4PassWord:3%s2(%s)",row[1], row[2]);
	 strscpy(ni->pass, row[2], PASSMAX);
	do_write_bdd(ni->nick, 1,ni->pass);
	 }

	 /* if (strcmp(ni->last_realname, row[0]) != 0) {
	canalopers( s_NickServ,"12WEBCHAT:4Nombre:3%s2(%s)",row[1], row[0]);
	  ni->last_realname = sstrdup(row[0]); }*/

	if (strcmp(ni->email, row[3]) != 0) {
	canalopers( s_NickServ,"12WEBCHAT:4email:3%s2(%s)",row[1], row[3]);
	  ni->email = sstrdup(row[3]); }



      }
	/* if (!(ni = findnick(row[1]))) {
		if (strcmp(ni->nick, row[1]) != 0) {
	canalopers( s_NickServ,"12WEBCHAT:4Borrado :3%s",row[1]);
	delnick(ni);
	 }*/

   


  
	   
}
mysql_free_result(res);

/*Registramos los nicks del IRC a la web (Registrando a tablas) <-- cuando desactivemos registro por web

char consulta[BUFSIZE],inserta[BUFSIZE],modifica[BUFSIZE],groups[BUFSIZE];
int cont;
cont=0;


 for (i = 0; i < aliases; i++)
        for (ni = nicklists[i]; ni; ni = ni->next) {
	      ni =findnick(ni->nick);
		snprintf(consulta, sizeof(consulta), "select username from jos_users where username ='%s'",ni->nick);
      if (mysql_query(conn,consulta)) 
				return;
		 res = mysql_use_result(conn);
              	while ((row = mysql_fetch_row(res)) != NULL) {
                           if (stricmp(row[0], ni->nick) == 0)
                               cont++;
                                }

                        if (cont) {
			   //canalopers( s_NickServ,"12 %s esta en tablas",ni->nick);
                           cont =0;
			   } else {
				/*registramos usuario que no esta en sql*/	
			 /*
                          id       int(11)
                          name     varchar(255)
			  username varchar(150)
			  email	   varchar(100)
			  password varchar(100)
			  usertype varchar(25)
			  block    tinyint (4)
			  sendEmail tinyint (4)
			  gid       tinyint (3) unsigned -->  18:Registrados; 19:autores ; 20:editores
			  registerDate datetime		      21:publicadores,23:gestores,24:Admins
			  lastvisitDate datetime
			  activation   varchar(100)		
			  params	text


insert into jos_users values(67,'guipuzcoano','donostiarra','admin.euskalirc@gmail.com','pass','Registered',
0,1,18,'2005-09-28 00:00:00','2007-09-28 00:00:00','','');

insert into jos_core_acl_aro values(15,'users','67',0,'guipuzcoano',0);

insert into jos_core_acl_groups_aro_map values(18,'',15);


			char buf[BUFSIZE];
			int id,idaro;
		
                            struct tm *tm,*te;
				User *u;
				tm = localtime(&ni->time_registered);
				strftime_lang(buf, sizeof(buf), u,STRFTIME_MESES_NUM, tm);
					//canalopers( s_NickServ,"5 %s NO esta en tablas",ni->nick);
		if (mysql_query(conn,"select id from jos_users;")) 
      				canaladmins(s_NickServ, "%s\n", mysql_error(conn));

				resid = mysql_use_result(conn);
                            id=60;idaro=15;
			 while ((rowid = mysql_fetch_row(resid)) != NULL) {
					id++; idaro++;
                              }
			
				canalopers( s_NickServ,"5VALOR Id es %d Idaro %d",id,idaro);
			
				canalopers( s_NickServ,"5Registrado a tablas:12%s2(%s)10<%s>",ni->nick, ni->email,ni->pass);
				snprintf(inserta, sizeof(inserta), "INSERT INTO jos_users VALUES('%d','%s','%s','%s','%s','%s','%d','%d','%d','%s','%d','%s','%s')", id,ni->nick,ni->nick,ni->email,ni->pass,"Registered",0,0,atol("18"), buf,ni->last_seen,"","\n");
				if (mysql_query(conn,inserta)) {
      				canaladmins(s_NickServ, "%s\n", mysql_error(conn));
				id++;
                                }

	
		snprintf(modifica, sizeof(modifica), "INSERT INTO jos_core_acl_aro VALUES('%d','%s','%d','%d','%s','%d')", idaro,"users",id,0,ni->last_realname,0);
				if (mysql_query(conn,modifica)) 
      				canaladmins(s_NickServ, "%s\n", mysql_error(conn));
      				
		snprintf(groups, sizeof(groups), "INSERT INTO jos_core_acl_groups_aro_map VALUES('%d','%s','%d')",atol("18"),"",idaro);
				if (mysql_query(conn,groups)) 
      				canaladmins(s_NickServ, "%s\n", mysql_error(conn));
      				
				
		                 }
              	       
	            }*/
 
      mysql_close(conn);
			
}
 
#endif
/*************************************************************************/

/* Register a nick. */

static void do_register(User *u)
{

    NickInfo *ni;
#if defined(REG_NICK_MAIL)
    char *email = strtok(NULL, " ");
    int i, nicksmail = 0;
#else
    char *pass = strtok(NULL, " ");
#endif

#if defined(SOPORTE_MYSQL)

 if (time(NULL) < u->lastnickreg + NSRegDelay) {
	notice_lang(s_NickServ, u, NICK_REG_PLEASE_WAIT, NSRegDelay);

    } else if (u->real_ni) {	/* i.e. there's already such a nick regged */
	if (u->real_ni->status & NS_VERBOTEN) {
	    log("%s: %s@%s tried to register FORBIDden nick %s", s_NickServ,
			u->username, u->host, u->nick);
	    notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
	} else {
	    notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
	}
}
notice(s_NickServ, u->nick, "4Comando deshabilitado, use 2%s/index.php?option=com_user&task=register ,para los registros de Nick", WebNetwork);
  return;
#endif
 /*   static char saltChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char salt[3];
    char * plaintext;
    *
    srandom(time(0));		 may not be the BEST salt, but its close
    salt[0] = saltChars[random() % 64];
    salt[1] = saltChars[random() % 64];
    salt[2] = 0;
    */
    if (readonly) {
	notice_lang(s_NickServ, u, NICK_REGISTRATION_DISABLED);
	return;
    }

#if defined (IRC_HISPANO) || defined (IRC_TERRA)
    /* Prevent "Guest" nicks from being registered. -TheShadow */
    if (NSForceNickChange) {
	int prefixlen = strlen(NSGuestNickPrefix);
	int nicklen = strlen(u->nick);

	/* A guest nick is defined as a nick...
	 * 	- starting with NSGuestNickPrefix
	 * 	- with a series of between, and including, 2 and 7 digits
	 * -TheShadow
	 */
	if (nicklen <= prefixlen+7 && nicklen >= prefixlen+2 &&
			stristr(u->nick, NSGuestNickPrefix) == u->nick &&
			strspn(u->nick+prefixlen, "1234567890") ==
							nicklen-prefixlen) {
	    notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
	    return;
	}
    }
#endif

#if defined(REG_NICK_MAIL)
    if (!email || (stricmp(email, u->nick) == 0 && strtok(NULL, " "))) {
	syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX);
#else
    if (!pass || (stricmp(pass, u->nick) == 0 && strtok(NULL, " "))) {
        syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX);
#endif
    } else if (time(NULL) < u->lastnickreg + NSRegDelay) {
	notice_lang(s_NickServ, u, NICK_REG_PLEASE_WAIT, NSRegDelay);

    } else if (u->real_ni) {	/* i.e. there's already such a nick regged */
	if (u->real_ni->status & NS_VERBOTEN) {
	    log("%s: %s@%s tried to register FORBIDden nick %s", s_NickServ,
			u->username, u->host, u->nick);
	    notice_lang(s_NickServ, u, NICK_CANNOT_BE_REGISTERED, u->nick);
	} else {
	    notice_lang(s_NickServ, u, NICK_ALREADY_REGISTERED, u->nick);
	}
#if defined(REG_NICK_MAIL)
    } else if (
          !strchr(email,'@') ||
               strchr(email,'@') != strrchr(email,'@') ||
                   !strchr(email,'.') ||
                               strchr(email,'|') ) {
        notice_lang(s_NickServ, u, NICK_MAIL_INVALID);
        syntax_error(s_NickServ, u, "REGISTER", NICK_REGISTER_SYNTAX);
    } else {
        strlower(email);
        for(i=0; i < aliases; i++)
          for (ni = nicklists[i]; ni; ni = ni->next)
             if(ni->email && !strcmp(email,ni->email))
                nicksmail++;
     if (nicksmail >= NicksMail) {
            notice_lang(s_NickServ, u, NICK_MAIL_ABUSE, NicksMail);

           return;
        }
#else
    } else if (stricmp(u->nick, pass) == 0
            || (StrictPasswords && strlen(pass) < 5)) {
        notice_lang(s_NickServ, u, MORE_OBSCURE_PASSWORD);
    } else {

#endif
	ni = makenick(u->nick);
		
	if (ni) {





#if defined(REG_NICK_MAIL)
/*** Registro de nicks por mail by Zoltan ***/

            char pass[255];
            static char saltChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	    char salt[7];
	    int cnt = 0;
	    unsigned long long xi;
	    salt[6] = '\0';
            __asm__ volatile (".byte 0x0f, 0x31" : "=A" (xi));
	    srandom(xi);

	    for(cnt = 0; cnt < 6; ++cnt)
		  salt[cnt] = saltChars[random() % 36];

            sprintf(pass,"%s",salt);

            strscpy(ni->pass, pass, PASSMAX);
            ni->email = sstrdup(email);

 char clave[255];
            static char saltLetras[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]";
	    char salt2[13];
	    cnt = 0;
	    unsigned long long xj;
	    salt2[12] = '\0';
            __asm__ volatile (".byte 0x0f, 0x31" : "=A" (xj));
	    srandom(xj);

	    for(cnt = 0; cnt < 12; ++cnt)
		  salt2[cnt] = saltLetras[random() % 64];

            sprintf(clave,"%s",salt2);

            strscpy(ni->clave, clave, PASSMAX);



            {
            char *buf;
            char subject[BUFSIZE];                        
            if (fork()==0) {

               buf = smalloc(sizeof(char *) * 1024);
	      if (NSRegMail) {
		sprintf(buf,"\n   NiCK: %s\n"
                             "Password Temporal: %s\n\n"
                             "Para identificarte   -> /nick %s:%s\n\n"
                       	     "Para poder seguir usando esta informaci�n ,cuando te identifiques,\n"
			     "debes completar el registro de tu nick,y para ello s�lo basta que\n"
			     "confirmes que has leido este correo, y as� tu Password ya dejar� de ser "
			     "temporal.\n\n"
			     "Para confirmar haz /msg NiCK CONFIRM %s\n\n"
			     "Despues de confirmar,recuerda que para una mayor seguridad,\n"
			     "y una vez identificado, debes cambiar periodicamente tu Password.\n\n"
			     "Para cambio de Password -> /msg %s SET PASSWORD nueva_password\n\n"
			      "P�gina de Informaci�n %s\n",
                       ni->nick, ni->pass, ni->nick, ni->pass, ni->clave,s_NickServ, WebNetwork); }
		else {
               sprintf(buf,"\n    NiCK: %s\n"
                             "Password: %s\n\n"
                             "Para identificarte   -> /nick %s:%s\n"
                             "Para cambio de Password -> /msg %s SET PASSWORD nueva_password\n\n"
                             "P�gina de Informaci�n %s\n",
                       ni->nick, ni->pass, ni->nick, ni->pass, s_NickServ, WebNetwork);
			}
       
               snprintf(subject, sizeof(subject), "Registro del NiCK '%s'", ni->nick);
		if (NSRegMail) 
		notice_lang(s_NickServ, u, NICK_DONE, u->nick, ni->email);
                else {
		notice_lang(s_NickServ, u, NICK_REGISTERED, u->nick, ni->email);
                 }
	
            	//notice_lang(s_NickServ, u, NICK_IN_MAIL);
                    #if defined(IRC_UNDERNET_P09)
		if (NSRegMail) {
			if (NSRegMail >= 86400) {
                      notice_lang(s_NickServ, u, NICK_PASSWORD_TEMP, NSRegMail/86400,ni->pass,ni->nick,ni->pass);
			} else if (NSRegMail < 86400 & NSRegMail >= 3600) {
			   notice_lang(s_NickServ, u, NICK_PASSWORD_TEMP_HORAS, NSRegMail/3600,ni->pass,ni->nick,ni->pass);
			 
                           } else if (NSRegMail < 3600) {
				 notice_lang(s_NickServ, u, NICK_PASSWORD_TEMP_MINUTOS, NSRegMail/60,ni->pass,ni->nick,ni->pass);
				}
                       else
			notice_lang(s_NickServ, u, NICK_BDD_NEW_REG,ni->pass, ni->nick, ni->pass);
			}
		
                   /*en colores*/
             /*  privmsg(s_NickServ, ni->nick, "Su clave es %s. Recuerdela por si no le llega notificacion al correo. Use /nick %s:%s para identificarse.",ni->pass, ni->nick, ni->pass);*/
	       send_cmd(NULL, "RENAME %s", ni->nick);
		
		ni->status |= NI_ON_BDD;
		do_write_bdd(ni->nick, 1, ni->pass);
		 if (NSRegMail)
			canalopers( s_NickServ,"5Registro-Temporal:12%s2(%s)",ni->nick,ni->email);
		else
		canalopers( s_NickServ,"5Registrado:12%s2(%s)",ni->nick,ni->email);
             	  	#endif	       
                 #if defined(IRC_UNDERNET_P10)
		/*notice_lang(s_NickServ, u, NICK_IN_MAIL);
		 notice_lang(s_NickServ, u, NICK_BDD_NEW_REG,ni->pass, ni->nick, ni->pass);
                ep_tablan(ni->nick, ni->pass, 'n');
		/* privmsg(s_NickServ, u->numerico, "Su clave es %s. Recuerdela por si no le llega notificacion al correo. Use /nick %s:%s para identificarse.",ni->pass, ni->nick, ni->pass);*/
		send_cmd(NULL, "%c RENAME :%s", convert2y[ServerNumerico], ni->nick);
                 #endif
  enviar_correo(ni->email, subject, buf);
		             
               exit(0);
            }
           }                                                                                                                                                                      

#else	
#if defined(USE_ENCRYPTION)
	    int len = strlen(pass);
	    if (len > PASSMAX) {
		len = PASSMAX;
		pass[len] = 0;
		notice_lang(s_NickServ, u, PASSWORD_TRUNCATED, PASSMAX);
	    }
	    if (encrypt(pass, len, ni->pass, PASSMAX) < 0) {
		memset(pass, 0, strlen(pass));
		log("%s: Failed to encrypt password for %s (register)",
			s_NickServ, u->nick);
		notice_lang(s_NickServ, u, NICK_REGISTRATION_FAILED);
		return;
	    }
	    memset(pass, 0, strlen(pass));
	    ni->status = NS_ENCRYPTEDPW | NS_IDENTIFIED | NS_RECOGNIZED;
#else
	    if (strlen(pass) > PASSMAX-1) /* -1 for null byte */
		notice_lang(s_NickServ, u, PASSWORD_TRUNCATED, PASSMAX-1);
	    strscpy(ni->pass, pass, PASSMAX);
	    ni->status = NS_IDENTIFIED | NS_RECOGNIZED;
#endif
#endif /* REG_NICK_MAIL */

	    ni->flags = 0;
	    ni->estado = 0;
	    if (NSDefKill)
		ni->flags |= NI_KILLPROTECT;
	    if (NSDefKillQuick)
		ni->flags |= NI_KILL_QUICK;
	    if (NSDefSecure)
		ni->flags |= NI_SECURE;
	    if (NSDefPrivate)
		ni->flags |= NI_PRIVATE;
	    if (NSDefHideEmail)
		ni->flags |= NI_HIDE_EMAIL;
	    /*if (NSDefHideUsermask)
		ni->flags |= NI_HIDE_MASK;*/
	    if (NSDefHideQuit)
		ni->flags |= NI_HIDE_QUIT;
	    if (NSDefMemoSignon)
		ni->flags |= NI_MEMO_SIGNON;
	    if (NSDefMemoReceive)
		ni->flags |= NI_MEMO_RECEIVE;
	    ni->memos.memomax = MSMaxMemos;
	    ni->channelcount = 0;
	    ni->channelmax = CSMaxReg;
	    ni->last_usermask = smalloc(strlen(u->username)+strlen(u->host)+2);
	    sprintf(ni->last_usermask, "%s@%s", u->username, u->host);
	    ni->last_realname = sstrdup(u->realname);
	    ni->time_registered = ni->last_seen = time(NULL);
	   ni->expira_min = ni->last_seen+NSExpire;
	    ni->accesscount = 1;
	    ni->access = smalloc(sizeof(char *));
	    ni->access[0] = create_mask(u);
	    ni->language = DEF_LANGUAGE;
	    ni->link = NULL;
	    u->ni = u->real_ni = ni;
	   if (NSRegMail) {
		ni->active &= ~ACTIV_CONFIRM;
		 ni->active |= ACTIV_PROCESO;
}
else {
ni->active &= ~ACTIV_PROCESO;
ni->active |= ACTIV_CONFIRM;
}

#if defined(REG_NICK_MAIL)
	 if (NSRegMail) {
	log("%s: %s' registered by %s@%s Email: %s Pass: %s", s_NickServ,
                   u->nick, u->username, u->host, ni->email, ni->pass); 
         }
         else {               
            notice_lang(s_NickServ, u, NICK_REGISTERED, u->nick, ni->email);
		
            log("%s: %s' registered by %s@%s Email: %s Pass: %s", s_NickServ,
                   u->nick, u->username, u->host, ni->email, ni->pass);                
            notice_lang(s_NickServ, u, NICK_REGISTERED, u->nick, ni->email);
            notice_lang(s_NickServ, u, NICK_IN_MAIL);
           }
  
#else                                            
	    log("%s: `%s' registered by %s@%s", s_NickServ,
			u->nick, u->username, u->host);
	    notice_lang(s_NickServ, u, NICK_REGISTERED, u->nick, ni->access[0]);
	    
#ifndef USE_ENCRYPTION
	    notice_lang(s_NickServ, u, NICK_PASSWORD_IS, ni->pass);
#endif
#endif
	    u->lastnickreg = time(NULL);
#if defined (IRC_DAL4_4_15) || defined (IRC_BAHAMUT)
	    send_cmd(ServerName, "SVSMODE %s +r", u->nick);
#endif
	} else {
	    log("%s: makenick(%s) failed", s_NickServ, u->nick);
	    notice_lang(s_NickServ, u, NICK_REGISTRATION_FAILED);
	}

    }

}

/*************************************************************************/

static void do_identify(User *u)
{
    char *pass = strtok(NULL, " ");
    NickInfo *ni;
    int res;

    if (!pass) {
	syntax_error(s_NickServ, u, "IDENTIFY", NICK_IDENTIFY_SYNTAX);

    } else if (!(ni = u->real_ni)) {
        privmsg(s_NickServ, u->nick, "Tu nick no est� registrado");
    } else if (ni->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_SUSPENDED, ni->suspendreason);
    } else if (!(res = check_password(pass, ni->pass))) {
	log("%s: Failed IDENTIFY for %s!%s@%s",
		s_NickServ, u->nick, u->username, u->host);
	notice_lang(s_NickServ, u, PASSWORD_INCORRECT);
	bad_password(u);

    } else if (res == -1) {
	notice_lang(s_NickServ, u, NICK_IDENTIFY_FAILED);

    } else {
        if (ni->status & NS_IDENTIFIED)
           notice_lang(s_NickServ, u, NICK_IS_IDENTIFIED);
	ni->id_timestamp = u->signon;
	if (!(ni->status & NS_RECOGNIZED)) {
	    ni->last_seen = time(NULL);
	   ni->expira_min = ni->last_seen+NSExpire;
	    if (ni->last_usermask)
		free(ni->last_usermask);
	    ni->last_usermask = smalloc(strlen(u->username)+strlen(u->host)+2);
	    sprintf(ni->last_usermask, "%s@%s", u->username, u->host);
	    if (ni->last_realname)
		free(ni->last_realname);
	    ni->last_realname = sstrdup(u->realname);
	}
#if defined (IRC_TERRA)
	send_cmd(ServerName, "SVSMODE %s +r", u->nick);
#endif
	log("%s: %s!%s@%s identified for nick %s", s_NickServ,
			u->nick, u->username, u->host, u->nick);
        if (!(ni->status & NS_IDENTIFIED))
          notice_lang(s_NickServ, u, NICK_IDENTIFY_SUCCEEDED);
        ni->status |= NS_IDENTIFIED;
	if (!(ni->status & NS_RECOGNIZED))
	    check_memos(u);
	strcpy(ni->nick, u->nick);    

    }
}
/*************************************************************************/

static void do_validar(User *u)
{
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
    User *u2;

    /*if (readonly && !is_services_cregadmin(u)) {
	notice_lang(s_NickServ, u, NICK_DROP_DISABLED);
	return;
    }*/
    if (!nick) {
	syntax_error(s_NickServ, u, "VALIDAR", NICK_VALIDAR_SYNTAX);
	return;
	}

    if (!is_services_admin(u) && nick) {
	syntax_error(s_NickServ, u, "VALIDAR", NICK_VALIDAR_SYNTAX);

    } else if (!(ni = (nick ? findnick(nick) : u->real_ni))) {
	if (nick)
	    notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
	else
	    notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);

    } else if ( nick && (nick_is_services_admin(ni) 
            && !is_services_root(u))) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	
    } else if (!nick && !nick_identified(u)) {
	notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);

    } else if ((ni->active & ACTIV_CONFIRM) || (ni->active & ACTIV_FORZADO)) {
        notice_lang(s_NickServ, u, NICK_ACTIVADO,nick);
            

    } else {
	if (readonly)
            notice_lang(s_NickServ, u, READ_ONLY_MODE);
                #if defined(IRC_UNDERNET_P10)
		ni->active &= ~ ACTIV_PROCESO;
		 ni->active |= ACTIV_FORZADO;
		 ni->nickactoper = sstrdup(u->nick);

		#endif

	canalopers(s_NickServ, "12%s activ� el nick 12%s", u->nick, nick);
	log("%s: %s!%s@%s activated nickname %s", s_NickServ,
		u->nick, u->username, u->host, nick ? nick : u->nick);
	     ni->active &= ~ ACTIV_PROCESO;
		 ni->active |= ACTIV_FORZADO;
		ni->nickactoper = sstrdup(u->nick);
	if (nick)
	    notice_lang(s_NickServ, u, NICK_X_ACTIVADO, nick);
	else
	    notice_lang(s_NickServ, u, NICK_ACTIVE);

    }
}
/*************************************************************************/

static void do_drop(User *u)
{
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
    User *u2;

    if (readonly && !is_services_cregadmin(u)) {
	notice_lang(s_NickServ, u, NICK_DROP_DISABLED);
	return;
    }

    if (!is_services_cregadmin(u) && nick) {
	syntax_error(s_NickServ, u, "DROP", NICK_DROP_SYNTAX);

    } else if (!(ni = (nick ? findnick(nick) : u->real_ni))) {
	if (nick)
	    notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
	else
	    notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);

    } else if (NSSecureAdmins && nick && (nick_is_services_admin(ni) 
            || nick_is_services_oper(ni)) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	
    } else if (!nick && !nick_identified(u)) {
	notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);

    } else if (ni->status & NS_VERBOTEN) {
        notice_lang(s_NickServ, u, NICK_DROP_FORBIDDEN);
            

    } else {
	if (readonly)
            notice_lang(s_NickServ, u, READ_ONLY_MODE);
                #if defined(IRC_UNDERNET_P10)
		 ed_tablan(ni->nick, 0, 'n');
		#endif

#if defined (IRC_TERRA)
	send_cmd(ServerName, "SVSMODE %s -r", ni->nick);
#endif
	canalopers(s_NickServ, "12%s elimin� el nick 12%s", u->nick, nick);
	log("%s: %s!%s@%s dropped nickname %s", s_NickServ,
		u->nick, u->username, u->host, nick ? nick : u->nick);
	send_cmd(NULL, "RENAME %s", nick ? nick : u->nick);
	delnick(ni);

	if (nick) 
	    notice_lang(s_NickServ, u, NICK_X_DROPPED, nick);
	else
	    notice_lang(s_NickServ, u, NICK_DROPPED);
	if (nick && (u2 = finduser(nick)))
	    u2->ni = u2->real_ni = NULL;
	else if (!nick)
	    u->ni = u->real_ni = NULL;
	#if defined(SOPORTE_MYSQL)
 MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_users where username ='%s';",nick ? nick : u->nick);
//canalopers(s_NickServ, "5%s eliminadose de tablas", nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

     #endif
    }
}
/******************MARCADO DE UN NICK *********/
static void do_marcar(User *u)
{
   char *nick = strtok(NULL, " ");
    char *razon = strtok(NULL, "");
    
     NickInfo *ni;

    if (!razon) {
        privmsg(s_NickServ, u->nick, "Sintaxis: 12MARCAR <Nick> 2 <motivo>");
    } else if  (!(ni = findnick(nick))) {
        privmsg(s_NickServ, u->nick, "El Nick  12%s no registrado", nick);
   
    } else if ((ni->estado & NS_MARCADO))  {
        privmsg(s_NickServ, u->nick, "El Nick 12%s Ya est� MARCADO ", nick);
     } else if ((ni->status &  NS_SUSPENDED))  {
        privmsg(s_NickServ, u->nick, "El Nick  12%s est� SUSPENDIDO",nick );
      /*si esta suspendido el nick,no necesario marcarlo porque ya tiene su razon*/
     } else {
      
      ni->motivo = sstrdup(razon);
       ni->time_motivo = time(NULL);
       ni->nickoper = sstrdup(u->nick);
        ni->estado = 0;
        ni->estado |= NS_MARCADO;
        privmsg(s_NickServ, u->nick, "Al Nick 12%s se le ha MARCADO", nick);
        canaladmins(s_NickServ, "12%s ha MARCADO el Nick  12%s Motivo:5%s", u->nick, nick,ni->motivo); 
    }
}
/******************DESMARCAR UN CANAL *********/
static void do_desmarcar(User *u)
{
   char *nick = strtok(NULL, " ");
      
     NickInfo *ni;
  
    if (!nick) {
        privmsg(s_NickServ, u->nick, "Sintaxis: 12DESMARCAR <Nick>");
        return;
      }
    
    if (!(ni = findnick(nick))) {
        privmsg(s_NickServ, u->nick, "El Nick 12%s no est� registrado", nick);
         return;
        
    } else if (!(ni->estado & NS_MARCADO))  {
        privmsg(s_NickServ, u->nick, "El nickl 12%s no est� MARCADO", nick);
     
     } else {
        if (ni->motivo)
	free(ni->motivo);
        ni->nickoper = sstrdup(u->nick);
        ni->estado  &= ~NS_MARCADO;
        
        privmsg(s_NickServ, u->nick, "Al nick 12%s se le ha DESMARCADO", nick);
        canaladmins(s_NickServ, "12%s ha DESMARCADO el nick  12%s", u->nick, nick); 
    }
}
/*************************************************************************/

static void do_set(User *u)
{
    char *cmd    = strtok(NULL, " ");
    char *param  = strtok(NULL, " ");
    NickInfo *ni;
    int is_servdevel = is_services_devel(u);

    int set_nick = 0;

    if (readonly) {
	notice_lang(s_NickServ, u, NICK_SET_DISABLED);
	return;
    }

    if (is_servdevel && cmd && (ni = findnick(cmd))) {
	 if (nick_is_services_root(ni)) {
            notice_lang(s_NickServ, u, PERMISSION_DENIED);
            return;
        }
        if (nick_is_services_admin(ni) && !is_services_root(u)) {
            notice_lang(s_NickServ, u, PERMISSION_DENIED);
            return;
        }
	if (nick_is_services_cregadmin(ni) && !is_services_admin(u) && !is_services_root(u)) {
            notice_lang(s_NickServ, u, PERMISSION_DENIED);
            return;
        }
	if (nick_is_services_devel(ni) && !is_services_cregadmin(u) && !is_services_admin(u) && !is_services_root(u)) {
            notice_lang(s_NickServ, u, PERMISSION_DENIED);
            return;
        }
                                        
	cmd = param;
	param = strtok(NULL, " ");
	set_nick = 1;
    } else {
	ni = u->ni;
    }
    if (!param && (!cmd || (stricmp(cmd,"URL")!=0 && stricmp(cmd,"EMAIL")!=0))){
	if (is_servdevel) {
	    syntax_error(s_NickServ, u, "SET", NICK_SET_SERVADMIN_SYNTAX);
	} else {
	    syntax_error(s_NickServ, u, "SET", NICK_SET_SYNTAX);
	}
	//notice_lang(s_NickServ, u, MORE_INFO, s_NickServ, "SET");
    } else if (!ni) {
	notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
    } else if (!is_servdevel && !nick_identified(u)) {
	notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
    } else if (stricmp(cmd, "PASSWORD") == 0) {
	do_set_password(u, set_nick ? ni : u->real_ni, param);
    } else if (stricmp(cmd, "PASS") == 0) {
        do_set_password(u, set_nick ? ni : u->real_ni, param);            	
    } else if (stricmp(cmd, "LANGUAGE") == 0) {
	do_set_language(u, ni, param);
    } else if (stricmp(cmd, "URL") == 0) {
	do_set_url(u, set_nick ? ni : u->real_ni, param);
    } else if ((stricmp(cmd, "EMAIL") == 0) && is_services_admin(u)) {
	do_set_email(u, set_nick ? ni : u->real_ni, param);
     } else if ((stricmp(cmd, "TIMEREG") == 0) && is_services_admin(u)) {
	do_set_timeregister(u, set_nick ? ni : u->real_ni, param);
    }/* else if (stricmp(cmd, "KILL") == 0) {
	do_set_kill(u, ni, param);
    } else if (stricmp(cmd, "SECURE") == 0) {
	do_set_secure(u, ni, param);
    }*/ else if (stricmp(cmd, "PRIVATE") == 0) {
	do_set_private(u, ni, param);
    } else if (stricmp(cmd, "HIDE") == 0) {
	do_set_hide(u, ni, param);
    } else if (stricmp(cmd, "NOEXPIRE") == 0) {
	do_set_noexpire(u, ni, param);
    } else if (stricmp(cmd, "BDD") == 0) {
        do_set_bdd(u, ni, param);
    } else if (stricmp(cmd, "VHOST") == 0) {
        do_set_vhost(u, ni, param);
	
    } else {
	if (is_servdevel)
	    notice_lang(s_NickServ, u, NICK_SET_UNKNOWN_OPTION_OR_BAD_NICK,
			strupper(cmd));
	else
	    notice_lang(s_NickServ, u, NICK_SET_UNKNOWN_OPTION, strupper(cmd));
    }
}

/*************************************************************************/

static void do_getnewpass(User *u)
{
            NickInfo *ni;
	    char pass[255]; 
            static char saltChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]";
	    char salt[13];
	    unsigned long long xi;
	    int cnt = 0;
	    salt[12] = '\0';
		__asm__ volatile (".byte 0x0f, 0x31" : "=A" (xi));

	    ni = u->ni;

	    srandom(xi);

	    for(cnt = 0; cnt < 12; ++cnt)
		  salt[cnt] = saltChars[random() % 64];

            sprintf(pass,"%s",salt);
                                    
            strscpy(ni->pass, salt, PASSMAX);
	    privmsg(s_NickServ, u->nick, "Generada clave de alta calidad para tu nick. La clave es %s, recuerdala", ni->pass);
	   
	  if (ni->status & NI_ON_BDD) 
		do_write_bdd(ni->nick, 1, ni->pass);
		
		

}

static void do_set_password(User *u, NickInfo *ni, char *param)
{
    int len = strlen(param);

    if (NSSecureAdmins && u->real_ni != ni && nick_is_services_admin(ni) && 
    							!is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	return;
    } else if (stricmp(ni->nick, param) == 0 || (StrictPasswords && len < 5)) {
	notice_lang(s_NickServ, u, MORE_OBSCURE_PASSWORD);
	return;
    }

#if defined(USE_ENCRYPTION)
    if (len > PASSMAX) {
	len = PASSMAX;
	param[len] = 0;
	notice_lang(s_NickServ, u, PASSWORD_TRUNCATED, PASSMAX);
    }
    if (encrypt(param, len, ni->pass, PASSMAX) < 0) {
	memset(param, 0, strlen(param));
	log("%s: Failed to encrypt password for %s (set)",
		s_NickServ, ni->nick);
	notice_lang(s_NickServ, u, NICK_SET_PASSWORD_FAILED);
	return;
    }
    memset(param, 0, strlen(param));
    notice_lang(s_NickServ, u, NICK_SET_PASSWORD_CHANGED);
#else
    if (strlen(param) > PASSMAX-1) /* -1 for null byte */
	notice_lang(s_NickServ, u, PASSWORD_TRUNCATED, PASSMAX-1);
    strscpy(ni->pass, param, PASSMAX);
    notice_lang(s_NickServ, u, NICK_SET_PASSWORD_CHANGED_TO, ni->pass);
  
    if (ni->status & NI_ON_BDD)
   #if defined(IRC_UNDERNET_P10)
      ep_tablan(ni->nick, ni->pass, 'n');
     privmsg(s_NickServ,u->numerico, "Para identificarse haga 2,15/nick %s!%s",ni->nick,ni->pass);
   #else
   do_write_bdd(ni->nick, 1, ni->pass);
   #endif

#if defined(SOPORTE_MYSQL)
 MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set password ='%s' where username ='%s';",  ni->pass,ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

     #endif
#endif
    if (u->real_ni != ni) {
        canalopers(s_NickServ, "12%s  ha usado SET PASSWORD sobre 12%s",
         u->nick, ni->nick);
	log("%s: %s!%s@%s used SET PASSWORD as Services admin on %s",
		s_NickServ, u->nick, u->username, u->host, ni->nick);	
    }
}

/*************************************************************************/

static void do_set_language(User *u, NickInfo *ni, char *param)
{
    int langnum;

    if (param[strspn(param, "0123456789")] != 0) {  /* i.e. not a number */
	syntax_error(s_NickServ, u, "SET LANGUAGE", NICK_SET_LANGUAGE_SYNTAX);
	return;
    }
    langnum = atoi(param)-1;
    if (langnum < 0 || langnum >= NUM_LANGS || langlist[langnum] < 0) {
	notice_lang(s_NickServ, u, NICK_SET_LANGUAGE_UNKNOWN,
		langnum+1, s_NickServ);
	return;
    }
    ni->language = langlist[langnum];
    notice_lang(s_NickServ, u, NICK_SET_LANGUAGE_CHANGED);
}

/*************************************************************************/

static void do_set_url(User *u, NickInfo *ni, char *param)
{
    if (ni->url)
	free(ni->url);
    if (param) {
	ni->url = sstrdup(param);
	notice_lang(s_NickServ, u, NICK_SET_URL_CHANGED, param);
    } else {
	ni->url = NULL;
	notice_lang(s_NickServ, u, NICK_SET_URL_UNSET);
    }
}

/*************************************************************************/



static void do_set_vhost(User *u, NickInfo *ni, char *param)
{
    if (!is_services_oper(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	return;
    }
    
    if (!(ni->status & NI_ON_BDD)) {
         notice_lang(s_NickServ, u, NICK_MUST_BE_ON_BDD);
	 return;
    }
     if (!param) {
		    privmsg(s_NickServ, u->nick, "V-Host sin Rellenar  Minimo 3 caracteres");
		    return;
	}
    
    if (strlen(param) > 56) {
		    privmsg(s_NickServ, u->nick, "V-Host demasiado larga. M�ximo 56 caracteres");
		    return;
	}
    if (strlen(param) <= 2) {
		    privmsg(s_IpVirtual, u->nick, "V-Host demasiado corta. M�nimo 3 caracteres");
		    return;
	}
		    
    if (stricmp(param, "OFF") == 0) {
         #if defined(IRC_UNDERNET_P09)
          
	 do_write_bdd(ni->nick, 4, "");
	#endif
	 notice_lang(s_NickServ, u, NICK_SET_VHOST_OFF);
    } else {
        strcat(param , ".virtual");
	ni->vhost=sstrdup(param);
	#if defined(IRC_UNDERNET_P09)
        do_write_bdd(ni->nick, 4, param);
	#endif
	notice_lang(s_NickServ, u, NICK_SET_VHOST_ON, param);
    }
}
	
static void do_set_email(User *u, NickInfo *ni, char *param)
{
   if (!is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	return;
    }
    if (ni->email)
	free(ni->email);
    if (param) {
	ni->email = sstrdup(param);
	notice_lang(s_NickServ, u, NICK_SET_EMAIL_CHANGED, param);
    } else {
	ni->email = NULL;
	notice_lang(s_NickServ, u, NICK_SET_EMAIL_UNSET);
    }
#if defined(SOPORTE_MYSQL)
 MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_NickServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "update jos_users set email='%s' where username ='%s';",  ni->email,ni->nick);
if (mysql_query(conn,modifica)) 
canaladmins(s_NickServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

     #endif
}

static void do_set_timeregister(User *u, NickInfo *ni, char *param)
{  
		 if (!is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	return;
    }
 
    if (param) {
	ni->time_registered = ni->time_registered + dotime(param);
	notice_lang(s_NickServ, u, NICK_SET_TIMEREG_CHANGED, param);
    } 
}
/*************************************************************************/

static void do_set_kill(User *u, NickInfo *ni, char *param)
{
    if (stricmp(param, "ON") == 0) {
	ni->flags |= NI_KILLPROTECT;
	ni->flags &= ~(NI_KILL_QUICK | NI_KILL_IMMED);
	notice_lang(s_NickServ, u, NICK_SET_KILL_ON);
    } else if (stricmp(param, "QUICK") == 0) {
	ni->flags |= NI_KILLPROTECT | NI_KILL_QUICK;
	ni->flags &= ~NI_KILL_IMMED;
	notice_lang(s_NickServ, u, NICK_SET_KILL_QUICK);
    } else if (stricmp(param, "IMMED") == 0) {
	if (NSAllowKillImmed) {
	    ni->flags |= NI_KILLPROTECT | NI_KILL_IMMED;
	    ni->flags &= ~NI_KILL_QUICK;
	    notice_lang(s_NickServ, u, NICK_SET_KILL_IMMED);
	} else {
	    notice_lang(s_NickServ, u, NICK_SET_KILL_IMMED_DISABLED);
	}
    } else if (stricmp(param, "OFF") == 0) {
	ni->flags &= ~(NI_KILLPROTECT | NI_KILL_QUICK | NI_KILL_IMMED);
	notice_lang(s_NickServ, u, NICK_SET_KILL_OFF);
    } else {
	syntax_error(s_NickServ, u, "SET KILL",
		NSAllowKillImmed ? NICK_SET_KILL_IMMED_SYNTAX
		                 : NICK_SET_KILL_SYNTAX);
    }
}

/*************************************************************************/

static void do_set_secure(User *u, NickInfo *ni, char *param)
{
    if (stricmp(param, "ON") == 0) {
	ni->flags |= NI_SECURE;
	notice_lang(s_NickServ, u, NICK_SET_SECURE_ON);
    } else if (stricmp(param, "OFF") == 0) {
	ni->flags &= ~NI_SECURE;
	notice_lang(s_NickServ, u, NICK_SET_SECURE_OFF);
    } else {
	syntax_error(s_NickServ, u, "SET SECURE", NICK_SET_SECURE_SYNTAX);
    }
}

/*************************************************************************/

static void do_set_private(User *u, NickInfo *ni, char *param)
{
    if (stricmp(param, "ON") == 0) {
	ni->flags |= NI_PRIVATE;
	notice_lang(s_NickServ, u, NICK_SET_PRIVATE_ON);
    } else if (stricmp(param, "OFF") == 0) {
	ni->flags &= ~NI_PRIVATE;
	notice_lang(s_NickServ, u, NICK_SET_PRIVATE_OFF);
    } else {
	syntax_error(s_NickServ, u, "SET PRIVATE", NICK_SET_PRIVATE_SYNTAX);
    }
}

/*************************************************************************/

static void do_set_hide(User *u, NickInfo *ni, char *param)
{
    int flag, onmsg, offmsg;

    if (stricmp(param, "EMAIL") == 0) {
	flag = NI_HIDE_EMAIL;
	onmsg = NICK_SET_HIDE_EMAIL_ON;
	offmsg = NICK_SET_HIDE_EMAIL_OFF;
    } /*else if (stricmp(param, "USERMASK") == 0) {
	flag = NI_HIDE_MASK;
	onmsg = NICK_SET_HIDE_MASK_ON;
	offmsg = NICK_SET_HIDE_MASK_OFF;
    }*/ else if (stricmp(param, "QUIT") == 0) {
	flag = NI_HIDE_QUIT;
	onmsg = NICK_SET_HIDE_QUIT_ON;
	offmsg = NICK_SET_HIDE_QUIT_OFF;
    } else {
	syntax_error(s_NickServ, u, "SET HIDE", NICK_SET_HIDE_SYNTAX);
	return;
    }
    param = strtok(NULL, " ");
    if (!param) {
	syntax_error(s_NickServ, u, "SET HIDE", NICK_SET_HIDE_SYNTAX);
    } else if (stricmp(param, "ON") == 0) {
	ni->flags |= flag;
	notice_lang(s_NickServ, u, onmsg, s_NickServ);
    } else if (stricmp(param, "OFF") == 0) {
	ni->flags &= ~flag;
	notice_lang(s_NickServ, u, offmsg, s_NickServ);
    } else {
	syntax_error(s_NickServ, u, "SET HIDE", NICK_SET_HIDE_SYNTAX);
    }
}

/*************************************************************************/

static void do_set_noexpire(User *u, NickInfo *ni, char *param)
{
    if (!is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	return;
    }
    if (!param) {
	syntax_error(s_NickServ, u, "SET NOEXPIRE", NICK_SET_NOEXPIRE_SYNTAX);
	return;
    }
    if (stricmp(param, "ON") == 0) {
	ni->status |= NS_NO_EXPIRE;
	notice_lang(s_NickServ, u, NICK_SET_NOEXPIRE_ON, ni->nick);
    } else if (stricmp(param, "OFF") == 0) {
	ni->status &= ~NS_NO_EXPIRE;
	notice_lang(s_NickServ, u, NICK_SET_NOEXPIRE_OFF, ni->nick);
    } else {
	syntax_error(s_NickServ, u, "SET NOEXPIRE", NICK_SET_NOEXPIRE_SYNTAX);
    }
}

/*************************************************************************/

static void do_set_bdd(User *u, NickInfo *ni, char *param)
{
     if (!is_services_admin(u)) {
          notice_lang(s_NickServ, u, PERMISSION_DENIED);
	  return;
     }

     if (!param) {
         syntax_error(s_NickServ, u, "SET BDD", NICK_SET_BDD_SYNTAX);
	 return;
     }
     
     if (nick_is_services_oper(ni)) {
     	 notice_lang(s_NickServ, u, ACCESS_DENIED);
	 return;
     }

     if (stricmp(param, "ON") == 0) {
         ni->status |= NI_ON_BDD;
	 notice_lang(s_NickServ, u, NICK_SET_BDD_ON, ni->nick);
	#if defined(IRC_UNDERNET_P09)
	privmsg(s_NickServ,u->nick, "Para identificarse haga 2,15/nick %s!%s",ni->nick,ni->pass);
     	 do_write_bdd(ni->nick, 1, ni->pass);
	#else
	privmsg(s_NickServ,u->numerico, "Para identificarse haga 2,15/nick %s!%s",ni->nick,ni->pass);
        ep_tablan(ni->nick, ni->pass, 'n');
         #endif
     } else if (stricmp(param, "OFF") == 0) {
         ni->status &= ~NI_ON_BDD;
	 notice_lang(s_NickServ, u, NICK_SET_BDD_OFF, ni->nick);
	#if defined(IRC_UNDERNET_P09)
	 do_write_bdd(ni->nick, 15, "");
	 do_write_bdd(ni->nick, 2, "");
	 do_write_bdd(ni->nick, 3, "");
	 do_write_bdd(ni->nick, 4, "");
	#endif
         #if defined(IRC_UNDERNET_P10)
         ed_tablan(ni->nick, 0, 'n');
         #endif
     } else {
         syntax_error(s_NickServ, u, "SET BDD", NICK_SET_BDD_SYNTAX);
     }
}
static void do_userip(User *u)
{
    char *nick = strtok(NULL, " ");
    User *u2;
    struct hostent *hp;
    //struct in_addr inaddr;

   if (!nick) {
     	syntax_error(s_NickServ,u, "USERIP", NICK_USERIP_SYNTAX);
     } else if (!(u2 = finduser(nick))) {
     	  notice_lang(s_NickServ, u, NICK_USERIP_CHECK_NO, nick);
     } else {
	  if ((hp=gethostbyname(u2->host)) == NULL) {
	  privmsg(s_NickServ, u->nick, "Fallo al intentar resolver %s", u2->host);
	  return;
	  }
	  notice_lang(s_NickServ, u, NICK_USERIP_CHECK_OK, nick, u2->host, inet_ntoa(*((struct in_addr *)hp->h_addr)));
	  canalopers(s_NickServ, "12%s us� USERIP sobre 12%s.",u->nick, nick);
     }
}


/**************************************************************************/ 
      


static void do_access(User *u)
{
    char *cmd = strtok(NULL, " ");
    char *mask = strtok(NULL, " ");
    NickInfo *ni;
    int i;
    char **access;

    if (cmd && stricmp(cmd, "LIST") == 0 && mask && is_services_oper(u)
			&& (ni = findnick(mask))) {
	ni = getlink(ni);
	notice_lang(s_NickServ, u, NICK_ACCESS_LIST_X, mask);
	mask = strtok(NULL, " ");
	for (access = ni->access, i = 0; i < ni->accesscount; access++, i++) {
	    if (mask && !match_wild(mask, *access))
		continue;
#if defined(IRC_UNDERNET_P10)
	    privmsg(s_NickServ, u->numerico, "    %s", *access);
#else
            notice(s_NickServ, u->nick, "    %s", *access);
#endif	    
	}

    } else if (!cmd || ((stricmp(cmd,"LIST")==0) ? !!mask : !mask)) {
	syntax_error(s_NickServ, u, "ACCESS", NICK_ACCESS_SYNTAX);

    } else if (mask && !strchr(mask, '@')) {
	notice_lang(s_NickServ, u, BAD_USERHOST_MASK);
	notice_lang(s_NickServ, u, MORE_INFO, s_NickServ, "ACCESS");

    } else if (!(ni = u->ni)) {
	notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);

    } else if (!nick_identified(u)) {
	notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);

    } else if (stricmp(cmd, "ADD") == 0) {
	if (ni->accesscount >= NSAccessMax) {
	    notice_lang(s_NickServ, u, NICK_ACCESS_REACHED_LIMIT, NSAccessMax);
	    return;
	}
	for (access = ni->access, i = 0; i < ni->accesscount; access++, i++) {
	    if (strcmp(*access, mask) == 0) {
		notice_lang(s_NickServ, u,
			NICK_ACCESS_ALREADY_PRESENT, *access);
		return;
	    }
	}
	ni->accesscount++;
	ni->access = srealloc(ni->access, sizeof(char *) * ni->accesscount);
	ni->access[ni->accesscount-1] = sstrdup(mask);
	notice_lang(s_NickServ, u, NICK_ACCESS_ADDED, mask);

    } else if (stricmp(cmd, "DEL") == 0) {
	/* First try for an exact match; then, a case-insensitive one. */
	for (access = ni->access, i = 0; i < ni->accesscount; access++, i++) {
	    if (strcmp(*access, mask) == 0)
		break;
	}
	if (i == ni->accesscount) {
	    for (access = ni->access, i = 0; i < ni->accesscount;
							access++, i++) {
		if (stricmp(*access, mask) == 0)
		    break;
	    }
	}
	if (i == ni->accesscount) {
	    notice_lang(s_NickServ, u, NICK_ACCESS_NOT_FOUND, mask);
	    return;
	}
	notice_lang(s_NickServ, u, NICK_ACCESS_DELETED, *access);
	free(*access);
	ni->accesscount--;
	if (i < ni->accesscount)	/* if it wasn't the last entry... */
	    memmove(access, access+1, (ni->accesscount-i) * sizeof(char *));
	if (ni->accesscount)		/* if there are any entries left... */
	    ni->access = srealloc(ni->access, ni->accesscount * sizeof(char *));
	else {
	    free(ni->access);
	    ni->access = NULL;
	}

    } else if (stricmp(cmd, "LIST") == 0) {
	notice_lang(s_NickServ, u, NICK_ACCESS_LIST);
	for (access = ni->access, i = 0; i < ni->accesscount; access++, i++) {
	    if (mask && !match_wild(mask, *access))
		continue;
	    privmsg(s_NickServ, u->nick, "    %s", *access);
	}

    } else {
	syntax_error(s_NickServ, u, "ACCESS", NICK_ACCESS_SYNTAX);

    }
}

/*************************************************************************/

/*static void do_link(User *u)
{
    char *nick = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    NickInfo *ni = u->real_ni, *target;
    int res;

    if (NSDisableLinkCommand) {
	notice_lang(s_NickServ, u, NICK_LINK_DISABLED);
	return;
    }

    if (!pass) {
	syntax_error(s_NickServ, u, "LINK", NICK_LINK_SYNTAX);

    } else if (!ni) {
	notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);

    } else if (!nick_identified(u)) {
	notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);

    } else if (!(target = findnick(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);

    } else if (target == ni) {
	notice_lang(s_NickServ, u, NICK_NO_LINK_SAME, nick);

    } else if (target->status & NS_VERBOTEN) {
	notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, nick);

    } else if (target->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_X_SUSPENDED, nick);           

    } else if (!(res = check_password(pass, target->pass))) {
	log("%s: LINK: bad password for %s by %s!%s@%s",
		s_NickServ, nick, u->nick, u->username, u->host);
	notice_lang(s_NickServ, u, PASSWORD_INCORRECT);
	bad_password(u);

    } else if (res == -1) {
	notice_lang(s_NickServ, u, NICK_LINK_FAILED);

    } else {
	NickInfo *tmp;*/

	/* Make sure they're not trying to make a circular link */
	/*for (tmp = target; tmp; tmp = tmp->link) {
	    if (tmp == ni) {
		notice_lang(s_NickServ, u, NICK_LINK_CIRCULAR, nick);
		return;
	    }
	}*/

	/* If this nick already has a link, break it */
	/*if (ni->link)
	    delink(ni);

	ni->link = target;
	target->linkcount++;
	do {
	    target->channelcount += ni->channelcount;
	    if (target->link)
		target = target->link;
	} while (target->link);
	if (ni->access) {
	    int i;
	    for (i = 0; i < ni->accesscount; i++) {
		if (ni->access[i])
		    free(ni->access[i]);
	    }
	    free(ni->access);
	    ni->access = NULL;
	    ni->accesscount = 0;
	}
	if (ni->memos.memos) {
	    int i, num;
	    Memo *memo;
	    if (target->memos.memos) {
		num = 0;
		for (i = 0; i < target->memos.memocount; i++) {
		    if (target->memos.memos[i].number > num)
			num = target->memos.memos[i].number;
		}
		num++;
		target->memos.memos = srealloc(target->memos.memos,
			sizeof(Memo) * (ni->memos.memocount +
			                target->memos.memocount));
	    } else {
		num = 1;
		target->memos.memos = smalloc(sizeof(Memo)*ni->memos.memocount);
		target->memos.memocount = 0;
	    }
	    memo = target->memos.memos + target->memos.memocount;
	    for (i = 0; i < ni->memos.memocount; i++, memo++) {
		*memo = ni->memos.memos[i];
		memo->number = num++;
	    }
	    target->memos.memocount += ni->memos.memocount;
	    ni->memos.memocount = 0;
	    free(ni->memos.memos);
	    ni->memos.memos = NULL;
	    ni->memos.memocount = 0;
	}
	u->ni = target;
	notice_lang(s_NickServ, u, NICK_LINKED, nick);*/
	/* They gave the password, so they might as well have IDENTIFY'd */
	/*target->status |= NS_IDENTIFIED;
    }
}*/

/*************************************************************************/

/*static void do_unlink(User *u)
{
    NickInfo *ni;
    char *linkname;
    char *nick = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    int res = 0;

    if (nick) {
	int is_servadmin = is_services_admin(u);
	ni = findnick(nick);
	if (!ni) {
	    notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
	} else if (!ni->link) {
	    notice_lang(s_NickServ, u, NICK_X_NOT_LINKED, nick);
	} else if (!is_servadmin && !pass) {
	    syntax_error(s_NickServ, u, "UNLINK", NICK_UNLINK_SYNTAX);
	} else if (!is_servadmin &&
				!(res = check_password(pass, ni->pass))) {
	    log("%s: LINK: bad password for %s by %s!%s@%s",
		s_NickServ, nick, u->nick, u->username, u->host);
	    notice_lang(s_NickServ, u, PASSWORD_INCORRECT);
	    bad_password(u);
	} else if (res == -1) {
	    notice_lang(s_NickServ, u, NICK_UNLINK_FAILED);
	} else {
	    linkname = ni->link->nick;
	    delink(ni);
	    notice_lang(s_NickServ, u, NICK_X_UNLINKED, ni->nick, linkname);*/
	    /* Adjust user record if user is online */
	    /* FIXME: probably other cases we need to consider here */
	    /*for (u = firstuser(); u; u = nextuser()) {
		if (u->real_ni == ni) {
		    u->ni = ni;
		    break;
		}
	    }
	}
    } else {
	ni = u->real_ni;
	if (!ni)
	    notice_lang(s_NickServ, u, NICK_NOT_REGISTERED);
	else if (!nick_identified(u))
	    notice_lang(s_NickServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
	else if (!ni->link)
	    notice_lang(s_NickServ, u, NICK_NOT_LINKED);
	else {
	    linkname = ni->link->nick;
	    u->ni = ni;*/  /* Effective nick now the same as real nick */
	    /*delink(ni);
	    notice_lang(s_NickServ, u, NICK_UNLINKED, linkname);
	}
    }
}*/

/*************************************************************************/

/*static void do_listlinks(User *u)
{
    char *nick = strtok(NULL, " ");
    char *param = strtok(NULL, " ");
    NickInfo *ni, *ni2;
    int count = 0, i;

    if (!nick || (param && stricmp(param, "ALL") != 0)) {
	syntax_error(s_NickServ, u, "LISTLINKS", NICK_LISTLINKS_SYNTAX);

    } else if (!(ni = findnick(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);

    } else if (ni->status & NS_VERBOTEN) {
	notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, ni->nick);

    } else {
	notice_lang(s_NickServ, u, NICK_LISTLINKS_HEADER, ni->nick);
	if (param)
	    ni = getlink(ni);
	for (i = 0; i < aliases; i++) {
	    for (ni2 = nicklists[i]; ni2; ni2 = ni2->next) {
		if (ni2 == ni)
		    continue;
		if (param ? getlink(ni2) == ni : ni2->link == ni) {
#ifdef IRC_UNDERNET_P10
		    privmsg(s_NickServ, u->numerico, "    %s", ni2->nick);
#else
                    privmsg(s_NickServ, u->nick, "    %s", ni2->nick);
#endif
		    count++;
		}
	    }
	}
	notice_lang(s_NickServ, u, NICK_LISTLINKS_FOOTER, count);
    }
}*/


/* Show hidden info to nick owners and sadmins when the "ALL" parameter is
 * supplied. If a nick is online, the "Last seen address" changes to "Is 
 * online from".
 * Syntax: INFO <nick> {ALL}
 * -TheShadow (13 Mar 1999)
 */

static void do_info(User *u)
{
    char *nick = strtok(NULL, " ");
    char *param = strtok(NULL, " ");
    NickInfo *ni, *ni2, *real;
    int is_servoper = is_services_oper(u);
    int i;

    if (!nick) {
    	syntax_error(s_NickServ, u, "INFO", NICK_INFO_SYNTAX);
    } else if (is_a_service(nick)) {
    	notice_lang(s_NickServ, u, NICK_IS_A_SERVICE, nick);
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (ni->status & NS_VERBOTEN) {
	notice_lang(s_NickServ, u, NICK_X_FORBIDDEN, nick);
        privmsg(s_NickServ, u->nick, "Motivo: 12%s", ni->forbidreason);
        if (is_services_oper(u))
            privmsg(s_NickServ, u->nick, "Forbideado por: 12%s", ni->forbidby);

    } else {
	struct tm *tm;
	char buf[BUFSIZE], *end;
	const char *commastr = getstring(u->ni, COMMA_SPACE);
	int need_comma = 0;
	int nick_online = 0;
	int show_hidden = 0;

	/* Is the real owner of the nick we're looking up online? -TheShadow */
	if (ni->status & NS_IDENTIFIED)
	    nick_online = 0;

        /* Only show hidden fields to owner and sadmins and only when the ALL
	 * parameter is used. -TheShadow */
        if (param && stricmp(param, "ALL") == 0 &&
			((nick_online && (stricmp(u->nick, nick) == 0)) ||
                        	is_services_oper(u)))
            show_hidden = 1;

	real = getlink(ni);

        if(stricmp(ni->nick,real->nick)!=0)
            notice_lang(s_NickServ, u, NICK_INFO_LINKED, real->nick);


 if ((ni->estado & NS_MARCADO)   && (is_services_admin(u) ||  is_services_cregadmin(u) || is_services_devel(u) ||  is_services_oper(u))) {
           privmsg(s_NickServ, u->nick, "5MARCADO2(Por 3 %s)   con 5Motivo : 2%s",ni->nickoper,ni->motivo);
                     char timebuf[32];
//                time_t now = time(NULL);            
                 tm = localtime(&ni->time_motivo);
                strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
                privmsg(s_NickServ, u->nick, "5Fecha del MARCADO: 2 %s", timebuf);

            }  
        



        if (ni->status & NS_SUSPENDED) {
            notice_lang(s_NickServ, u, NICK_INFO_SUSPENDED, ni->suspendreason);
            if (show_hidden) {
                char timebuf[32], expirebuf[256];
//                time_t now = time(NULL);            
                privmsg(s_NickServ, u->nick, "Suspendido por: 12%s", ni->suspendby);
                tm = localtime(&ni->time_suspend);
                strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
                privmsg(s_NickServ, u->nick, "Fecha de la suspensi�n: 12%s", timebuf);
                if (ni->time_expiresuspend == 0) {
                    snprintf(expirebuf, sizeof(expirebuf),
                           getstring(u->ni, OPER_AKILL_NO_EXPIRE));
/***
                } else if (ni->time_expiresuspend <= now) {
                    snprintf(expirebuf, sizeof(expirebuf),
                              getstring(u->ni, OPER_AKILL_EXPIRES_SOON));
                } else {
                    expires_in_lang(expirebuf, sizeof(expirebuf), u,
                              ni->time_expiresuspend - now + 59);
  */            }                           
                privmsg(s_NickServ, u->nick, "La suspensi�n expira en: 12%s", expirebuf);
            }  
                                                            
        }    
	notice_lang(s_NickServ, u, NICK_INFO_REALNAME,
		ni->nick, ni->last_realname);
	
	tm = localtime(&ni->time_registered);
	strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
	notice_lang(s_NickServ, u, NICK_INFO_TIME_REGGED, buf);

	if ((!is_services_oper(u)) && !(ni->active & ACTIV_CONFIRM) && !(ni->active & ACTIV_FORZADO)) {
		 privmsg(s_NickServ, u->nick, "Estado:4,15Este nick est� en proceso de registro");
		if (!is_services_oper(u)) return;
    }
	
	if (nick_online) {
		 if (is_services_admin(u))
		   notice_lang(s_NickServ, u, NICK_INFO_ADDRESS_ONLINE, ni->last_usermask);
	} else {
		if (is_services_admin(u))
			notice_lang(s_NickServ, u, NICK_INFO_ADDRESS, ni->last_usermask);
	}
	if ((ni->active & ACTIV_CONFIRM) || (ni->active & ACTIV_FORZADO)) {
         ni->expira_min =  ni->last_seen + NSExpire;
	
          if (!(ni->status & NS_NO_EXPIRE)  && (stricmp(ni->nick, u->nick) == 0)  && !(is_services_admin(u) &&  is_services_cregadmin(u) && is_services_devel(u) &&  is_services_oper(u))) {
		     tm = localtime(&ni->expira_min);
            strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
            privmsg(s_NickServ,u->nick, "Tiempo M�nimo Expiraci�n:4,15%s", buf);
	   }	
             else if (!(ni->status & NS_NO_EXPIRE)    && (is_services_admin(u) ||  is_services_cregadmin(u) || is_services_devel(u) ||  is_services_oper(u))) {
		     tm = localtime(&ni->expira_min);
            strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
            privmsg(s_NickServ,u->nick, "Tiempo M�nimo Expiraci�n:4,15%s", buf);
	   }	
	}
	if (ni && (ni->status & NS_IDENTIFIED)) 
		 notice_lang(s_NickServ, u, NICK_INFO_ADDRESS_ONLINE_NOHOST,
			ni->nick); 
	else {
		tm = localtime(&ni->last_seen);
            strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
            notice_lang(s_NickServ, u, NICK_INFO_LAST_SEEN, buf);
 }
	  /*			
#ifdef DB_NETWORKS
            if (show_hidden)
#else
	    if (show_hidden || !(real->flags & NI_HIDE_MASK))
#endif	    
		 notice_lang(s_NickServ, u, NICK_INFO_ADDRESS_ONLINE,
			ni->last_usermask); 
	    else
		 notice_lang(s_NickServ, u, NICK_INFO_ADDRESS_ONLINE_NOHOST,
			ni->nick); 

	}  else {
#ifdef DB_NETWORKS	
	    if (show_hidden)
#else
            if (show_hidden || !(real->flags & NI_HIDE_MASK))
#endif	    
		notice_lang(s_NickServ, u, NICK_INFO_ADDRESS,
			ni->last_usermask);
*/
            
//	} 	    
	  

/***/
	
	if ((ni->last_quit || (is_services_oper(u)) && (show_hidden  || !(real->flags & NI_HIDE_QUIT))) && !(ni->status & NS_SUSPENDED))
	    notice_lang(s_NickServ, u, NICK_INFO_LAST_QUIT, ni->last_quit);
	if (ni->url)
	    notice_lang(s_NickServ, u, NICK_INFO_URL, ni->url);
	if (ni->email && (show_hidden || !(real->flags & NI_HIDE_EMAIL) || (is_services_oper(u)))) {
		  if (ni->active & ACTIV_CONFIRM) 
	    notice_lang(s_NickServ, u, NICK_INFO_EMAIL, ni->email);
	    else if (ni->active & ACTIV_FORZADO) {
			notice_lang(s_NickServ, u, NICK_INFO_EMAIL_FORZADO, ni->email,ni->nickactoper);
               }  else notice_lang(s_NickServ, u, NICK_INFO_EMAIL_INVALID, ni->email);
	}

         if ((ni->vhost) && !(ni->vhost==NULL))
	 privmsg(s_NickServ,u->nick, "VHOST:%s", ni->vhost);
        
        /*if (is_services_oper(u))  {    
	     if (ni->active & ACTIV_CONFIRM) 
	    notice_lang(s_NickServ, u, NICK_INFO_EMAIL, ni->email);
	    else if (ni->active & ACTIV_FORZADO) {
			notice_lang(s_NickServ, u, NICK_INFO_EMAIL_FORZADO, ni->email,ni->nickactoper);
               }  else notice_lang(s_NickServ, u, NICK_INFO_EMAIL_INVALID, ni->email);
	}*/

/*	if (ni->status & NI_ON_BDD)
	    notice_lang(s_NickServ, u, NICK_INFO_OPT_BDD);
*/

	/* if ((u->status & is_services_oper) && !(u->status & is_services_admin)
	    privmsg(s_NickServ, u->nick, "Es un OPERador de la RED"); */
	if (nick_is_services_oper(ni) && !(stricmp(ni->nick, u->nick) == 0))
	    privmsg(s_NickServ, ni->nick, "%s ha utilizado %s INFO sobre ti", u->nick, s_NickServ);
	    
           if (nick_is_services_patrocina(ni) && !nick_is_services_oper(ni))
	    notice_lang(s_NickServ, u, NICK_INFO_SERV_PATROCINA);

	    if (nick_is_services_oper(ni)  && !nick_is_services_devel(ni))
	    notice_lang(s_NickServ, u, NICK_INFO_SERV_OPER);
	    
	 	
	 if (nick_is_services_devel(ni)  && !nick_is_services_cregadmin(ni))
	    notice_lang(s_NickServ, u, NICK_INFO_SERV_DEVEL);

	 if (nick_is_services_cregadmin(ni) && !nick_is_services_admin(ni))
	    notice_lang(s_NickServ, u, NICK_INFO_SERV_CREGADMIN);
	    
	if (nick_is_services_admin(ni))
	    notice_lang(s_NickServ, u, NICK_INFO_SERV_ADMIN);
	
	  for (i = 0; i <RootNumber ; i++) {
	if (stricmp(nick, ServicesRoots[i]) ==0) {
	    notice_lang(s_NickServ, u, NICK_INFO_SERV_DIRECTOR,Net);
		if (!(stricmp(ni->nick, u->nick) == 0) && !(nick_is_services_admin(ni)))
	    privmsg(s_NickServ, ni->nick, "%s ha utilizado %s INFO sobre ti", u->nick, s_NickServ);
		}
	   }
	*buf = 0;
	end = buf;
	if (real->flags & NI_KILLPROTECT) {
	    end += snprintf(end, sizeof(buf)-(end-buf), "%s",
			getstring(u->ni, NICK_INFO_OPT_KILL));
	    need_comma = 1;
	}
	if (real->flags & NI_SECURE) {
	    end += snprintf(end, sizeof(buf)-(end-buf), "%s%s",
			need_comma ? commastr : "",
			getstring(u->ni, NICK_INFO_OPT_SECURE));
	    need_comma = 1;
	}
	if (real->flags & NI_ON_BDD) {
	    end += snprintf(end, sizeof(buf)-(end-buf), "%s%s", need_comma ? commastr : "", getstring(u->ni, NICK_INFO_OPT_BDD));
	    need_comma = 1;
	}
	
	if (real->flags & NI_PRIVATE) {
	    end += snprintf(end, sizeof(buf)-(end-buf), "%s%s",
			need_comma ? commastr : "",
			getstring(u->ni, NICK_INFO_OPT_PRIVATE));
	    need_comma = 1;
	}
	notice_lang(s_NickServ, u, NICK_INFO_OPTIONS,
		*buf ? buf : getstring(u->ni, NICK_INFO_OPT_NONE));

	
	    
	if ((ni->status & NS_NO_EXPIRE) && (real == u->ni || is_servoper))
	    notice_lang(s_NickServ, u, NICK_INFO_NO_EXPIRE);
  
        if (is_services_oper(u) || (ni == u->ni) || (ni == u->real_ni) ) {
            registros(u, ni);

           
           for (i = 0; i < aliases; i++)
              for (ni2 = nicklists[i]; ni2; ni2 = ni2->next)
                 if( ni2->link == ni ) {
                        notice_lang(s_NickServ, u, NICK_INFO_LINKS, ni2->nick, ni2->email);
                        registros(u, ni2);
                 }
        }
    }
}

/*************************************************************************/

/* SADMINS can search for nicks based on their NS_VERBOTEN and NS_NO_EXPIRE
 * status. The keywords FORBIDDEN and NOEXPIRE represent these two states
 * respectively. These keywords should be included after the search pattern.
 * Multiple keywords are accepted and should be separated by spaces. Only one
 * of the keywords needs to match a nick's state for the nick to be displayed.
 * Forbidden nicks can be identified by "[Forbidden]" appearing in the last
 * seen address field. Nicks with NOEXPIRE set are preceeded by a "!". Only
 * SADMINS will be shown forbidden nicks and the "!" indicator.
 * Syntax for sadmins: LIST pattern [FORBIDDEN] [NOEXPIRE]
 * -TheShadow
 */
//(ni->active & ACTIV_PROCESO)
static void do_list(User *u)
{
    char *pattern = strtok(NULL, " ");
    char *keyword;
    NickInfo *ni;
    int nnicks, i;
    char buf[BUFSIZE],buf1[BUFSIZE];
    /*int is_servadmin = is_services_admin(u);*/
    int is_servdevel = is_services_devel(u);
    int16 matchflags = 0; /* NS_ flags a nick must match one of to qualify */
    int32 banderas = 0;

    if (NSListOpersOnly /*&& !(u->mode & UMODE_O)*/) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
	return;
    }

/*if (!is_servdevel) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
   return;
    }*/    
    if (!pattern) {
	syntax_error(s_NickServ, u, "LIST",
		is_servdevel ? NICK_LIST_SERVADMIN_SYNTAX : NICK_LIST_SYNTAX);
    } else {
	nnicks = 0;

	while (is_servdevel && (keyword = strtok(NULL, " "))) {
	    if (stricmp(keyword, "FORBIDDEN") == 0)
		matchflags |= NS_VERBOTEN;
	    if (stricmp(keyword, "NOEXPIRE") == 0)
		matchflags |= NS_NO_EXPIRE;
            if (stricmp(keyword, "SUSPENDED") == 0)
                matchflags |= NS_SUSPENDED;    
	    if (stricmp(keyword, "NOEMAIL") == 0)
                banderas |= NS_NOVALIDADO;     
	 }

	notice_lang(s_NickServ, u, NICK_LIST_HEADER, pattern);
	for (i = 0; i < aliases; i++) {
	    for (ni = nicklists[i]; ni; ni = ni->next) {
		if (!is_servdevel && ((ni->flags & NI_PRIVATE)
           || (ni->status & NS_VERBOTEN) || (ni->status & NS_SUSPENDED)))
		    continue;
		
		if ((matchflags != 0) && !(ni->status & matchflags))
		    continue;
		if ((banderas != 0) && (ni->active & banderas))
		    continue;
		/* We no longer compare the pattern against the output buffer.
		 * Instead we build a nice nick!user@host buffer to compare.
		 * The output is then generated separately. -TheShadow */
		snprintf(buf, sizeof(buf), "%s!%s", ni->nick,
				ni->last_usermask ? ni->last_usermask : "*@*");
		if (stricmp(pattern, ni->nick) == 0 ||
					match_wild_nocase(pattern, buf)) {
		    if (++nnicks <= NSListMax) {
			char noexpire_char = ' ';
			char noemail_char = ' ';
			if (is_servdevel && (ni->status & NS_NO_EXPIRE)) {
			    noexpire_char = '!';
			/*if (!is_servdevel && (ni->flags & NI_HIDE_MASK)) {
			    snprintf(buf, sizeof(buf), "%-20s  [Registrado]",
						ni->nick);
			} */
			
			} 
			  else if (ni->status & NS_VERBOTEN) {
			    snprintf(buf, sizeof(buf), "%-20s  [Forbideado]",
						ni->nick);
                        } else if (ni->status & NS_SUSPENDED) {
                            snprintf(buf, sizeof(buf), "%-20s  [Suspendido]",
                                                ni->nick);
	                 } else if (ni->active & ACTIV_PROCESO) {
                             noemail_char = '*';             
				snprintf(buf, sizeof(buf), "%-20s   [email 5%s no confirmado]",
                                                ni->nick,ni->email);
			} else {
                            if (is_servdevel)
			    snprintf(buf, sizeof(buf), "%-20s  %s%s",
						ni->nick, ni->last_usermask,"5(Registrado)");
				else   snprintf(buf, sizeof(buf), "%-20s  %s",
						ni->nick, "5(Registrado)");
			}
#if defined(IRC_UNDERNET_P10)
			privmsg(s_NickServ, u->numerico, "   %c%s",
#else
                        privmsg(s_NickServ, u->nick, "   %c%c%s",
#endif			
						noemail_char,noexpire_char, buf);
		    }
		}
	    }
	}
	notice_lang(s_NickServ, u, NICK_LIST_RESULTS,
			nnicks>NSListMax ? NSListMax : nnicks, nnicks);
    }
}

/*************************************************************************/

/*static void do_recover(User *u)
{
    char *nick = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    NickInfo *ni;
    User *u2;

    if (!nick) {
	syntax_error(s_NickServ, u, "RECOVER", NICK_RECOVER_SYNTAX);
    } else if (!(u2 = finduser(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_IN_USE, nick);
    } else if (!(ni = u2->real_ni)) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (stricmp(nick, u->nick) == 0) {
	notice_lang(s_NickServ, u, NICK_NO_RECOVER_SELF);
    } else if (ni->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_SUSPENDED, ni->suspendreason);
    } else if (pass) {
	int res = check_password(pass, ni->pass);
	if (res == 1) {
	    collide(ni, 0);
	    notice_lang(s_NickServ, u, NICK_RECOVERED, s_NickServ, nick);
	} else {
	    notice_lang(s_NickServ, u, ACCESS_DENIED);
	    if (res == 0) {
		log("%s: RECOVER: invalid password for %s by %s!%s@%s",
			s_NickServ, nick, u->nick, u->username, u->host);
		bad_password(u);
	    }
	}
    } else {
	if (!(ni->flags & NI_SECURE) && is_on_access(u, ni)) {
	    collide(ni, 0);
	    notice_lang(s_NickServ, u, NICK_RECOVERED, s_NickServ, nick);
	} else {
	    notice_lang(s_NickServ, u, ACCESS_DENIED);
	}
    }
}



static void do_release(User *u)
{
    char *nick = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    NickInfo *ni;

    if (!nick) {
	syntax_error(s_NickServ, u, "RELEASE", NICK_RELEASE_SYNTAX);
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (!(ni->status & NS_KILL_HELD)) {
	notice_lang(s_NickServ, u, NICK_RELEASE_NOT_HELD, nick);
    } else if (ni->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_SUSPENDED, ni->suspendreason);
    } else if (pass) {
	int res = check_password(pass, ni->pass);
	if (res == 1) {
	    release(ni, 0);
	    notice_lang(s_NickServ, u, NICK_RELEASED);
	} else {
	    notice_lang(s_NickServ, u, ACCESS_DENIED);
	    if (res == 0) {
		log("%s: RELEASE: invalid password for %s by %s!%s@%s",
			s_NickServ, nick, u->nick, u->username, u->host);
		bad_password(u);
	    }
	}
    } else {
	if (!(ni->flags & NI_SECURE) && is_on_access(u, ni)) {
	    release(ni, 0);
	    notice_lang(s_NickServ, u, NICK_RELEASED);
	} else {
	    notice_lang(s_NickServ, u, ACCESS_DENIED);
	}
    }
}

/*************************************************************************/

static void do_ghost(User *u)
{
    char *nick = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    NickInfo *ni;
    User *u2;

    if (!nick) {
	syntax_error(s_NickServ, u, "GHOST", NICK_GHOST_SYNTAX);
    } else if (!(u2 = finduser(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_IN_USE, nick);
    } else if (!(ni = u2->real_ni)) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (stricmp(nick, u->nick) == 0) {
	notice_lang(s_NickServ, u, NICK_NO_GHOST_SELF);
    } else if (ni->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_SUSPENDED, ni->suspendreason);
    } else if (pass) {
	int res = check_password(pass, ni->pass);
	if (res == 1) {
	    char buf[NICKMAX+32];
	    snprintf(buf, sizeof(buf), "%s ha usado el comando GHOST", u->nick);
	    kill_user(s_NickServ, nick, buf);
	    notice_lang(s_NickServ, u, NICK_GHOST_KILLED, nick);
	} else {
	    notice_lang(s_NickServ, u, ACCESS_DENIED);
	    if (res == 0) {
		log("%s: RELEASE: invalid password for %s by %s!%s@%s",
			s_NickServ, nick, u->nick, u->username, u->host);
		bad_password(u);
	    }
	}
    } else {
	if (!(ni->flags & NI_SECURE) && is_on_access(u, ni)) {
	    char buf[NICKMAX+32];
	    snprintf(buf, sizeof(buf), "%s ha usado el comando GHOST", u->nick);
	    kill_user(s_NickServ, nick, buf);
	    notice_lang(s_NickServ, u, NICK_GHOST_KILLED, nick);
	} else {
	    notice_lang(s_NickServ, u, ACCESS_DENIED);
	}
    }
}

/*************************************************************************/

static void do_status(User *u)
{
    NickInfo *ni;

    char *nick;
    User *u2;
    int i = 0;
    /* NickInfo *u2; */


    while ((nick = strtok(NULL, " ")) && (i++ < 16)) {
       if (is_a_service(nick))
           notice_lang(s_NickServ, u, NICK_IS_A_SERVICE, nick);
       else if (!(u2 = finduser(nick)))
           notice_lang(s_NickServ, u, NICK_STATUS_OFFLINE, nick);
       else if (!(findnick(nick)))
           notice_lang(s_NickServ, u, NICK_STATUS_NOT_REGISTRED, nick);
       else if (nick_suspendied(u2))
           notice_lang(s_NickServ, u, NICK_STATUS_SUSPENDED, nick);
       
       /*else if (nick_recognized(u2))
           notice_lang(s_NickServ, u, NICK_STATUS_RECOGNIZED, nick);*/
       else if ((ni = findnick(nick)) && (u2 = finduser(nick))) {
       /*if ((ni->status & NI_ON_BDD) && nick_identified(u2)) {
      	  notice_lang(s_NickServ, u, NICK_STATUS_ID_BDD);
       }*/
	
	if (ni->active & ACTIV_CONFIRM)
		notice_lang(s_NickServ, u, NICK_STATUS_VALIDADO,nick); 
	if (ni->active & ACTIV_PROCESO)
		notice_lang(s_NickServ, u, NICK_STATUS_NOVALIDADO,nick); 
		
        

      }
      else if (nick_identified(u2))
           notice_lang(s_NickServ, u, NICK_STATUS_IDENTIFIED, nick);
  
       else
           notice_lang(s_NickServ, u, NICK_STATUS_NOT_IDENTIFIED, nick);

       }
   
}
                                                                  

/*************************************************************************/

static void do_sendpass(User *u)
{
#if defined(REG_NICK_MAIL)
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
        
    if (!nick){
        syntax_error(s_NickServ, u, "SENDPASS", NICK_SENDPASS_SYNTAX);
    } else if (!(ni = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else {
        {
         char *buf;
         char subject[BUFSIZE];
                       
         if (fork()==0) {
                       
             buf = smalloc(sizeof(char *) * 1024);
             sprintf(buf,"\n    NiCK: %s\n"
                           "Password: %s\n\n"
                           "Para identificarte   -> /nick %s:%s\n"
                           "Para cambio de clave -> /msg %s SET PASSWORD nueva_contrase�a\n\n"
                           "P�gina de Informaci�n %s\n",                           
                  ni->nick, ni->pass, ni->nick, ni->pass, s_NickServ, WebNetwork);
                    
             snprintf(subject, sizeof(subject), "Contrase�a del NiCK '%s'", ni->nick);
       
             enviar_correo(ni->email, subject, buf);             
             exit(0);
         }
        }                           
         notice_lang(s_NickServ, u, NICK_SENDPASS_SUCCEEDED, nick, ni->email);
         canalopers(s_NickServ, "12%s ha usado 12SENDPASS sobre 12%s", u->nick, nick);
    }
#else
         privmsg(s_NickServ, u->nick, "En esta red no est� disponible el SENDPASS");    
#endif    
}                                                                                                 /*************************************************************************/

static void do_sendclave(User *u)
{
#if defined(REG_NICK_MAIL)
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
        
    if (!nick){
        syntax_error(s_NickServ, u, "SENDCLAVE", NICK_SENDCLAVE_SYNTAX);
    } else if (!(ni = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else {
        {
         char *buf;
         char subject[BUFSIZE];
                       
         if (fork()==0) {
                       
             buf = smalloc(sizeof(char *) * 1024);
             sprintf(buf,"\n    NiCK: %s\n"
                           "Clave: %s\n\n"
                           "Para Confirmar Registro   -> /msg %s CONFIRM %s\n"
			    "P�gina de Informaci�n %s\n",                           
                  ni->nick, ni->clave,  s_NickServ,ni->clave, WebNetwork);
                    
             snprintf(subject, sizeof(subject), "Clave Confirmaci�n del NiCK '%s'", ni->nick);
       
             enviar_correo(ni->email, subject, buf);             
             exit(0);
         }
        }                           
         notice_lang(s_NickServ, u, NICK_SENDCLAVE_SUCCEEDED, nick, ni->email);
         canalopers(s_NickServ, "12%s ha usado 12SENDCLAVE sobre 12%s", u->nick, nick);
    }
#else
         privmsg(s_NickServ, u->nick, "En esta red no est� disponible el SENDCLAVE");    
#endif    
}                                                                                                                             
/***************************************************************************/                 
/* El RENAME */
static void do_rename(User *u)
{
   char *nick = strtok(NULL, " ");
   NickInfo *ni;
      
   if (!nick) {
       privmsg(s_NickServ, u->nick, "No se ha especificado un nick");
   } else if (!(ni = findnick(nick))) {
       privmsg(s_NickServ, u->nick, "2Nick 12%s 4No Registrado",nick);
       privmsg(s_NickServ,u->nick,"10Pruebe 3/msg 2OPER 5RAW 4RENAME 12%s",nick);
       return;
   } else if (nick_is_services_admin(ni) && !is_services_root(u)) {
       notice_lang(s_NickServ, u, PERMISSION_DENIED);
   } else {
       send_cmd(NULL, "RENAME %s", ni->nick);
   }
}

/***************************************************************************/

static void do_getpass(User *u)
{
#ifndef USE_ENCRYPTION
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
#endif

    /* Assumes that permission checking has already been done. */
#if defined(USE_ENCRYPTION)
    notice_lang(s_NickServ, u, NICK_GETPASS_UNAVAILABLE);
#else
    if (!nick) {
	syntax_error(s_NickServ, u, "GETPASS", NICK_GETPASS_SYNTAX);
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (nick_is_services_root(ni) && (u->ni != ni)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED_DIRECTOR);
    } else if (nick_is_services_admin(ni) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
    } else if (nick_is_services_cregadmin(ni) && !is_services_admin(u) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
    }  else if (nick_is_services_devel(ni) && !is_services_cregadmin(u) && !is_services_admin(u) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
    } else {
        canalopers(s_NickServ, "12%s Us� GETPASS sobre 12%s", u->nick, ni->nick);
	log("%s: %s!%s@%s used GETPASS on %s",
		s_NickServ, u->nick, u->username, u->host, nick);
	notice_lang(s_NickServ, u, NICK_GETPASS_PASSWORD_IS, nick, ni->pass);
    }
#endif
}
static void do_getclave(User *u)
{
#ifndef USE_ENCRYPTION
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
#endif

    /* Assumes that permission checking has already been done. */
#if defined(USE_ENCRYPTION)
    notice_lang(s_NickServ, u, NICK_GETPASS_UNAVAILABLE);
#else
    if (!nick) {
	syntax_error(s_NickServ, u, "GETCLAVE", NICK_GETPASS_SYNTAX);
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (nick_is_services_root(ni) && (u->ni != ni)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED_DIRECTOR);
    } else if (nick_is_services_admin(ni) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
    } else if (nick_is_services_cregadmin(ni) && !is_services_admin(u) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
    }  else if (nick_is_services_devel(ni) && !is_services_cregadmin(u) && !is_services_admin(u) && !is_services_root(u)) {
	notice_lang(s_NickServ, u, PERMISSION_DENIED);
    } else {
        canalopers(s_NickServ, "12%s Us� GETCLAVE sobre 12%s", u->nick, ni->nick);
	log("%s: %s!%s@%s used GETCLAVE on %s",
		s_NickServ, u->nick, u->username, u->host, nick);
	notice_lang(s_NickServ, u, NICK_GETCLAVE_CLAVE_IS, nick, ni->clave);
    }
#endif
}
static void do_confirm(User *u)
{
#ifndef USE_ENCRYPTION
    char *pass = strtok(NULL, " ");
    char *nick;
    NickInfo *ni;
#endif

    /* Assumes that permission checking has already been done. */
#if defined(USE_ENCRYPTION)
    notice_lang(s_NickServ, u, NICK_GETPASS_UNAVAILABLE);
#else
    if (!pass) {
        syntax_error(s_NickServ, u, "CONFIRM", NICK_CONFIRM_SYNTAX);
    } else if (!(ni = findnick(u->nick))) {
	notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, u->nick);
    } else {
		if ((ni->active & ACTIV_CONFIRM) || (ni->active & ACTIV_FORZADO)) {
		notice_lang(s_NickServ, u, NICK_ACTIVADO,u->nick);
		return;
                }

	 if (strcmp(pass, ni->clave) != 0) {
		notice_lang(s_NickServ, u, NICK_CONFIRM_INCORRECT);
		             
		} else {
		ni->active &= ~ACTIV_PROCESO;
		ni->active |= ACTIV_CONFIRM;
		 notice_lang(s_NickServ, u, NICK_CONFIRM_CORRECT);
		 canalopers(s_NickServ, "12%s CONFIRMa su correo 12%s", u->nick, ni->email);
		char *buf;
         char subject[BUFSIZE];
                       
         if (fork()==0) {
                       
             buf = smalloc(sizeof(char *) * 1024);
            sprintf(buf,"\nBienvenid@ a %s.\n"
			     "Felicitaciones por el registro del nick %s\n\n"	
			     "Tu Password en la actualidad es %s\n"
                             "Recuerda que para una mayor seguridad,\n"
			     "y una vez identificado, lo debes cambiar periodicamente.\n\n"
			     "Para cambio de Password -> /msg %s SET PASSWORD nueva_password\n"
			     "Para cualquier duda te atendemos en el canal #%s\n\n"
			      "P�gina de Informaci�n %s\n",
                       Net,ni->nick,ni->pass,s_NickServ,CanalAyuda, WebNetwork);
                    
             snprintf(subject, sizeof(subject), "BienVenid@ a %s", Net);
       
             enviar_correo(ni->email, subject, buf);             
             exit(0);
         }
		}
       	
	
	
    }
#endif
}


/*************************************************************************/

static void do_suspend(User *u)
{

    NickInfo *ni;
    char *nick, *expiry, *reason;                        
    time_t expires;

    nick = strtok(NULL, " ");
    /* Por el momento, nada de expiraciones
    if (nick && *nick == '+') {
        expiry = nick;
        nick = strtok(NULL, " ");
    } else { */
        expiry = NULL;
/*    }
 */
    reason = strtok(NULL, "");    
            
    if (!reason) {
        syntax_error(s_NickServ, u, "SUSPEND", NICK_SUSPEND_SYNTAX);
    } else if (!(ni = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (ni->status & NS_VERBOTEN) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_FORBIDDEN);
    } else if (ni->status & NS_SUSPENDED) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_SUSPENDED, nick);
    } else if (nick_is_services_admin(ni)) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12SUSPENDer al admin 12%s (%s)",
                                        u->nick, ni->nick, reason);
    } else if (nick_is_services_cregadmin(ni)) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12SUSPENDer al co-admin 12%s (%s)",
                                        u->nick, ni->nick, reason);
    } else if (nick_is_services_devel(ni)) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12SUSPENDer al devel 12%s (%s)",
                                        u->nick, ni->nick, reason);
    } else if (nick_is_services_oper(ni)) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12SUSPENDer al oper 12%s (%s)",
              u->nick, ni->nick, reason);
    } else {
        if (expiry) {
            expires = dotime(expiry);
            if (expires < 0) {
                notice_lang(s_ChanServ, u, BAD_EXPIRY_TIME);
                return;
            } else if (expires > 0) {
                expires += time(NULL);
            }
        } else {
//            expires = time(NULL) + CSSuspendExpire;
            expires = 0; /* suspension indefinida */
        }                                    
        log("%s: %s!%s@%s usado SUSPEND on %s (%s)",
             s_NickServ, u->nick, u->username, u->host, nick, reason);
        ni->suspendby = sstrdup(u->nick);
        ni->suspendreason = sstrdup(reason);
        ni->time_suspend = time(NULL);
        ni->time_expiresuspend = expires;
        ni->status |= NS_SUSPENDED;
        ni->status &= ~NS_IDENTIFIED;
	
	if (ni->status & NI_ON_BDD)
	#if defined(IRC_UNDERNET_P10)
		privmsg(s_NickServ,u->numerico, "Para identificarse haga 2,15/nick %s!%s",ni->nick,ni->pass);
		ep_tablan(ni->nick, ni->pass, 'n');
	#else
	    privmsg (s_NickServ, ni->nick, "Tu nick 12%s ha sido 12SUSPENDido"
                 " temporalmente.", ni->nick);
            privmsg (s_NickServ, ni->nick, "Causa de suspensi�n:5 %s.", reason);
		do_write_bdd(ni->nick, 16, ni->pass); 
	#endif
        
	canalopers(s_NickServ, "12%s ha 12SUSPENDido el nick 12%s (%s)",
          u->nick, nick, reason); 
        notice_lang(s_NickServ, u, NICK_SUSPEND_SUCCEEDED, nick);
        
      
                          
    }
                                                     
}
/*************************************************************************/
static void do_forbid(User *u)
{

    NickInfo *ni;
    char *nick, *expiry, *reason;                        
    time_t expires;

    nick = strtok(NULL, " ");
    /* Por el momento, nada de expiraciones
    if (nick && *nick == '+') {
        expiry = nick;
        nick = strtok(NULL, " ");
    } else { */
        expiry = NULL;
/*    }
 */
    reason = strtok(NULL, "");    
      //  ni = findnick(nick);    
    if (!reason) {
        syntax_error(s_NickServ, u, "FORBID", NICK_FORBID_SYNTAX);
    } else if (!(ni = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (ni->status & NS_VERBOTEN) {
         privmsg (s_NickServ,u->nick, "El nick 12%s ya estaba 12Forbideado"
                 " indefinidamente.", ni->nick);
       //notice_lang(s_NickServ, u, NICK_FORBID_FORBIDDEN);
     } else if (ni->status & NS_SUSPENDED) {
         privmsg (s_NickServ,u->nick, "El nick 12%s ya estaba 12Suspendido"
                 " temporalmente.", ni->nick);
       
   
    } else if (nick_is_services_admin(ni)) {
         notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12Forbidear al admin 12%s (%s)",
                                        u->nick, ni->nick, reason);
    } else if (nick_is_services_cregadmin(ni)) {
         notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12Forbidear al co-admin 12%s (%s)",
                                        u->nick, ni->nick, reason);
    } else if (nick_is_services_devel(ni)) {
         notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12Forbidear al devel 12%s (%s)",
                                        u->nick, ni->nick, reason);
    } else if (nick_is_services_oper(ni)) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_OPER);
        canalopers(s_NickServ, "12%s ha intentado 12Forbidear al oper 12%s (%s)",
              u->nick, ni->nick, reason);
    } else {
        if (expiry) {
            expires = dotime(expiry);
            if (expires < 0) {
                notice_lang(s_ChanServ, u, BAD_EXPIRY_TIME);
                return;
            } else if (expires > 0) {
                expires += time(NULL);
            }
        } else {
//            expires = time(NULL) + CSSuspendExpire;
            expires = 0; /* suspension indefinida */
        }                                    
        log("%s: %s ha usado FORBID para nick %s", s_NickServ, u->nick, nick);
       ni->forbidby = sstrdup(u->nick);
        ni->forbidreason = sstrdup(reason);
         ni->status |= NS_VERBOTEN;
        ni->status &= ~NS_IDENTIFIED;
	
	if (ni->status & NI_ON_BDD)
	#if defined(IRC_UNDERNET_P10)
		
		//ep_tablan(ni->nick, ni->pass, 'n');
	#else
	    privmsg (s_NickServ, ni->nick, "Tu nick 12%s ha sido 12Forbideado"
                 " indefinidamente.", ni->nick);
            privmsg (s_NickServ, ni->nick, "Causa de prohibici�n:5 %s.", reason);
             send_cmd(NULL, "RENAME %s", ni->nick);
		do_write_bdd(nick, 1, "!");
	#endif
        
	canalopers(s_NickServ, "12%s ha 12Forbideado el nick 12%s (%s)",
          u->nick, nick, reason); 
        notice_lang(s_NickServ, u, NICK_FORBID_SUCCEEDED, nick);
        
      
                          
    }
                                                     
}

/*************************************************************************/
/*************************************************************************/
static void do_unsuspend(User *u)
{
    char *nick = strtok(NULL, " ");
    NickInfo *ni;
        
        
    /* Assumes that permission checking has already been done. */
    if (!nick) {
        syntax_error(s_NickServ, u, "UNSUSPEND", NICK_UNSUSPEND_SYNTAX);
    } else if (!(ni = findnick(nick))) {
        notice_lang(s_NickServ, u, NICK_X_NOT_REGISTERED, nick);
    } else if (!(ni->status & NS_SUSPENDED)) {
        notice_lang(s_NickServ, u, NICK_SUSPEND_NOT_SUSPEND, ni->nick);
    } else {
         log("%s: %s!%s@%s usado UNSUSPEND on %s",
                s_NickServ, u->nick, u->username, u->host, nick);
          ni->status &= ~NS_SUSPENDED;
          free(ni->suspendby);
          free(ni->suspendreason);
          ni->time_suspend = 0;
          ni->time_expiresuspend = 0;
	  canalopers(s_NickServ, "12%s ha reactivado el nick 12%s", u->nick, nick);

	  if (ni->status & NI_ON_BDD)
		#if defined(IRC_UNDERNET_P10)
                privmsg(s_NickServ,u->numerico, "Para identificarse haga 2,15/nick %s!%s",ni->nick,ni->pass);
		ep_tablan(ni->nick, ni->pass, 'n');
		#else
		  do_write_bdd(ni->nick, 1, ni->pass);
		#endif

          notice_lang(s_NickServ, u, NICK_UNSUSPEND_SUCCEEDED, nick);
          if (finduser(nick)) {
              privmsg (s_NickServ, ni->nick, "Tu nick 12%s ha sido reactivado.", ni->nick);
              privmsg (s_NickServ, ni->nick, "Vuelve a identificarte con tu nick.");
              send_cmd(NULL, "RENAME %s", ni->nick);
	  }
    }
}




static void do_unforbid(User *u)
{
    NickInfo *ni;
    char *nick = strtok(NULL, " ");
    User *u2;
  
    /* Assumes that permission checking has already been done. */
    if (!nick) {
        syntax_error(s_NickServ, u, "UNFORBID", NICK_UNFORBID_SYNTAX);
    } else if (!((ni = findnick(nick)) && (ni->status & NS_VERBOTEN))) {
        notice_lang(s_NickServ, u, NICK_UNFORBID_NOT_FORBID, nick);
    } else {
        if (readonly)
             notice_lang(s_NickServ, u, READ_ONLY_MODE);
                 
        
        log("%s: %s set UNFORBID for nick %s", s_NickServ, u->nick, nick);
        
	if (ni->status & NS_VERBOTEN) {
	   do_write_bdd(nick, 1, ni->pass);
	    ni->status &= ~NS_VERBOTEN;
          free(ni->forbidby);
          free( ni->forbidreason);

   	notice_lang(s_NickServ, u, NICK_UNFORBID_SUCCEEDED, nick);
        canalopers(s_NickServ, "12%s ha 12UNFORBIDeado el nick 12%s",
                           u->nick, nick);
	}
	
	
       /* if (nick && (u2 = finduser(nick)))
             u2->ni = u2->real_ni = NULL;*/
    }
                                                                                                  
}

/*************************************************************************/
