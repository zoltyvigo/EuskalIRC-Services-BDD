/* Funciones CyberServ.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * Modulo CyberServ hecho por zoltan <zolty@ctv.es>
 * 
 * Basado en CyBeR de GlobalChat.Org <irc.globalchat.org>
 * Thz a CyBeRpUnK por el codigo <cyberpunk@ncsa.es>
 */

/*************************************************************************/

#include "services.h"
#include "pseudo.h"

/*************************************************************************/

//static IlineInfo *ilinelists[256];

static void do_help(User *u);
static void do_cyberkill(User *u);
static void do_cybergline(User *u);
static void do_cyberunban(User *u);

static Command cmds[] = {
    { "HELP",       do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "AYUDA",      do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "SHOWCOMMANDS",  do_help,    NULL,  -1,                    -1,-1,-1,-1 },
    { ":?",         do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "?",          do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "KILL",       do_cyberkill,  NULL,  CYBER_HELP_KILL,       -1,-1,-1,-1 },
    { "GLINE",      do_cybergline, NULL,  CYBER_HELP_GLINE,      -1,-1,-1,-1 },
    { "UNBAN",      do_cyberunban, NULL,  CYBER_HELP_UNBAN,      -1,-1,-1,-1 },
    { "CLONES",     do_session,    NULL,  -1,
            CYBER_HELP_CLONES, OPER_HELP_SESSION,
            OPER_HELP_SESSION, OPER_HELP_SESSION },
    { "ILINE",      do_exception,  is_services_admin,
            OPER_HELP_EXCEPTION, -1,-1,-1, -1 },
    { NULL }
};

/*************************************************************************/

/* Main CyberServ routine. */

void cyberserv(const char *source, char *buf)
{
    char *cmd, *s;
    User *u = finduser(source);
        
    if (!u) {
        log("%s: user record for %s not found", s_CyberServ, source);
        notice(s_CyberServ, source,
           getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
        return;
    }
    
    cmd = strtok(buf, " ");
    if (!cmd) {
        return;
    } else if (stricmp(cmd, "\1PING") == 0) {
        if (!(s = strtok(NULL, "")))
            s = "\1";
        notice(s_CyberServ, source, "\1PING %s", s);   
    } else if (skeleton) {
        notice_lang(s_CyberServ, u, SERVICE_OFFLINE, s_CyberServ);
    } else {
        run_cmd(s_CyberServ, u, cmds, cmd);
    }
 
}
/*************************************************************************/

/* Return a help message. */

static void do_help(User *u)
{
    char *cmd = strtok(NULL, "");

    if (!cmd) {                                                      
        notice_help(s_CyberServ, u, CYBER_HELP);
        
        if (is_services_admin(u))
            notice_help(s_CyberServ, u, CYBER_SERVADMIN_HELP); 
    } else { 
        help_cmd(s_CyberServ, u, cmds, cmd);
    } 
}    

/*************************************************************************/
static void do_cyberkill(User *u)
{
    char *nick = strtok(NULL, " ");
    char *motivo = strtok(NULL, "");

    User *u2 = NULL;    

    if (!is_cyber_admin(u)) {
        notice_lang(s_CyberServ, u, CYBER_NO_ADMIN_CYBER);
        return;
    }    


    if (!nick) {
        syntax_error(s_CyberServ, u, "KILL", CYBER_KILL_SYNTAX);
        return;
    }    

   if (motivo) {
       char buf[BUFSIZE];
       snprintf(buf, sizeof(buf), "%s -> %s", s_CyberServ, motivo);
       motivo = sstrdup(buf);
   } else {
       char buf[BUFSIZE];
       snprintf(buf, sizeof(buf), "%s -> Killed.", s_CyberServ);
       motivo = sstrdup(buf);
   }
    
   u2 = finduser(nick);    

   if (!u2) {
       notice_lang(s_CyberServ, u, NICK_X_NOT_IN_USE, nick);
       return;
   }   
   
   if (stricmp(u2->host, u->host) != 0) {
       notice_lang(s_CyberServ, u, CYBER_NICK_NOT_IN_CYBER, u2->nick);
       return;                    
   } else { 
   /* BUG: Mirar esto */
       kill_user(s_CyberServ, u2->nick, motivo); 
       notice_lang(s_CyberServ, u, CYBER_KILL_SUCCEEDED, u2->nick);
       canalopers(s_CyberServ, "12%s KILLea a 12%s (%s)", u->nick, u2->nick, motivo);
   }
}
/*************************************************************************/
static void do_cybergline(User *u)
{

   char *nick = strtok(NULL, " ");
   char *motivo = strtok(NULL, "");
        
   User *u2 = NULL;



   if (!is_cyber_admin(u)) {
       notice_lang(s_CyberServ, u, CYBER_NO_ADMIN_CYBER);   
       return;
   }
    
   if (!nick) {
       syntax_error(s_CyberServ, u, "GLINE", CYBER_GLINE_SYNTAX);
       return;
   }    

   if (motivo) {
       char buf[BUFSIZE];
       snprintf(buf, sizeof(buf), "%s -> %s", s_CyberServ, motivo);
       motivo = sstrdup(buf);
   } else {
       char buf[BUFSIZE];
       snprintf(buf, sizeof(buf), "%s -> G-Lined", s_CyberServ);
       motivo = sstrdup(buf);    
   }    
   
   u2 = finduser(nick);   

   if (!u2) {
       notice_lang(s_CyberServ, u, NICK_X_NOT_IN_USE, nick);
       return;
   }
                    
   if (stricmp(u2->host, u->host) != 0) {
       notice_lang(s_CyberServ, u, CYBER_NICK_NOT_IN_CYBER, u2->nick);        
       return;
   } else {   
/* BUG: mirar esto, mirar el username que mete los glines mal   */
       kill_user(s_CyberServ, u2->nick, motivo);
       notice_lang(s_CyberServ, u, CYBER_GLINE_SUCCEEDED, u2->nick);     
       send_cmd(NULL, "GLINE * +%s@%s 300 :%s", u2->username, u2->host, motivo);
       canalopers(s_CyberServ, "12%s ha GLINEado a 12%s", u->nick, u2->nick);
   }    
}
/*************************************************************************/
static void do_cyberunban(User *u)
{

   char *chan = strtok(NULL, " ");
   Channel *c;
   int i;
   char *av[3];

   if (!is_cyber_admin(u)) {
       notice_lang(s_CyberServ, u, CYBER_NO_ADMIN_CYBER);       
       return;
   }

   if (!chan) {
       syntax_error(s_CyberServ, u, "UNBAN", CYBER_UNBAN_SYNTAX);
       return;
   }
   
   if (!(c = findchan(chan))) {
       notice_lang(s_CyberServ, u, CHAN_X_NOT_IN_USE, chan);
   } else {

       int count = c->bancount;
       int teniaban = 0;       
       char **bans = smalloc(sizeof(char *) * count);
       memcpy(bans, c->bans, sizeof(char *) * count);         
       
       av[0] = chan;
       av[1] = sstrdup("-b");
       for (i = 0; i < count; i++) {
           if (match_usermask(bans[i], u)) {
               send_cmd(MODE_SENDER(s_CyberServ), "MODE %s -b %s",
                          chan, bans[i]);       
               av[2] = sstrdup(bans[i]);
               do_cmode(s_CyberServ, 3, av);
               free(av[2]);
               teniaban = 1;
           }    
       }    
       free(av[1]);
       if (teniaban)
           notice_lang(s_CyberServ, u, CYBER_UNBAN_SUCCEEDED, chan);
       else
           notice_lang(s_CyberServ, u, CYBER_UNBAN_FAILED, chan);
       free(bans);
   }
}