/* Servicio EuskalIRC, botshispanobdd
 *
 * (C) 2009 donostiarra  admin.euskalirc@gmail.com  http://euskalirc.wordpress.com
 *
 * Este programa es software libre. Puede redistribuirlo y/o modificarlo
 * bajo los términos de la Licencia Pública General de GNU según es publicada
 * por la Free Software Foundation.
 */
#include "services.h"
#include "pseudo.h"
 /* Main EuskalServ routine. */

void do_euskal(User *u) /*la colocamos en extern.h y asi la llamamos desde oper*/
{    
        char *cmd, *nick;
        cmd = strtok(NULL, " ");
        nick = strtok(NULL, " ");
        NickInfo *ni;
  
    char adm[BUFSIZE];
      
  

    if ((!cmd) || ((!stricmp(cmd, "ACEPTA") == 0) && (!stricmp(cmd, "RECHAZA") == 0))) {
       privmsg(s_EuskalIRCServ,u->nick, " /msg %s 2DUDA 12ACEPTA/RECHAZA 5<NICK>",s_OperServ);
    	return;
    }
   
    if (!nick) {
    	privmsg(s_EuskalIRCServ, u-> nick, "4Falta un Nick /msg %s 2DUDA 12ACEPTA/RECHAZA 5<NICK>",s_OperServ);
    	return;
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_EuskalIRCServ, u, NICK_X_NOT_REGISTERED, nick);
	return;
    } else  if (!(ni->status & NI_ON_BDD)) {
        notice_lang(s_EuskalIRCServ, u, NICK_MUST_BE_ON_BDD);
	return;
    }
     


      ni = findnick(nick);
   
      if  ((stricmp(cmd, "ACEPTA") == 0) &&  (ni->in_ayu & AYU_SI)) {
	if (!is_services_oper(u)) {
	    notice_lang(s_EuskalIRCServ, u, PERMISSION_DENIED);
	    return;
	} else if (ni && (ni->status & NS_IDENTIFIED)) {
          privmsg(s_EuskalIRCServ, nick, "El OPERador/a 5%s se pondrá en contacto contigo en breve.Por favor, abandone el canal una vez atendido. Gracias.",u->nick);
	canaladmins( s_EuskalIRCServ,"12OPER 4%s 3ACEPTA DUDA de  2%s",u->nick,nick);
	ni->in_ayu = AYU_NO ;
	return;
            } 
            else {
               privmsg(s_EuskalIRCServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
		return;
              }
            }
       
       else if ((stricmp(cmd, "RECHAZA") == 0) && (ni->in_ayu & AYU_SI))  {
	if (!is_services_oper(u)) {
	    notice_lang(s_EuskalIRCServ, u, PERMISSION_DENIED);
	    return;
	}  else if (ni && (ni->status & NS_IDENTIFIED)) {
          privmsg(s_EuskalIRCServ, nick , "El OPERador/a 5%s ha rechazado la solicitud de ayuda.",u->nick);
          canaladmins( s_EuskalIRCServ,"12OPER 4%s 5RECHAZA DUDA de  2%s",u->nick,nick);
	ni->in_ayu = AYU_NO ;
        return;
	  }
          else  {
               privmsg(s_EuskalIRCServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
		return; }

          }
if ((stricmp(cmd, "ACEPTA") == 0) && (ni->in_ayu & AYU_NO))  { 
    
       privmsg(s_EuskalIRCServ,u->nick, " 2El Nick 12%s 2Ni Solicita ni Precisa Asistencia!.5 ACEPTACION3 ilógica!",ni->nick);
	return;
     }
 else if ((stricmp(cmd, "RECHAZA") == 0) && (ni->in_ayu & AYU_NO))  { 
    
       privmsg(s_EuskalIRCServ,u->nick, " 2El Nick 12%s 2Ni Solicita ni Precisa Asistencia!.5 RECHAZO3 ilógico!",ni->nick);
	return;
     }
       
}

void euskalserv(const char *source, char *buf)
{
    int num;
    NickInfo *ni;
    char *cmd;
    char *s;
    User *u = finduser(source);

    ni = findnick(source);
    char ayu[BUFSIZE];
snprintf(ayu, sizeof(ayu), "#%s", CanalAyuda);

    if (!u) {
	log("%s: user record for %s not found", s_EuskalIRCServ, source);
	notice(s_EuskalIRCServ, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    //log("yiha %s: %s: %s", s_EuskalIRCServ, source, buf);
  canaladmins( s_EuskalIRCServ,"5CONSULTA! 12%s : 2%s", source,buf);
   if ((ni = u->real_ni))
   if (ni->in_ayu & AYU_NO) {
   send_cmd(s_EuskalIRCServ, "MODE #%s +v %s", CanalAyuda, ni->nick);
   privmsg(s_EuskalIRCServ, ni->nick , "Gracias, en breve te informaré del nick del OPERador/a que te va a ayudar. Por favor, no abandones el canal mientras eres atendido/a");
   ni->in_ayu = AYU_SI ;
   }
   
}
void euskalirc_canal(const char *source,const char *chan, char *buf)
 
 {
 
 char adm[BUFSIZE];
char ayu[BUFSIZE];
snprintf(adm, sizeof(adm), "#%s", CanalAdmins);
snprintf(ayu, sizeof(ayu), "#%s", CanalAyuda);
   
 
if (!strcmp(chan, adm))  return;
if (!strcmp(chan, ayu)) {
	        privmsg(s_EuskalIRCServ, adm , "Ey! %s me dijo en el canal %s 4%s !!", source,ayu,buf);
			return;
          }
		
	
	
 }


 void mandar_mensaje(const char *source)
 {
    NickInfo *ni;
    User *u = finduser(source);
     ni = findnick(source);
        char ayu[BUFSIZE];
     snprintf(ayu, sizeof(ayu), "#%s", CanalAyuda);

          
      if ((ni = u->real_ni) && !(is_services_oper(u))) {
        ni->in_ayu = AYU_NO;
        privmsg(s_EuskalIRCServ,source, "Hola %s",source);
	privmsg(s_EuskalIRCServ,source, "Soy el encargado de ponerte en contacto con un OPER de Servicios.");
	privmsg(s_EuskalIRCServ,source, "  5,15 Por favor, ¿podrías describirme en una línea el problema?");
         }
        else if  (!is_services_oper(u)) {
          privmsg(s_EuskalIRCServ,source, "Buenas 2%s5",source);
	  privmsg(s_EuskalIRCServ,source, "Veo que no tienes el Nick Registrado.");
          privmsg(s_EuskalIRCServ,source, "Para Solicitar Soporte debe Registrarse Primero.");
          privmsg(s_EuskalIRCServ,source, "2Instrucciones de Registro: lo encontrara ejecutando el comando 4/msg 2%s %s",s_HelpServ,s_NickServ);
            return;
          } 
   if   (is_services_root(u))  {
    privmsg(s_EuskalIRCServ,ayu, "Hola 4%s,BienVenido/a 5Root de Red ",source);
    return;
     }
     else if (is_services_admin(u))  {
    privmsg(s_EuskalIRCServ,ayu, "Hola 4%s,BienVenido/a 5Administrador de Red ",source);
    return;
     }
     else if (is_services_cregadmin(u))  {
    privmsg(s_EuskalIRCServ,ayu, "Hola 4%s,BienVenido/a 5Co-Administrador de Red ",source);
    return;
     }
    else if (is_services_devel(u))  {
    privmsg(s_EuskalIRCServ,ayu, "Hola 4%s,BienVenido/a 5Devel de Red ",source);
    return;
    }
    else if (is_services_oper(u))  {
    privmsg(s_EuskalIRCServ,ayu, "Hola 4%s,BienVenido/a 5Operador de Red ",source);
    return;
    }
     
}
