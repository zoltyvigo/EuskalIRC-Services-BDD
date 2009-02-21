/* OperServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"
#include "pseudo.h"
typedef struct ipv_ IpvInfo;
struct ipv_ {
    int32 numero;
    char *cadena;
    char titular[NICKMAX];
    
};
static int32 ipvirt = 0;
static int32 ipvirt_size = 0;
static IpvInfo *ipvirtual = NULL;

static int add_ipv(User *u, const char *text);
static int del_ipv(int num);

static void do_ipv_add(User *u);
static void do_ipv_del(User *u);

static void do_ipv_list(User *u);
static void do_ipv_usar(User *u);
static void do_credits (User *u);
static void do_vhost (User *u);
static void do_cambia_vhost (User *u);
static void do_help (User *u);
static void do_set(User *u);

static void do_set_vhost(User *u, NickInfo *ni, char *param);



/*************************************************************************/

static Command cmds[] = {
    { "CREDITS",    do_credits,    NULL,  -1,                   -1,-1,-1,-1 },
    { "ACTIVAR",	    do_vhost,	   is_services_admin,   IPV_HELP_ACTIVAR,   -1,-1,-1,-1 },
     { "CAMBIAR",	    do_cambia_vhost,	   NULL,   IPV_HELP_CAMBIAR,   -1,-1,-1,-1 },
    { "CREDITOS",   do_credits,    NULL,  -1,                   -1,-1,-1,-1 },        
    { "HELP",       do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "AYUDA",      do_help,       NULL,  -1,                   -1,-1,-1,-1 },
    { "SHOWCOMMANDS",    do_help,  NULL,  -1,                   -1,-1,-1,-1 },
    { "SET VHOST", NULL,       NULL,  NICK_HELP_SET_VHOST,    -1,-1,-1,-1 },
    { "SET",      do_set,     is_services_admin,  IPV_HELP_SET,
		-1, IPV_SERVADMIN_HELP_SET,
		IPV_SERVADMIN_HELP_SET, IPV_SERVADMIN_HELP_SET },
     { "PONER",       do_ipv,       NULL,  -1,      -1,-1,-1,-1 },
     { "USAR",       do_ipv_usar,       NULL,  -1,      -1,-1,-1,-1 },
   
    /* Fencepost: */
    { NULL }
};



/* Main OperServ routine. */

void ipvserv(const char *source, char *buf)
{
    char *cmd;
    char *s;
    User *u = finduser(source);

    if (!u) {
	log("%s: user record for %s not found", s_IpVirtual, source);
	notice(s_IpVirtual, source,
		getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
	return;
    }

    log("%s: %s: %s", s_IpVirtual, source, buf);

    cmd = strtok(buf, " ");
    if (!cmd) {
	return;
    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_IpVirtual, source, "\1PING %s", s);
    } else if (stricmp(cmd, "\1VERSION\1") == 0) {
        notice(s_IpVirtual, source, "\1VERSION %s %s -- %s\1",
               PNAME, s_IpVirtual, version_build);                
    } else {
	run_cmd(s_IpVirtual, u, cmds, cmd);
    }
}

/*************************************************************************/
/*********************** Spam item loading/saving ************************/
/*************************************************************************/

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Read error on %s", SpamDBName);	\
	ipvirt = i;					\
	break;						\
    }							\
} while (0)

void load_ipv()
{
    dbFILE *f;
    int i;
    int16 n;
    int32 tmp32;

    if (!(f = open_db(s_IpVirtual, IpVirtualDBName, "r")))
	return;
    switch (i = get_file_version(f)) {
      case 8:
      case 7:
	SAFE(read_int16(&n, f));
	ipvirt = n;
	if (ipvirt < 8)
	    ipvirt_size = 16;
	else if (ipvirt >= 16384)
	    ipvirt_size = 32767;
	else
	    ipvirt_size = 2*ipvirt;
	ipvirtual = smalloc(sizeof(*ipvirtual) * ipvirt_size);
	if (!ipvirt) {
	    close_db(f);
	    return;
	}
	for (i = 0; i < ipvirt; i++) {
	    SAFE(read_int32(&ipvirtual[i].numero, f));  //posiciones totales
	    SAFE(read_string(&ipvirtual[i].cadena, f));
	    SAFE(read_buffer(ipvirtual[i].titular, f));
	   
	}
	break;

      default:
	fatal("Unsupported version (%d) on %s", i, IpVirtualDBName);
    } /* switch (ver) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {						\
    if ((x) < 0) {						\
	restore_db(f);						\
	log_perror("Write error on %s", IpVirtualDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {		\
	    canalopers(NULL, "Write error on %s: %s", IpVirtualDBName,	\
			strerror(errno));			\
	    lastwarn = time(NULL);				\
	}							\
	return;							\
    }								\
} while (0)

void save_ipv()
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_IpVirtual, IpVirtualDBName, "w")))
	return;
    SAFE(write_int16(ipvirt, f));
    for (i = 0; i < ipvirt; i++) {
	SAFE(write_int32(ipvirtual[i].numero, f));
	SAFE(write_string(ipvirtual[i].cadena, f));
	SAFE(write_buffer(ipvirtual[i].titular, f));

    }
    close_db(f);
}

#undef SAFE

void do_ipv(User *u)
{
    int is_servadmin = is_services_admin(u);
    char *cmd = strtok(NULL, " ");
    char *typename;
    int *msgs;
    NickInfo *ni;
    char  *nick;

if (!cmd) {
	char buf[32];
	#ifdef IRC_UNDERNET_P10
	privmsg(s_IpVirtual, u->numerico, "Sintaxis: PONER ADD/DEL/LIST [param]");
	#else
	privmsg(s_IpVirtual, u->nick,"Sintaxis: PONER ADD/DEL/LIST [param]");
	#endif
    	cmd = "";
       }
 if (stricmp(cmd, "HELP") == 0) {
	char buf[32];
	#ifdef IRC_UNDERNET_P10
	privmsg(s_IpVirtual, u->numerico, "Sintaxis: PONER ADD/DEL/LIST [param]");
	#else
	privmsg(s_IpVirtual, u->nick,"Sintaxis: PONER ADD/DEL/LIST [param]");
	#endif
    	cmd = "";
       }	

    if (stricmp(cmd, "LIST") == 0) {
	do_ipv_list(u);

    } else if (stricmp(cmd, "ADD") == 0) {
     
          if (!(ni = findnick(u->nick))) {
		notice_lang(s_IpVirtual, u, NICK_X_NOT_REGISTERED, u->nick);
		return;
		}
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_IpVirtual, u->nick, "No pueden añadir los nicks que no esten migrados a la BDD");
	       return;	 
		 }
             
         do_ipv_add(u);
	/*if (is_servadmin)
	   	else
	    notice_lang(s_IpVirtual, u, PERMISSION_DENIED);*/

       } 
	
    if (stricmp(cmd, "DEL") == 0) {
       
        if (!(ni = findnick(u->nick))) {
		notice_lang(s_IpVirtual, u, NICK_X_NOT_REGISTERED, u->nick);
		return;
		}
	    if (!(ni->status & NI_ON_BDD)) {
	       privmsg(s_IpVirtual, u->nick, "No pueden borrar los nicks que no esten migrados a la BDD");
	       return;	 
		 }
		do_ipv_del(u); 
		} /* else {
        #ifdef IRC_UNDERNET_P10
	privmsg(s_IpVirtual, u->numerico, "Sintaxis: PONER ADD/LIST [param]");
	#else
	privmsg(s_IpVirtual, u->nick,"Sintaxis: PONER ADD/LIST [param]");
	#endif 
	
}*/
}
static int del_ipv(int num)
{
    int i;
    int count = 0;

    for (i = 0; i < ipvirt; i++) {
	if (num == 0 || ipvirtual[i].numero == num) {
	    free(ipvirtual[i].cadena);
	   
	    count++;
	    ipvirt--;
	    if (i < ipvirt)
		memcpy(ipvirtual+i, ipvirtual+i+1, sizeof(*ipvirtual) * (ipvirt-i));
	    i--;
	}
    }
    return count;
}


static int add_ipv(User *u, const char *text)
{
    int i, num;

    if (ipvirt >= 32767)
	return -1;

    if (ipvirt >= ipvirt_size) {
	if (ipvirt_size < 8)
	    ipvirt_size = 8;
	else
	    ipvirt_size *= 2;
	ipvirtual = srealloc(ipvirtual, sizeof(*ipvirtual) * ipvirt_size);
    }
    num = 0;
    for (i = ipvirt-1; i >= 0; i--) {
	
	    num = ipvirtual[i].numero;
	    break;
	
    }
    

    ipvirtual[ipvirt].numero = num+1;
    ipvirtual[ipvirt].cadena = sstrdup(text);
    strscpy(ipvirtual[ipvirt].titular, u->nick, NICKMAX);
    ipvirt++;
    return num+1;
}

static void do_ipv_add(User *u)
{
    char *text = strtok(NULL, "");

    if (!text) {
	char buf[32];
	#ifdef IRC_UNDERNET_P10
	privmsg(s_IpVirtual, u->numerico, "Sintaxis: PONER ADD/DEL/LIST [param]");
	#else
	privmsg(s_IpVirtual, u->nick,"Sintaxis: PONER ADD/DEL/LIST [param]");
	#endif
    } else {
	int n = add_ipv(u, text);
	if (n < 0)
	    #ifdef IRC_UNDERNET_P10
	    privmsg(s_IpVirtual, u->numerico, "La lista esta llena.");
	   #else
	    privmsg(s_IpVirtual, u->nick,"La lista esta llena.");
	    #endif
	else
	     #ifdef IRC_UNDERNET_P10
	    privmsg(s_IpVirtual, u->numerico, "Se ha añadido a la lista de ips.");
	    #else
	    privmsg(s_IpVirtual, u->nick, "Se ha añadido a la lista de ips.");
	     #endif
	    
	if (readonly)
	    notice_lang(s_IpVirtual, u, READ_ONLY_MODE);
    }
}

static void do_ipv_list(User *u)
{
    int i = 0;
    int cont;
     int is_servadmin = is_services_admin(u);
    char timebuf[64];
    char *cmd = strtok(NULL, " ");
    #ifdef IRC_UNDERNET_P10
    privmsg(s_IpVirtual, u->numerico, "ID :: Texto :: Agregado por");
    #else
    privmsg(s_IpVirtual, u->nick,"ID :: Texto :: Agregado por");
    #endif
   cont=0;
        if ((!cmd) || !(is_servadmin))
	cmd = u->nick;

    for (i = 0; i < ipvirt; i++) {
        cont++;
       #ifdef IRC_UNDERNET_P10
	   if (stricmp(cmd, ipvirtual[i].titular) == 0) { 
 privmsg(s_IpVirtual, u->numerico, "%d [ %d ] - '%s' - '%s'", cont, ipvirtual[i].numero,ipvirtual[i].cadena, *ipvirtual[i].titular ? ipvirtual[i].titular : "<unknown>");
 }
        #else
	 if (stricmp(cmd, ipvirtual[i].titular) == 0) {
 privmsg(s_IpVirtual, u->nick,"%d [ %d ] - '%s' - '%s'", cont,  ipvirtual[i].numero,ipvirtual[i].cadena, *ipvirtual[i].titular ? ipvirtual[i].titular : "<unknown>");
	  }
	 #endif
       }
    
        #ifdef IRC_UNDERNET_P10
	privmsg(s_IpVirtual, u->numerico, "Fin del Listado.");
	#else
	 privmsg(s_IpVirtual, u->nick,"Fin del Listado.");
	 #endif
	 
	
}
static void do_ipv_usar(User *u)
{
     int i = 0;
    static const char karak[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.";
    time_t oraingoa = time(NULL);
    time_t kadukatu;
    struct tm *tm;
    char timebuf[32];
    NickInfo *ni;
     char *cmd;
    char *cadena;
     char *text = strtok(NULL, "");
     int vhost;
    
      
      if (!text) {
	privmsg(s_IpVirtual, u->nick,"Sintaxis: USAR [numero]");
	  return;
       }
       else {
       cmd = u->nick;
       vhost=atoi(text);
	vhost--;
	 cadena=ipvirtual[vhost].cadena; //para que no tenga que copiar toda la linea

       if (stricmp(cmd, ipvirtual[vhost].titular) == 0) {
          
        
        //do_usar_vhost(cmd,ipvirtual[vhost].cadena);
 ni = findnick(cmd);
   
if (oraingoa < ni->time_vhost)
{

tm = localtime(&ni->time_vhost);

strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Solamente se admite un cambio cada 1m. Podras cambiar el vhost a la hora: %s", timebuf);
#else
privmsg(s_IpVirtual, u->nick, "Solamente se admite un cambio cada 1m. Podras cambiar el vhost a la hora: %s", timebuf);
#endif
return;
}

if (strlen(cadena) > 125) {
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Como maximo puede tener 124 caracteres.");
#else
privmsg(s_IpVirtual, u->nick, "Como maximo puede tener 124 caracteres.");
#endif
return;
}
if ((strstr(cadena, "root")) || (strstr(cadena, "admin")) || (strstr(cadena, "coadmin")) || (strstr(cadena, "devel")) || (strstr(cadena, "oper")) || (strstr(cadena, "patrocina")) || (strstr(cadena, "euskalirc")) || (strstr(cadena, "net")) || (strstr(cadena, "com")) || (strstr(cadena, "org")) || (strstr(cadena, "tk")) || (strstr(cadena, "cat")) || (strstr(cadena, "eu")) || (strstr(cadena, "hisp")) || (strstr(cadena, "http")) || (strstr(cadena, "www")) || (strstr(cadena, "ircop")) || (strstr(cadena, "help"))) {
 privmsg(s_IpVirtual, u->nick, "contiene palabras no permitidas");
 return;
 }
if (cadena[strspn(cadena, karak)]) {
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "El vhost solamente admite caracteres entre a-z, y 0-9 incluidas el guion y el punto.");
#else
privmsg(s_IpVirtual, u->nick, "El vhost solamente admite caracteres entre a-z, y 0-9 incluidas el guion y el punto.");
#endif

} else {
kadukatu = dotime("1m");
kadukatu += time(NULL);
ni->time_vhost = kadukatu;
tm = localtime(&kadukatu);


strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
//sartu_datua(22, u->nick, vhost);
#ifdef IRC_UNDERNET_P09
do_write_bdd(u->nick, 4, cadena);
#endif
    notice_lang(s_IpVirtual, u, IPV_ACTIVAR_SET, u->nick, cadena);
//vhost_aldaketa(u->nick, vhost, 1);
canaladmins(s_IpVirtual, "2%s4 VHOST[12%s4]", u->nick,cadena);
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Acabas de personalizar tu vhost, el proximo cambio podras hacer dentro de 1 minuto");
privmsg(s_IpVirtual, u->numerico, "Hora exacta: %s", timebuf);
#else
privmsg(s_IpVirtual, u->nick, "Acabas de personalizar tu vhost, el proximo cambio podras hacer dentro de 1 minuto");
privmsg(s_IpVirtual, u->nick, "Hora exacta: %s", timebuf);
#endif
	return;
}


}
     	else
        privmsg(s_IpVirtual, u->nick, "fuera del rango");

}
}
static void do_ipv_del(User *u)
{
    char *text = strtok(NULL, " ");

    if (!text) {
	char buf[32];
	#ifdef IRC_UNDERNET_P10
	privmsg(s_IpVirtual, u->numerico, "Sintaxis: PONER ADD/DEL/LIST [param]");
	#else
	privmsg(s_IpVirtual, u->nick,"Sintaxis: PONER ADD/DEL/LIST [param]");
	#endif
    
    } else {
    
	if (stricmp(text, "ALL") != 0) {
	    int num = atoi(text);
	    if (num > 0 && del_ipv(num))
	      #ifdef IRC_UNDERNET_P10
		privmsg(s_IpVirtual, u->numerico, "Entrada eliminada.");
		#else
		privmsg(s_IpVirtual, u->nick,"Entrada eliminada.");
		#endif
	    else
	        #ifdef IRC_UNDERNET_P10
		privmsg(s_IpVirtual, u->numerico, "Numero incorrecto.");
		#else
		privmsg(s_IpVirtual, u->nick,"Numero incorrecto.");
		#endif
	} else {
	    if (del_ipv(0))
	         #ifdef IRC_UNDERNET_P10
		privmsg(s_IpVirtual, u->numerico, "Se han eliminado todas las entradas");
		#else
		privmsg(s_IpVirtual, u->nick, "Se han eliminado todas las entradas");
		#endif
	    else
	         #ifdef IRC_UNDERNET_P10	        
		privmsg(s_IpVirtual, u->numerico, "No se ha eliminado nada");
		#else
		privmsg(s_IpVirtual, u->nick,"No se ha eliminado nada");
		#endif
	}
	if (readonly)
	    notice_lang(s_OperServ, u, READ_ONLY_MODE);
    }
}
static void do_cambia_vhost(User *u)
{
    char *vhost = strtok(NULL, " ");
    static const char karak[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.";
    time_t oraingoa = time(NULL);
    time_t kadukatu;
    struct tm *tm;
    char timebuf[32];
    NickInfo *ni;

if (!(ni = findnick(u->nick))) {
	notice_lang(s_IpVirtual, u, NICK_X_NOT_REGISTERED, u->nick);
	return;
}
if (!vhost) {
 #ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Uso: CAMBIAR <nueva.ip.virtual>");
#else
privmsg(s_IpVirtual, u->nick, "Uso: CAMBIAR <nueva.ip.virtual>");
#endif
return;
}

if (oraingoa < ni->time_vhost)
{
tm = localtime(&ni->time_vhost);

strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Solamente se admite un cambio cada 1m. Podras cambiar el vhost a la hora: %s", timebuf);
#else
privmsg(s_IpVirtual, u->nick, "Solamente se admite un cambio cada 1m. Podras cambiar el vhost a la hora: %s", timebuf);
#endif
return;
}
if (!stricmp(vhost, "OFF")) {
#ifdef IRC_UNDERNET_P09
do_write_bdd(u->nick, 4, "");
#endif
notice_lang(s_IpVirtual, u, IPV_ACTIVAR_UNSET, u->nick);;
//vhost_aldaketa(u->nick, 0, 0);
canaladmins(s_IpVirtual, "2 %s 5 Desactiva Su VHOST ", u->nick);
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Vhost eliminada.");
#else
privmsg(s_IpVirtual, u->nick, "Vhost eliminada.");
#endif
		return;
}
if (strlen(vhost) > 125) {
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Como maximo puede tener 124 caracteres.");
#else
privmsg(s_IpVirtual, u->nick, "Como maximo puede tener 124 caracteres.");
#endif
return;
}
 if ((strstr(vhost, "root")) || (strstr(vhost, "admin")) || (strstr(vhost, "coadmin")) || (strstr(vhost, "devel")) || (strstr(vhost, "oper")) || (strstr(vhost, "patrocina")) || (strstr(vhost, "euskalirc")) || (strstr(vhost, "net")) || (strstr(vhost, "com")) || (strstr(vhost, "org")) || (strstr(vhost, "tk")) || (strstr(vhost, "cat")) || (strstr(vhost, "eu")) || (strstr(vhost, "hisp")) || (strstr(vhost, "http")) || (strstr(vhost, "www")) || (strstr(vhost, "ircop")) || (strstr(vhost, "help"))) {
 privmsg(s_IpVirtual, u->nick, "contiene palabras no permitidas");
 return;
 }
// egiaztatu karaktereak
if (vhost[strspn(vhost, karak)]) {
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "El vhost solamente admite caracteres entre a-z, y 0-9 incluidas el guion y el punto.");
#else
privmsg(s_IpVirtual, u->nick, "El vhost solamente admite caracteres entre a-z, y 0-9 incluidas el guion y el punto.");
#endif

} else {
kadukatu = dotime("1m");
kadukatu += time(NULL);
ni->time_vhost = kadukatu;
tm = localtime(&kadukatu);


strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_DATE_TIME_FORMAT, tm);
//sartu_datua(22, u->nick, vhost);
#ifdef IRC_UNDERNET_P09
do_write_bdd(u->nick, 4, vhost);
#endif
    notice_lang(s_IpVirtual, u, IPV_ACTIVAR_SET, u->nick, vhost);
//vhost_aldaketa(u->nick, vhost, 1);
canaladmins(s_IpVirtual, "2%s4 VHOST[12%s4]", u->nick,vhost);
#ifdef IRC_UNDERNET_P10
privmsg(s_IpVirtual, u->numerico, "Acabas de personalizar tu vhost, el proximo cambio podras hacer dentro de 1 minuto");
privmsg(s_IpVirtual, u->numerico, "Hora exacta: %s", timebuf);
#else
privmsg(s_IpVirtual, u->nick, "Acabas de personalizar tu vhost, el proximo cambio podras hacer dentro de 1 minuto");
privmsg(s_IpVirtual, u->nick, "Hora exacta: %s", timebuf);
#endif
	return;
}



}

static void do_vhost(User *u)
{
    char *nick = strtok(NULL, " ");
    char *mask = strtok(NULL, "");
  
    /*    User *u2 = NULL; */
    NickInfo *ni;
    
    if (!nick) {
    	syntax_error(s_IpVirtual, u, "ACTIVAR", IPV_ACTIVAR_SYNTAX);
    	return;
    } else if (!(ni = findnick(nick))) {
	notice_lang(s_IpVirtual, u, NICK_X_NOT_REGISTERED, nick);
	return;
    } else  if (!(ni->status & NI_ON_BDD)) {
        notice_lang(s_IpVirtual, u, NICK_MUST_BE_ON_BDD);
	return;
    }
    
    if (!mask) {
	#ifdef IRC_UNDERNET_P09
    	do_write_bdd(nick, 2, "");
	#endif
	notice_lang(s_IpVirtual, u, IPV_ACTIVAR_UNSET, nick);
	return;
    }
    if (strlen(mask) > 64) {
	 privmsg(s_IpVirtual, u->nick, "V-Host demasiado larga. Máximo 64 carácteres.");
	 return;
    }
    
    #ifdef IRC_UNDERNET_P09
    do_write_bdd(nick, 2, mask);
#endif
    notice_lang(s_IpVirtual, u, IPV_ACTIVAR_SET, nick, mask);
}
/*************************************************************************/
/*********************** OperServ command functions **********************/
/*************************************************************************/
static void do_credits(User *u)
{
    notice_lang(s_IpVirtual, u, SERVICES_CREDITS);
}
    
/***********************************************************************/
    /***********************************************************************/
    

/* HELP command. */

static void do_help(User *u)
{
    const char *cmd = strtok(NULL, "");

    if (!cmd) {
	notice_help(s_IpVirtual, u, IPV_HELP);
    } else {
	help_cmd(s_IpVirtual, u, cmds, cmd);
    }
}
/*************************************************************************/

static void do_set(User *u)
{
    char *cmd    = strtok(NULL, " ");
    char *param  = strtok(NULL, " ");
    NickInfo *ni;
    int is_servadmin = is_services_admin(u);
    int set_nick = 0;

    if (readonly) {
	notice_lang(s_IpVirtual, u, NICK_SET_DISABLED);
	return;
    }

    if (is_servadmin && cmd && (ni = findnick(cmd))) {
        if (nick_is_services_admin(ni) && !is_services_root(u)) {
            notice_lang(s_IpVirtual, u, PERMISSION_DENIED);
            return;
        }
                                        
	cmd = param;
	param = strtok(NULL, " ");
	set_nick = 1;
    } else {
	ni = u->ni;
    }
    if (!param && (!cmd)) {
	if (is_servadmin) {
	    syntax_error(s_IpVirtual, u, "SET", IPV_SET_SERVADMIN_SYNTAX);
	} else {
	    syntax_error(s_IpVirtual, u, "SET", IPV_SET_SYNTAX);
	}
	
    } else if (!ni) {
	notice_lang(s_IpVirtual, u, NICK_NOT_REGISTERED);
    } else if (!is_servadmin && !nick_identified(u)) {
	notice_lang(s_IpVirtual, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
    } else if (stricmp(cmd, "VHOST") == 0) {
        do_set_vhost(u, ni, param);
	
    } else {
	if (is_servadmin)
	    notice_lang(s_IpVirtual, u, NICK_SET_UNKNOWN_OPTION_OR_BAD_NICK,
			strupper(cmd));
	else
	    notice_lang(s_IpVirtual, u, NICK_SET_UNKNOWN_OPTION, strupper(cmd));
    }
}
/*************************************************************************/

static void do_set_vhost(User *u, NickInfo *ni, char *param)
{
   if (!is_services_oper(u)) {
	notice_lang(s_IpVirtual, u, PERMISSION_DENIED); 
	return;
    }
    
    if (!(ni->status & NI_ON_BDD)) {
         notice_lang(s_IpVirtual, u, NICK_MUST_BE_ON_BDD);
	 return;
    }
    if (!param) {
		    privmsg(s_IpVirtual, u->nick, "V-Host sin Rellenar  Minimo 3 carácteres");
		    return;
	}
    if (strlen(param) > 56) {
		    privmsg(s_IpVirtual, u->nick, "V-Host demasiado larga. Máximo 56 carácteres");
		    return;
	}
     if (strlen(param) <= 2) {
		    privmsg(s_IpVirtual, u->nick, "V-Host demasiado corta. Minimo 3 carácteres");
		    return;
	}
	
		    
    if (stricmp(param, "OFF") == 0) {
         canaladmins(s_IpVirtual, "2 %s 5 Desactiva con 4SET VHOST  a  12%s ", u->nick,ni->nick);
	#ifdef IRC_UNDERNET_P09
	 do_write_bdd(ni->nick, 4, "");
	#endif
	 notice_lang(s_IpVirtual, u, IPV_SET_VHOST_OFF);
    } else {
        strcat(param , ".usuario.de.euskalirc");
	canaladmins(s_IpVirtual, "2%s4 VHOST[12%s4]", ni->nick,param);
       #ifdef IRC_UNDERNET_P09
        do_write_bdd(ni->nick, 4, param);
        #endif
	notice_lang(s_IpVirtual, u, IPV_SET_VHOST_ON, param);
    }
}
	
/*************************************************************************/

/**************************************************************************/
/* Kill basico */
/*
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


/* Gline de 5 minutos */

/*static void do_block(User *u)
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
                                          
}*/





