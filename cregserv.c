/* Funciones CReGServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * Modulo CregServ hecho por zoltan <zolty@ctv.es>
 *
 */
 
/*************************************************************************/
 
#include "services.h"
#include "pseudo.h"
 
/*************************************************************************/
 


static void do_help(User *u);
static void do_registra(User *u);
static void do_apolla(User *u);


static Command cmds[] = {
    { "HELP",       do_help,     NULL,  -1,                      -1,-1,-1,-1 },
    { "AYUDA",      do_help,     NULL,  -1,                      -1,-1,-1,-1 },    
    { "REGISTRA",   do_registra, NULL,  CREG_HELP_REGISTRA,      -1,-1,-1,-1 },
    { "APOYA",      NULL,        NULL,  CREG_HELP_APOYA,         -1,-1,-1,-1 },
    { "APOLLA",     do_apolla,   NULL,  -1,                      -1,-1,-1,-1 },       
    { "ESTADO",     NULL,        NULL,  CREG_HELP_ESTADO,        -1,-1,-1,-1 },
    { "APOYOS",     NULL,        NULL,  CREG_HELP_APOYOS,         -1,-1,-1,-1 },        
    { NULL }
};



/*************************************************************************/

/* Main NickServ routine. */

void cregserv(const char *source, char *buf)
{
    char *cmd, *s;
    User *u = finduser(source);
        
    if (!u) {
        log("%s: user record for %s not found", s_CregServ, source);
        notice(s_CregServ, source,
            getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
        return;
    }
    
    cmd = strtok(buf, " ");
    if (!cmd) {
        return;
    } else if (stricmp(cmd, "\1PING") == 0) {
        if (!(s = strtok(NULL, "")))
            s = "\1";
        notice(s_CregServ, source, "\1PING %s", s);
    } else if (skeleton) {
        notice_lang(s_CregServ, u, SERVICE_OFFLINE, s_CregServ);
    } else {
        if (!u->ni && stricmp(cmd, "HELP") != 0)
            notice_lang(s_CregServ, u, NICK_NOT_REGISTERED_HELP, s_CregServ);
        else
            run_cmd(s_CregServ, u, cmds, cmd);
    }
}

/*************************************************************************/
                                                                                                                                                                    
/* Return a help message. */

static void do_help(User *u)
{
    char *cmd = strtok(NULL, "");
    
    if (!cmd) {
        notice_help(s_CregServ, u, CREG_HELP);
        
        if (is_services_admin(u))
            notice_help(s_CregServ, u, CREG_SERVADMIN_HELP);                                   
    } else {
        help_cmd(s_CregServ, u, cmds, cmd);
    }
}
/*************************************************************************/

static void do_registra(User *u)
{
    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    char *desc = strtok(NULL, "");

    NickInfo *ni = u->ni;

    if (readonly) {
        notice_lang(s_CregServ, u, CHAN_REGISTER_DISABLED);
        return;
    }
                        
    if (!desc) {
         syntax_error(s_CregServ, u, "REGISTRA", CHAN_REGISTER_SYNTAX);
    } else if (*chan == '&') {
         notice_lang(s_CregServ, u, CHAN_REGISTER_NOT_LOCAL);
    } else if (!(*chan == '#')) {
         privmsg(s_CregServ, u->nick, "Canal no valido para registrar");                       
    } else if (!ni) {
         notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
         notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK,
                s_NickServ, s_NickServ);
    } else {
        privmsg(s_CregServ, u->nick, "Canal 12%s aceptado para el proceso de registro", chan);
        privmsg(s_CregServ, u->nick, "DEBUG pass: %s descripcion: %s", pass, desc);
    }
}                                                                        
                                                        
                                                        
                                                        
/*************************************************************************/

static void do_apolla(User *u)
{
  
        privmsg(s_CregServ, u->nick, "No, que duele mucho (Prueba con  APOYA)");
        wallops(s_CregServ, "12%s ha ejecutado el comando APOLLA, hehehe", u->nick);
}
        
                                                                                                                                                                          