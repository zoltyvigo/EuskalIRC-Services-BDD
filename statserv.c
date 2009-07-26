/* StatServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 * (20009) donostiarra admin_euskalirc@gmail.com
 *  http://euskalirc.wordpress.com
 */

#include "services.h"
#include "pseudo.h"




static void do_credits (User *u);
static void do_help (User *u);
static void do_stats(User *u);
static void do_sendweb(User *u);



/*************************************************************************/

static Command cmds[] = {
    { "CREDITS",    do_credits,    NULL,  -1,                   -1,-1,-1,-1 },
    { "CREDITOS",   do_credits,    NULL,  -1,                   -1,-1,-1,-1 },        
    { "HELP",       do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "AYUDA",      do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "SHOWCOMMANDS",    do_help,  NULL,  -1,                   -1,-1,-1,-1 },
    { "SENDWEB",    do_sendweb,    NULL,  STATS_HELP_SENDWEB,                   -1,-1,-1,-1 },
    { "STATS",      do_stats,      NULL,  OPER_HELP_STATS,      -1,-1,-1,-1 },
    { "UPTIME",     do_stats,      NULL,  OPER_HELP_STATS,      -1,-1,-1,-1 },
   /*del servers.c*/
    { "SERVERS",    do_servers,    NULL,  -1,                   -1,-1,-1,-1 },
      /* Commands for Services CoAdmins: */

  /* de channels.c */
   { "CHANLIST",  send_channel_list,  is_services_cregadmin, -1,-1,-1,-1,-1 },
   { "CHANUSERS",   send_channel_users, is_services_cregadmin, -1,-1,-1,-1,-1 },
/*de users.c*/
  { "USERLIST",  send_user_list,     is_services_cregadmin, -1,-1,-1,-1,-1 },
  { "USERINFO",   send_user_info,     is_services_cregadmin, -1,-1,-1,-1,-1 },
       
    /* Fencepost: */
    { NULL }
};



/* Main StatServ routine. */

void statserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	log("%s: user record for %s not found", s_StatServ, source);
	notice(s_StatServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    log("%s: %s: %s", s_StatServ, source, buf);

    cmd = strtok(buf, " ");
    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_StatServ, source, "\1PING %s", s);
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_StatServ, source, "\1VERSION %s %s -- %s\1",
               PNAME, s_StatServ, version_build);                
    } else {
	run_cmd(s_StatServ, u, cmds, cmd);
    }
}
static void do_credits(User *u)
{
    notice_lang(s_StatServ, u, SERVICES_CREDITS);
}
static void do_help(User *u)
{
    const char *cmd = strtok(NULL, "");
	
    if (!cmd) {
	if (is_services_cregadmin(u))
	notice_help(s_StatServ, u, STATS_ADMIN_HELP);
	} else {
	help_cmd(s_StatServ, u, cmds, cmd);
    }

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
	    notice_lang(s_StatServ, u, OPER_STATS_AKILL_COUNT, num_akills());
	    if (timeout >= 172800)
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_DAYS,
			timeout/86400);
	    else if (timeout >= 86400)
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_DAY);
	    else if (timeout >= 7200)
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_HOURS,
			timeout/3600);
	    else if (timeout >= 3600)
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_HOUR);
	    else if (timeout >= 120)
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_MINS,
			timeout/60);
	    else if (timeout >= 60)
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_MIN);
	    else
		notice_lang(s_StatServ, u, OPER_STATS_AKILL_EXPIRE_NONE);
	    return;
	} else {
	    notice_lang(s_StatServ, u, OPER_STATS_UNKNOWN_OPTION,
			strupper(extra));
	}
    }

    notice_lang(s_StatServ, u, OPER_STATS_CURRENT_USERS, usercnt, opcnt,helpcnt,invcnt);
    tm = localtime(&maxusertime);
    strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
    notice_lang(s_StatServ, u, OPER_STATS_MAX_USERS, maxusercnt, timebuf);
    if (days > 1) {
	notice_lang(s_StatServ, u, OPER_STATS_UPTIME_DHMS,
		days, hours, mins, secs);
    } else if (days == 1) {
	notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1DHMS,
		days, hours, mins, secs);
    } else {
	if (hours > 1) {
	    if (mins != 1) {
		if (secs != 1) {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_HMS,
				hours, mins, secs);
		} else {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_HM1S,
				hours, mins, secs);
		}
	    } else {
		if (secs != 1) {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_H1MS,
				hours, mins, secs);
		} else {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_H1M1S,
				hours, mins, secs);
		}
	    }
	} else if (hours == 1) {
	    if (mins != 1) {
		if (secs != 1) {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1HMS,
				hours, mins, secs);
		} else {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1HM1S,
				hours, mins, secs);
		}
	    } else {
		if (secs != 1) {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1H1MS,
				hours, mins, secs);
		} else {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1H1M1S,
				hours, mins, secs);
		}
	    }
	} else {
	    if (mins != 1) {
		if (secs != 1) {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_MS,
				mins, secs);
		} else {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_M1S,
				mins, secs);
		}
	    } else {
		if (secs != 1) {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1MS,
				mins, secs);
		} else {
		    notice_lang(s_StatServ, u, OPER_STATS_UPTIME_1M1S,
				mins, secs);
		}
	    }
	}
    }

    if (extra && stricmp(extra, "ALL") == 0 && is_services_admin(u)) {
	long count, mem, count2, mem2;

	notice_lang(s_StatServ, u, OPER_STATS_BYTES_READ, total_read / 1024);
	notice_lang(s_StatServ, u, OPER_STATS_BYTES_WRITTEN, 
			total_written / 1024);
        get_server_stats(&count, &mem);
        notice_lang(s_StatServ, u, OPER_STATS_SERVER_MEM,
                        count, (mem+512) / 1024);
	get_user_stats(&count, &mem);
	notice_lang(s_StatServ, u, OPER_STATS_USER_MEM,
			count, (mem+512) / 1024);
	get_channel_stats(&count, &mem);
	notice_lang(s_StatServ, u, OPER_STATS_CHANNEL_MEM,
			count, (mem+512) / 1024);
	get_nickserv_stats(&count, &mem);
	notice_lang(s_StatServ, u, OPER_STATS_NICKSERV_MEM,
			count, (mem+512) / 1024);
	get_chanserv_stats(&count, &mem);
	notice_lang(s_StatServ, u, OPER_STATS_CHANSERV_MEM,
			count, (mem+512) / 1024);
	count = 0;
	get_akill_stats(&count2, &mem2);
	count += count2;
	mem += mem2;
	get_news_stats(&count2, &mem2);
	count += count2;
	mem += mem2;
	/*notice_lang(s_StatServ, u, OPER_STATS_OPERSERV_MEM,
			count, (mem+512) / 1024);*/ /*habra que mirar otros registros*/

    }
}
static void do_sendweb(User *u)
{
    NickInfo *ni;
    
   struct tm *tm;
   char timebuf[64]; 
tm = localtime(&maxusertime);
strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);

   
         char *buf;
         char desde[BUFSIZE];
                       
         if (fork()==0) {
                       
             buf = smalloc(sizeof(char *) * 1024);
             sprintf(buf,"\n Información Estadística \n<br><br>"
                        "Usuarios Conectados %d IRCops %d Helpers %d Invitados %d\n<br>"
			"Máximo Número de Usuarios %d (%s)\n<br>",                           
          usercnt, opcnt,helpcnt,invcnt,maxusercnt, timebuf);
                    
             snprintf(desde, sizeof(desde), "Red de Noticias Estadísticas '%s'", WebNetwork);
         
             enviar_web(desde, buf);      
                      
         
                               
      privmsg(s_StatServ, u->nick, "Stats Enviadas a la Web");    
         canaladmins(s_StatServ, "12%s ha usado 12SENDWEB", u->nick);
     exit(0);    
    }

}                                           



