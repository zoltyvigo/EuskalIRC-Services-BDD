/* IRCuH (P9) protocol module for IRC Services.
 *
 * IRC Services is copyright (c) 1996-2009 Andrew Church.
 *     E-mail: <achurch@achurch.org>
 * Parts written by Andrew Kempe and others.
 * This program is free but copyrighted software; see the file GPL.txt for
 * details.
 */

#include "services.h"
#include "modules.h"
#include "conffile.h"
#include "language.h"
#include "messages.h"
#include "version.h"
#include "bdd.c"
/*************************************************************************/

static char *NetworkDomain = NULL;

char *s_NickServ,*s_ChanServ,*s_HelpServ,*s_MemoServ,*s_OperServ,*s_GlobalServ,*s_StatServ,*s_CregServ;

/*************************************************************************/
/************************** User/channel modes ***************************/
/*************************************************************************/

struct modedata_init {
    uint8 mode;
    ModeData data;
};

static const struct modedata_init new_usermodes[] = {
    {'g', {0x00000008}},        /* Receive globops */
    {'h', {0x00000010}},        /* Helpop */
    {'r', {0x00000020,0,0,0,MI_REGISTERED}},
                                /* Registered nick */
    {'x', {0x00000040}},        /* Mask hostname */
};

static const struct modedata_init new_chanmodes[] = {
    {'R', {0x00000100,0,0,0,MI_REGNICKS_ONLY}},
                               /* Only identified users can join (m_services)*/
    {'r', {0x00000200,0,0,0,MI_REGISTERED}},
                                /* Set for all registered channels */
    {'c', {0x00000400,0,0}},    /* No ANSI colors in channel (m_blockcolor) */
   
   
    {'C', {0x00001000,0,0}},    /* No CTCPs (m_noctcp) */
    {'N', {0x00002000,0,0}},    /* No nick changing (m_nonicks) */
    {'u', {0x00004000,0,0}},    /* No CTCPs (m_noctcp) */
    {'l', {0x00008000,0,0}},    
    {'M', {0x00100000,0,0}},    /* Moderated to unregged nicks (m_services) */

};


static const struct modedata_init new_chanusermodes[] = {
};

static void init_modes(void)
{
    int i;

    for (i = 0; i < lenof(new_usermodes); i++)
        usermodes[new_usermodes[i].mode] = new_usermodes[i].data;
    for (i = 0; i < lenof(new_chanmodes); i++)
        chanmodes[new_chanmodes[i].mode] = new_chanmodes[i].data;
    for (i = 0; i < lenof(new_chanusermodes); i++)
        chanusermodes[new_chanusermodes[i].mode] = new_chanusermodes[i].data;

    mode_setup();
};

/*************************************************************************/
/************************* IRC message receiving *************************/
/*************************************************************************/

static void m_nick(char *source, int ac, char **av)
{
    /* ircu sends the server as the source for a NICK message for a new
     * user. */
    if (strchr(source, '.'))
        *source = 0;

    if (*source) {
        /* Old user changing nicks. */
        if (ac != 2) {
            module_log_debug(1, "NICK message: wrong number of parameters"
                             " (%d) for source `%s'", ac, source);
        } else {
            do_nick(source, ac, av);
        }
        return;
    }

    /* New user. */

    if ((!*source && ac != 7) || (*source && ac != 2)) {
        module_log_debug(1, "NICK message: wrong number of parameters (%d)"
                         " for new user", ac);
        return;
    }
    do_nick(source, ac, av);
}
static void m_count(char *source, int ac, char **av)
{
	char *cntbdd;
	int finalc;

	cntbdd = strtok(av[4],"S=");
	finalc = atoi(cntbdd);
	if (ac < 1)
	  return;

	if (stricmp(av[3],"'n'") == 0) 
       	   do_count_bdd(1,finalc);
           	
	if (stricmp(av[3],"'v'") == 0) 
	   do_count_bdd(2,finalc);

	if (stricmp(av[3],"'o'") == 0)
	   do_count_bdd(3,finalc);

	if (stricmp(av[3],"'w'") == 0)
	   do_count_bdd(4,finalc);

	if (stricmp(av[3],"'i'") == 0)
	   do_count_bdd(5,finalc);
	if (stricmp(av[3],"'z'") == 0)
	   do_count_bdd(6,finalc);
	if (stricmp(av[3],"'c'") == 0)
	   do_count_bdd(7,finalc);
	if (stricmp(av[3],"'r'") == 0)
	   do_count_bdd(9,finalc);
	if (stricmp(av[3],"'j'") == 0)
	   do_count_bdd(10,finalc);
}
static void m_info(char *source, int ac, char **av)
{
    int i;
    struct tm *tm;
    char timebuf[64];

    if (!*source) {
        log("Source missing from INFO message");
        return;
    }

    tm = localtime(&start_time);
    strftime(timebuf, sizeof(timebuf), "%a %b %d %H:%M:%S %Y %Z", tm);

    for (i = 0; info_text[i]; i++)
        send_cmd(ServerName, "371 %s :%s", source, info_text[i]);
    send_cmd(ServerName, "371 %s :euskalirc-services-bdd %s %s", source,
              version_number,version_build);
    send_cmd(ServerName, "371 %s :On-line since %s", source, timebuf);
    send_cmd(ServerName, "374 %s :End of /INFO list.", source);
}

/*************************************************************************/

static Message ircuh_p9_messages[] = {
    { "GLINE",     NULL },
    { "NICK",      m_nick },
    { "249",	   m_count },
    { "INFO",      m_info },
 
    { NULL }
};

/*************************************************************************/
/************************** IRC message sending **************************/
/*************************************************************************/

/* Send a NICK command for a new user. */

static void do_send_nick(const char *nick, const char *user, const char *host,
                         const char *server, const char *name,
                         const char *modes)
{
    send_cmd(ServerName, "NICK %s 1 %ld %s %s %s :%s", nick,
             (long)time(NULL), user, host, server, name);
       	send_cmd(nick, "JOIN #%s", CanalOpers);
	send_cmd(ServerName, "MODE #%s +o %s", CanalOpers, nick);
	send_cmd(nick, "JOIN #%s", CanalAdmins);
        send_cmd(ServerName, "MODE #%s +o %s", CanalAdmins, nick);
    if (modes)
        send_cmd(nick, "MODE %s +%s", nick, modes);

send_cmd(NULL, "STATS b");
}

/*************************************************************************/

/* Send a NICK command to change an existing user's nick. */

static void do_send_nickchange(const char *nick, const char *newnick)
{
    send_cmd(nick, "NICK %s %ld", newnick, (long)time(NULL));
}

/*************************************************************************/

/* Send a command to change a user's "real name". */

static void do_send_namechange(const char *nick, const char *newname)
{
    /* Not supported by this protocol. */
}

/*************************************************************************/

/* Send a SERVER command, and anything else needed at the beginning of the
 * connection.
 */

static void do_send_server(void)
{
     start_time = time(NULL);
    send_cmd(NULL, "PASS :%s", RemotePassword);
    send_cmd(NULL, "SERVER %s 1 %lu %lu P09 :%s", ServerName,start_time, start_time, ServerDesc);
}

/*************************************************************************/

/* Send a SERVER command for a remote (juped) server. */

static void do_send_server_remote(const char *server, const char *reason)
{
    send_cmd(NULL, "SERVER %s 2 %lu %lu P09 :%s", server,start_time, start_time, reason);
}

/*************************************************************************/

/* Send a WALLOPS. */

static void do_wallops(const char *source, const char *fmt, ...)
{
    va_list args;
    char buf[BUFSIZE];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    send_cmd(source ? source : ServerName, "WALLOPS :%s", buf);
}

/*************************************************************************/

/* Send a NOTICE to all users on the network. */

static void do_notice_all(const char *source, const char *fmt, ...)
{
    va_list args;
    char msgbuf[BUFSIZE];

    va_start(args, fmt);
    vsnprintf(msgbuf, sizeof(msgbuf), fmt, args);
    va_end(args);
    if (NetworkDomain) {
        send_cmd(source, "NOTICE $*.%s :%s", NetworkDomain, msgbuf);
    } else {
        /* Go through all common top-level domains.  If you have others,
         * add them here. */
        send_cmd(source, "NOTICE $*.com :%s", msgbuf);
        send_cmd(source, "NOTICE $*.net :%s", msgbuf);
        send_cmd(source, "NOTICE $*.org :%s", msgbuf);
        send_cmd(source, "NOTICE $*.edu :%s", msgbuf);
    }
}

/*************************************************************************/

/* Send a command which modifies channel status. */

static void do_send_channel_cmd(const char *source, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsend_cmd(source, fmt, args);
    va_end(args);
}

/*************************************************************************/
/******************************* Callbacks *******************************/
/*************************************************************************/

static int do_set_topic(const char *source, Channel *c, const char *topic,
                        const char *setter, time_t t)
{
    if (setter)
        return 0;
    c->topic_time = t;
    send_cmd(source, "TOPIC %s %s %lu :%s", c->name, c->topic_setter,
             (long)c->topic_time, c->topic ? c->topic : "");
    return 1;
}

/*************************************************************************/

static int do_send_akill(const char *username, const char *host,
                         time_t expires, const char *who, const char *reason)
{
    time_t now = time(NULL);

    send_cmd(ServerName, "GLINE * +%s@%s %ld :%s", username, host,
             (long)(expires && expires > now ? expires-now : (0x7FFFFFFE)-now),
             reason);
    return 1;
}

/*************************************************************************/

static int do_cancel_akill(const char *username, const char *host)
{
    send_cmd(ServerName, "GLINE * -%s@%s", username, host);
    return 1;
}

/*************************************************************************/
/***************************** Module stuff ******************************/
/*************************************************************************/

ConfigDirective module_config[] = {
    { "NetworkDomain", { { CD_STRING, 0, &NetworkDomain } } },
    { NULL }
};

/*************************************************************************/

static int do_load_module(Module *mod, const char *modname)
{
    if (strcmp(modname, "operserv/akill") == 0) {
        if (!add_callback(mod, "send_akill", do_send_akill))
            module_log("Unable to add send_akill callback");
        if (!add_callback(mod, "cancel_akill", do_cancel_akill))
            module_log("Unable to add cancel_akill callback");
    }
    return 0;
}

/*************************************************************************/

static int do_unload_module(Module *mod)
{
    return 0;
}

/*************************************************************************/

int init_module(void)
{
    protocol_name     = "IRCuH";
    protocol_version  = "2.9.x";
    protocol_features = 0;
    protocol_nickmax  = 15;

    if (!register_messages(ircuh_p9_messages)) {
        module_log("Unable to register messages");
        exit_module(1);
        return 0;
    }

    if (!add_callback(NULL, "load module", do_load_module)
     || !add_callback(NULL, "unload module", do_unload_module)
     || !add_callback(NULL, "set topic", do_set_topic)
    ) {
        module_log("Unable to add callbacks");
        exit_module(1);
        return 0;
    }

    init_modes();

    send_nick          = do_send_nick;
    send_nickchange    = do_send_nickchange;
    send_namechange    = do_send_namechange;
    send_server        = do_send_server;
    send_server_remote = do_send_server_remote;
    wallops            = do_wallops;
    notice_all         = do_notice_all;
    send_channel_cmd   = do_send_channel_cmd;
    pseudoclient_modes = "rdkB";
    enforcer_modes     = "";
    pseudoclient_oper  = 1;

    mapstring(OPER_BOUNCY_MODES, OPER_BOUNCY_MODES_U_LINE);

    return 1;
}

/*************************************************************************/

int exit_module(int shutdown)
{
    if (!shutdown) {
        /* Do not allow removal */
        return 0;
    }

    remove_callback(NULL, "set topic", do_set_topic);
    remove_callback(NULL, "unload module", do_unload_module);
    remove_callback(NULL, "load module", do_load_module);
    unregister_messages(ircuh_p9_messages);
    return 1;
}

/*************************************************************************/

/*
 * Local variables:
 *   c-file-style: "stroustrup"
 *   c-file-offsets: ((case-label . *) (statement-case-intro . *))
 *   indent-tabs-mode: nil
 * End:
 *
 * vim: expandtab shiftwidth=4:
 */
