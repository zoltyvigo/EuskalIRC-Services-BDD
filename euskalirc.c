/* Servicio EuskalIRC, botshispanobdd
 *
 * (C) 2009 donostiarra 
 *
 * Este programa es software libre. Puede redistribuirlo y/o modificarlo
 * bajo los términos de la Licencia Pública General de GNU según es publicada
 * por la Free Software Foundation.
 */
#include "services.h"
#include "language.h"
#include <sys/stat.h>
#include "pseudo.h"
 /* Main EuskalServ routine. */


void do_euskal(User *u)
{
char *cmd, *nick;
    NickInfo *ni;
   cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";
nick = strtok(NULL, " ");
    
   if (stricmp(cmd, "ACEPTA") == 0) {
	if (!is_services_oper(u)) {
	    notice_lang(s_EuskalIRCServ, u, PERMISSION_DENIED);
	    return;
	} else 
          privmsg(s_EuskalIRCServ, nick, "El OPERador/a 5%s se pondrá en contacto contigo en breve.Por favor, abandone el canal una vez atendido. Gracias.",u->nick);
	
            }

       if (stricmp(cmd, "RECHAZA") == 0) {
	if (!is_services_oper(u)) {
	    notice_lang(s_EuskalIRCServ, u, PERMISSION_DENIED);
	    return;
	}  else
          privmsg(s_EuskalIRCServ, nick , "El OPERador/a 5%s ha rechazado la solicitud de ayuda.",u->nick);
	
            }
}


void euskalserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	log("%s: user record for %s not found", s_EuskalIRCServ, source);
	notice(s_EuskalIRCServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    //log("yiha %s: %s: %s", s_EuskalIRCServ, source, buf);
  canaladmins( s_EuskalIRCServ,"Ey! %s me dijo al privi 4%s !!", source,buf);
  send_cmd(s_EuskalIRCServ, "MODE #%s +v %s", CanalCybers, source);
   privmsg(s_EuskalIRCServ, source , "Gracias, en breve te informaré del nick del OPERador/a que te va a ayudar. Por favor, no abandones el canal mientras eres atendido/a");

    cmd = strtok(buf, " ");
    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_EuskalIRCServ, source, "\1PING %s", s);
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_EuskalIRCServ, source, "\1VERSION %s %s -- %s\1",
               PNAME, s_EuskalIRCServ, version_build);                
    }/* else {
	run_cmd(s_EuskalIRCServ, u, cmds, cmd); }*/
   
}
void euskalirc_canal(const char *source,const char *chan, char *buf)
 
 {
 
 char adm[BUFSIZE];
char cyb[BUFSIZE];
snprintf(adm, sizeof(adm), "#%s", CanalAdmins);
snprintf(cyb, sizeof(cyb), "#%s", CanalCybers);
   
 
if (!strcmp(chan, adm))  return;
if (!strcmp(chan, cyb)) {
	        privmsg(s_EuskalIRCServ, adm , "Ey! %s me dijo en el canal %s 4%s !!", source,cyb,buf);
			return;
          }
		
	
	
 }
    
 void mirar_pregunta(const char *source, char *buf[BUFSIZE])
 {
 int i;
 User *u = finduser(source);
 const char *cmd = strtok(NULL, "");
if (!cmd) {
	//notice_help(s_OperServ, u, OPER_HELP);
    } else {
	/*help_cmd(s_OperServ, u, cmds, cmd);*/
    }

       //canalopers( s_EuskalIRCServ,"Ey! %s me dijo al privi 4%s !!", source,buf);
			return;

 }

 void mandar_mensaje(const char *source)
 {
 
    time_t ahora = time(NULL);
    time_t caducado;
    struct tm *tm;
   
	privmsg(s_EuskalIRCServ,source, "Hola %s",source);
	privmsg(s_EuskalIRCServ,source, "Soy el encargado de ponerte en contacto con un OPER de Servicios.");
	privmsg(s_EuskalIRCServ,source, "  5,15 Por favor, ¿podrías describirme en una línea el problema?");

}

