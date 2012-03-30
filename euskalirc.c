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
      
  

    if ((!cmd) || ((!stricmp(cmd, "ACEPTA") == 0) && (!stricmp(cmd, "RECHAZA") == 0) && (!stricmp(cmd, "ATENDIDO") == 0)&& (!stricmp(cmd, "DESATIENDE") == 0))) {
      	 notice_lang(s_OperServ, u, EUSKALIRC_HELP_DUDA);
    	return;
    }
   
    if (!nick) {
    	notice_lang(s_OperServ, u, EUSKALIRC_FALTA_NICK,s_OperServ);
	 return;
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_OperServ, u, NICK_X_NOT_REGISTERED, nick);
	return;
    } else  if (!(ni->status & NI_ON_BDD)) {
        notice_lang(s_OperServ, u, NICK_MUST_BE_ON_BDD);
	return;
    }
     


      ni = findnick(nick);
   
      if  ((stricmp(cmd, "ACEPTA") == 0) &&  (ni->in_ayu & AYU_PROCESO)) {
	if (!is_services_oper(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	} else if (ni && (ni->status & NS_IDENTIFIED)) {
         privmsg(s_EuskalIRCServ, nick, "El OPERador/a 5%s se pondrá en contacto contigo en breve.Por favor, abandone el canal una vez atendido. Gracias.",u->nick);
	canalopers( s_EuskalIRCServ,"12OPER 4%s 3ACEPTA DUDA de  2%s",u->nick,nick);
	ni->in_ayu   &= ~AYU_PROCESO;
	ni->in_ayu |= AYU_ACEPTA ;
	return;
            } 
            else {
               privmsg(s_OperServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
		return;
              }
            }
       
       else if ((stricmp(cmd, "RECHAZA") == 0) && (ni->in_ayu & AYU_PROCESO))  {
	if (!is_services_oper(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}  else if (ni && (ni->status & NS_IDENTIFIED)) {
          privmsg(s_EuskalIRCServ, nick , "En este momento no hay ningún OPERador/a disponible.Inténtelo más tarde",u->nick);
          canalopers( s_EuskalIRCServ,"12OPER 4%s 5RECHAZA DUDA de 2%s",u->nick,nick);
	ni->in_ayu   &= ~AYU_PROCESO;
	ni->in_ayu |= AYU_RECHAZA ;
        send_cmd(s_EuskalIRCServ, "MODE #%s -v %s", CanalAyuda, ni->nick);
        return;
	  }
          else  {
               privmsg(s_EuskalIRCServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
		return; }

          }

       else if ((stricmp(cmd, "ATENDIDO") == 0) && (ni->in_ayu & AYU_ACEPTA))  {
	if (!is_services_oper(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}  else if (ni && (ni->status & NS_IDENTIFIED)) {
          privmsg(s_EuskalIRCServ, nick , "Por favor.Recuerde abandonar el canal,una vez que haya sido atendido/a.Si aún no se le resolvió el problema planteado, o deseara realizar otra consulta,deberá igualmente salir previamente del canal (2#%s) y volver a entrar, para asignarle nuevamente otro representante de servicios. Gracias.",CanalAyuda);
          canalopers( s_EuskalIRCServ,"12OPER 4%s 5ATIENDE DUDA de  2%s",u->nick,nick);
	ni->in_ayu   &= ~AYU_ACEPTA;
	ni->in_ayu |= AYU_ATENDIDO;
        send_cmd(s_EuskalIRCServ, "MODE #%s -v %s", CanalAyuda, ni->nick); 
        return;
	  }
          else  {
               privmsg(s_EuskalIRCServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
		return; }

          }
        else if ((stricmp(cmd, "DESATIENDE") == 0) && (ni->in_ayu & AYU_ACEPTA))  {
	if (!is_services_oper(u)) {
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);
	    return;
	}  else if (ni && (ni->status & NS_IDENTIFIED)) {
          privmsg(s_EuskalIRCServ, nick , "OPER 4%s deja de estar disponible.En breve te informaré del nick del OPERador/a que te va a ayudar. Por favor, no abandones el canal mientras eres atendido/a",u->nick);
          canalopers( s_EuskalIRCServ,"12OPER 4%s 5DESATIENDE DUDA de  2%s",u->nick,nick);
	ni->in_ayu   &= ~AYU_ACEPTA;
	ni->in_ayu |= AYU_PROCESO ;
        return;
	  }
          else  {
               privmsg(s_EuskalIRCServ,u->nick, "El Nick 5%s No esta ONLINE.",ni->nick);
		return; }

          }
        
if ((stricmp(cmd, "ACEPTA") == 0) && !(ni->in_ayu & AYU_PROCESO))  { 
    
       privmsg(s_OperServ,u->nick, " 2El Nick 12%s 2Ni Solicita ni Precisa Asistencia!.5 ACEPTACION3 ilógica!",ni->nick);
	return;
     }
 else if ((stricmp(cmd, "RECHAZA") == 0) && !(ni->in_ayu & AYU_PROCESO))  { 
    
       privmsg(s_OperServ,u->nick, " 2El Nick 12%s 2Ni Solicita ni Precisa Asistencia!.5 RECHAZO3 ilógico!",ni->nick);
	return;
     }
  else if ((stricmp(cmd, "ATENDIDO") == 0) && !(ni->in_ayu & AYU_ACEPTA))  { 
    
       privmsg(s_OperServ,u->nick, " 2El Nick 12%s 2No estaba siendo atendido!.5 Finalización del soporte3 ilógico!",ni->nick);
	return;
     }
  else if ((stricmp(cmd, "DESATIENDE") == 0) && !(ni->in_ayu & AYU_ACEPTA))  { 
    
       privmsg(s_OperServ,u->nick, " 2El Nick 12%s 2No estaba siendo atendido!.5 Abandono del soporte3 ilógico!",ni->nick);
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
  
   if ((ni = u->real_ni)) {
   if (ni->in_ayu & AYU_ENTRA) {
	canalopers( s_EuskalIRCServ,"5CONSULTA! 12%s : 2%s", source,buf);
   send_cmd(s_EuskalIRCServ, "MODE #%s +v %s", CanalAyuda, ni->nick);
   privmsg(s_EuskalIRCServ, ni->nick , "Gracias, en breve te informaré del nick del OPERador/a que te va a ayudar. Por favor, no abandones el canal mientras eres atendido/a");
    ni->in_ayu   &= ~AYU_ENTRA;
   ni->in_ayu |= AYU_PROCESO ;
      }
   
   }
}
void euskalirc_canal(const char *source,const char *chan, char *buf)
 
 {
 
 char opr[BUFSIZE];
char ayu[BUFSIZE];
snprintf(opr, sizeof(opr), "#%s", CanalOpers);
snprintf(ayu, sizeof(ayu), "#%s", CanalAyuda);
   
 
if (!strcmp(chan, opr))  return;
if (!strcmp(chan, ayu)) {
	        privmsg(s_EuskalIRCServ, opr , "Ey! %s me dijo en el canal %s 4%s !!", source,ayu,buf);
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
	 ni->in_ayu = 0;
          ni->in_ayu |= AYU_ENTRA;
	canalopers( s_EuskalIRCServ,"4%s Entra para solicitar soporte",source);
	 send_helperchan_users(source);
	             }
        else if  (!is_services_oper(u)) {
	notice_lang(s_EuskalIRCServ, u, EUSKALIRC_MENSAJE_NOREG,source,s_HelpServ,s_NickServ);
         return;
          } 
	send_helpers_aviso(source);
     
}
