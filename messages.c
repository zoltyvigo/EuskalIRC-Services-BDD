/* Definitions of IRC message functions and list of messages.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"
#include "messages.h"
#include "language.h"

/* List of messages is at the bottom of the file. */

/*************************************************************************/

static void m_server(char *source, int ac, char **av)
{

     char numerico[2];

/* Handle a server SERVER command.
 *      source = Server hub
  *      av[0] = Nuevo Server
   *      av[1] = hop count
    *      av[2] = signon time
     *      av[3] = signon
      *      av[4] = protocolo
       *      av[5] = numerico
        *      av[6] = user's server
         *      av[7] = Descripcion
          */
          

     strscpy(numerico, av[5], 2);
     wallops(s_OperServ, "SERVER 12%s Numeric 12%s entra en la RED.", av[0], numerico);
}
/*************************************************************************/
static void m_squit(char *source, int ac, char **av)
{
   /* Handle a server SQUIT command.
    *       soruce =  nick/server
    *      av[0] = Nuevo Server
    *      av[1] = signon time
    *      av[2] = motivo
    */
  
     wallops(s_OperServ, "SQUIT de 12%s por 12%s Motivo: %s", av[0], source, av[2]);
}
                          
/*************************************************************************/
static void m_db(char *source, int ac, char **av)
{
/*
    DB[1].registros  Tabla 'b' Bots Clones/Ilines
    DB[2].registros  Tabla 'c' Canales Temporal
    DB[3].registros  Tabla 'i' Clones/Ilines
    DB[4].registros  Tabla 'n' NiCKS
    DB[5].registros  Tabla 'o' Opers/Admins
    DB[6].registros  Tabla 't' Temporal
    DB[7].registros  Tabla 'v' Vhosts
*/

    if (strcmp(av[5], "2")  == 0) {
        DB[4].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla N  Serie: %lu", DB[4].registros);
    }                            
    if (strcmp(av[5], "b")  == 0) {
        DB[1].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla B  Serie: %lu", DB[1].registros);
    }
    if (strcmp(av[5], "c")  == 0) {
        DB[2].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla C  Serie: %lu", DB[2].registros);
    }
    if (strcmp(av[5], "i")  == 0) {
        DB[3].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla I  Serie: %lu", DB[3].registros);
    }
    if (strcmp(av[5], "n")  == 0) {
        DB[4].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla N  Serie: %lu", DB[4].registros);
    }                                                
    if (strcmp(av[5], "o")  == 0) {
        DB[5].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla O  Serie: %lu", DB[5].registros);
    }
    if (strcmp(av[5], "t")  == 0) {
        DB[6].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla T  Serie: %lu", DB[6].registros);
    }    
    if (strcmp(av[5], "v")  == 0) {
        DB[7].registros = atol(av[4]);
//        canalopers(s_OperServ, "Tabla V  Serie: %lu", DB[7].registros);
    }

}
       
                         
/*************************************************************************/

static void m_nickcoll(char *source, int ac, char **av)
{
    if (ac < 1)
	return;
    if (!skeleton && !readonly)
	introduce_user(av[0]);
}

/*************************************************************************/

static void m_ping(char *source, int ac, char **av)
{
    if (ac < 1)
	return;
    send_cmd(ServerName, "PONG %s %s", ac>1 ? av[1] : ServerName, av[0]);
}

/*************************************************************************/

static void m_away(char *source, int ac, char **av)
{
    User *u = finduser(source);

    if (u && (ac == 0 || *av[0] == 0))	/* un-away */
	check_memos(u);
}

/*************************************************************************/

static void m_join(char *source, int ac, char **av)
{
    if (ac != 1)
	return;
    do_join(source, ac, av);
}

/*************************************************************************/

static void m_kick(char *source, int ac, char **av)
{
    if (ac != 3)
	return;
    do_kick(source, ac, av);
}

/*************************************************************************/

static void m_kill(char *source, int ac, char **av)
{
    if (ac != 2)
	return;
    /* Recover if someone kills us. */
    if (stricmp(av[0], s_OperServ) == 0 ||
        stricmp(av[0], s_NickServ) == 0 ||
        stricmp(av[0], s_ChanServ) == 0 ||
        stricmp(av[0], s_CregServ) == 0 ||
        stricmp(av[0], s_MemoServ) == 0 ||
        stricmp(av[0], s_HelpServ) == 0 ||
        stricmp(av[0], s_NewsServ) == 0 ||
        (s_IrcIIHelp && stricmp(av[0], s_IrcIIHelp) == 0) ||
        (s_DevNull && stricmp(av[0], s_DevNull) == 0) ||
        stricmp(av[0], s_GlobalNoticer) == 0
    ) {
	if (!readonly && !skeleton)
	    introduce_user(av[0]);
    } else {
	do_kill(source, ac, av);
    }
}

/*************************************************************************/

static void m_mode(char *source, int ac, char **av)
{
    if (*av[0] == '#' || *av[0] == '&') {
	if (ac < 2)
	    return;
	do_cmode(source, ac, av);
    } else {
	if (ac != 2)
	    return;
	do_umode(source, ac, av);
    }
}

/*************************************************************************/

static void m_motd(char *source, int ac, char **av)
{
    FILE *f;
    char buf[BUFSIZE];

    f = fopen(MOTDFilename, "r");
    send_cmd(ServerName, "375 %s :- %s Mensaje del Dia",
		source, ServerName);
   if (f) {
	while (fgets(buf, sizeof(buf), f)) {
	    buf[strlen(buf)-1] = 0;
	    send_cmd(ServerName, "372 %s :- %s", source, buf);
	}
	fclose(f);
    } else {
	send_cmd(ServerName, "372 %s :- Archivo MOTD no ha sido encontrado!  Por favor"
			" contacta con tu IRC Administrador.", source);
    }

    /* Look, people.  I'm not asking for payment, praise, or anything like
     * that for using Services... is it too much to ask that I at least get
     * some recognition for my work?  Please don't remove the copyright
     * message below.
     */

    send_cmd(ServerName, "372 %s :-", source);
    send_cmd(ServerName, "372 %s :- Services est� en copyright (c) "
		"1996-1999 Andy Church.", source);
    send_cmd(ServerName, "372 %s :- 2000, Toni Garc�a, ZoLtan. Proyecto GNU ", source);
                    		
    send_cmd(ServerName, "376 %s :End of /MOTD command.", source);
}

/*************************************************************************/

static void m_nick(char *source, int ac, char **av)
{

    /* ircu sends the server as the source for a NICK message for a new
     * user. */
    if (strchr(source, '.'))
	*source = 0;

    if ((!*source && ac != 7) || (*source && ac != 2)) {
	if (debug) {
	    log("debug: NICK message: expecting 2 or 7 parameters after "
	        "parsing; got %d, source=`%s'", ac, source);
	}
	return;
    }
    do_nick(source, ac, av);

    /* Nothing to do yet; information comes from USER command. */

}

/*************************************************************************/

static void m_part(char *source, int ac, char **av)
{
    if (ac < 1 || ac > 2)
	return;
    do_part(source, ac, av);
}

/*************************************************************************/

static void m_privmsg(char *source, int ac, char **av)
{
    time_t starttime, stoptime;	/* When processing started and finished */
    char *s;

    if (ac != 2)
	return;

    /* Check if we should ignore.  Operators always get through. */
    if (allow_ignore && !is_oper(source)) {
	IgnoreData *ign = get_ignore(source);
	if (ign && ign->time > time(NULL)) {
	    log("Ignored message from %s: \"%s\"", source, inbuf);
	    return;
	}
    }

    /* If a server is specified (nick@server format), make sure it matches
     * us, and strip it off. */
    s = strchr(av[0], '@');
    if (s) {
	*s++ = 0;
	if (stricmp(s, ServerName) != 0)
	    return;
    }

    starttime = time(NULL);

    if (stricmp(av[0], s_OperServ) == 0) {
	if (is_oper(source)) {
	    operserv(source, av[1]);
	} else {
	    User *u = finduser(source);
	    if (u)
		notice_lang(s_OperServ, u, ACCESS_DENIED);
	    else
		notice(s_OperServ, source, "Access denied.");
	    if (WallBadOS)
		wallops(s_OperServ, "Denegando el acceso a %s desde %s (No es OPER)",
			s_OperServ, source);
	}
    } else if (stricmp(av[0], s_NickServ) == 0) {
	nickserv(source, av[1]);
    } else if (stricmp(av[0], s_ChanServ) == 0) {
	chanserv(source, av[1]);
    } else if (stricmp(av[0], s_CregServ) == 0) {
        cregserv(source, av[1]);
    } else if (stricmp(av[0], s_MemoServ) == 0) {
	memoserv(source, av[1]);
    } else if (stricmp(av[0], s_HelpServ) == 0) {
	helpserv(s_HelpServ, source, av[1]);
/***    } else if (stricmp(av[0], s_NewsServ) == 0) {
        newsserv(sourde, av[1]); *****/
    } else if (s_IrcIIHelp && stricmp(av[0], s_IrcIIHelp) == 0) {
	char buf[BUFSIZE];
	snprintf(buf, sizeof(buf), "ircII %s", av[1]);
	helpserv(s_IrcIIHelp, source, buf);
    }

    /* Add to ignore list if the command took a significant amount of time. */
    if (allow_ignore) {
	stoptime = time(NULL);
	if (stoptime > starttime && *source && !strchr(source, '.'))
	    add_ignore(source, stoptime-starttime);
    }
}

/*************************************************************************/

static void m_quit(char *source, int ac, char **av)
{
    if (ac != 1)
	return;
    do_quit(source, ac, av);
}

/*************************************************************************/

static void m_stats(char *source, int ac, char **av)
{
    if (ac < 1)
	return;
    switch (*av[0]) {
      case 'u': {
	int uptime = time(NULL) - start_time;
	send_cmd(NULL, "242 %s :Services up %d day%s, %02d:%02d:%02d",
		source, uptime/86400, (uptime/86400 == 1) ? "" : "s",
		(uptime/3600) % 24, (uptime/60) % 60, uptime % 60);
	send_cmd(NULL, "250 %s :Current users: %d (%d ops); maximum %d",
		source, usercnt, opcnt, maxusercnt);
	send_cmd(NULL, "219 %s u :End of /STATS report.", source);
	break;
      } /* case 'u' */

      case 'l':
	send_cmd(NULL, "211 %s Server SendBuf SentBytes SentMsgs RecvBuf "
		"RecvBytes RecvMsgs ConnTime", source);
	send_cmd(NULL, "211 %s %s %d %d %d %d %d %d %ld", source, RemoteServer,
		read_buffer_len(), total_read, -1,
		write_buffer_len(), total_written, -1,
		start_time);
	send_cmd(NULL, "219 %s l :End of /STATS report.", source);
	break;

      case 'c':
      case 'h':
      case 'i':
      case 'k':
      case 'm':
      case 'o':
      case 'y':
	send_cmd(NULL, "219 %s %c :/STATS %c not applicable or not supported.",
		source, *av[0], *av[0]);
	break;
    }
}

/*************************************************************************/

static void m_time(char *source, int ac, char **av)
{
    time_t t;
    struct tm *tm;
    char buf[64];

    time(&t);
    tm = localtime(&t);
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y %Z", tm);
    send_cmd(NULL, "391 :%s", buf);
}

/*************************************************************************/

static void m_topic(char *source, int ac, char **av)
{
    if (ac != 4)
	return;
    do_topic(source, ac, av);
}

/*************************************************************************/

static void m_user(char *source, int ac, char **av)
{
    /* Do nothing - we get everything we need from the NICK command. */
}

/*************************************************************************/

void m_version(char *source, int ac, char **av)
{
    if (source)
	send_cmd(ServerName, "351 %s ircservices-%s %s :-- %s",
			source, version_number, ServerName, version_build);
}

/*************************************************************************/

void m_whois(char *source, int ac, char **av)
{
    const char *clientdesc;

    if (source && ac >= 1) {
	if (stricmp(av[0], s_NickServ) == 0)
	    clientdesc = desc_NickServ;
	else if (stricmp(av[0], s_ChanServ) == 0)
	    clientdesc = desc_ChanServ;
        else if (stricmp(av[0], s_CregServ) == 0)
            clientdesc = desc_CregServ;                    	    
	else if (stricmp(av[0], s_MemoServ) == 0)
	    clientdesc = desc_MemoServ;
	else if (stricmp(av[0], s_HelpServ) == 0)
	    clientdesc = desc_HelpServ;
	else if (stricmp(av[0], s_NewsServ) == 0)
	    clientdesc = desc_NewsServ;	                        
	else if (s_IrcIIHelp && stricmp(av[0], s_IrcIIHelp) == 0)
	    clientdesc = desc_IrcIIHelp;
	else if (stricmp(av[0], s_OperServ) == 0)
	    clientdesc = desc_OperServ;
	else if (stricmp(av[0], s_GlobalNoticer) == 0)
	    clientdesc = desc_GlobalNoticer;
	else if (s_DevNull && stricmp(av[0], s_DevNull) == 0)
	    clientdesc = desc_DevNull;
	else {
	    send_cmd(ServerName, "401 %s %s :No such service.", source, av[0]);
	    return;
	}
	send_cmd(ServerName, "311 %s %s %s %s :%s", source, av[0],
		ServiceUser, ServiceHost, clientdesc);
	send_cmd(ServerName, "312 %s %s %s :%s", source, av[0],
		ServerName, ServerDesc);           
        send_cmd(ServerName, "318 End of /WHOIS response.");
    }
}

/*************************************************************************/
/*************************************************************************/

Message messages[] = {

    { "219",       NULL },
    { "436",       m_nickcoll },
    { "AWAY",      m_away },
    { "CONFIG",    NULL },
    { "DB",        m_db },
    { "JOIN",      m_join },
    { "KICK",      m_kick },
    { "KILL",      m_kill },
    { "MODE",      m_mode },
    { "MOTD",      m_motd },
    { "NICK",      m_nick },
    { "NOTICE",    NULL },
    { "PART",      m_part },
    { "PASS",      NULL },
    { "PING",      m_ping },
    { "PRIVMSG",   m_privmsg },
    { "QUIT",      m_quit },
    { "SERVER",    m_server },
    { "SQUIT",     m_squit },
    { "STATS",     m_stats },
    { "TIME",      m_time },
    { "TOPIC",     m_topic },
    { "USER",      m_user },
    { "VERSION",   m_version },
/*    { "WALLOPS",   NULL }, */
    { "WHOIS",     m_whois },
/*    { "GLINE",     NULL }, */
    { "C",         m_db },
    { NULL }

};

/*************************************************************************/

Message *find_message(const char *name)
{
    Message *m;

    for (m = messages; m->name; m++) {
	if (stricmp(name, m->name) == 0)
	    return m;
    }
    return NULL;
}

/*************************************************************************/
