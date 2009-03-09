/* Servicio Anti-Spam, botshispanobdd
 *
 * (C) 2002 David Martín Díaz <equis@fuckmicrosoft.com>
 *
 * Este programa es software libre. Puede redistribuirlo y/o modificarlo
 * bajo los términos de la Licencia Pública General de GNU según es publicada
 * por la Free Software Foundation.
 */
 
 #include "services.h"
 #include "pseudo.h"
 
 // SpamServ estruktura

typedef struct spam_ SpamInfo;
struct spam_ {
    int32 zenb;
    char *hitza;
    char zeinek[NICKMAX];
};

static int32 sspam = 0;
static int32 spam_size = 0;
static SpamInfo *spam = NULL;

// SpamServ Funtzioak.
static void do_spam_list(User *u);
static void do_spam_add(User *u);
static int add_spam(User *u, const char *text);
static void do_spam_del(User *u); // externu
static int del_spam(int num);
/*************************************************************************/
/*********************** Spam item loading/saving ************************/
/*************************************************************************/

#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Read error on %s", SpamDBName);	\
	sspam = i;					\
	break;						\
    }							\
} while (0)

void load_spam()
{
    dbFILE *f;
    int i;
    int16 n;
    int32 tmp32;

    if (!(f = open_db(s_OperServ, SpamDBName, "r")))
	return;
    switch (i = get_file_version(f)) {
      case 8:
      case 7:
	SAFE(read_int16(&n, f));
	sspam = n;
	if (sspam < 8)
	    spam_size = 16;
	else if (sspam >= 16384)
	    spam_size = 32767;
	else
	    spam_size = 2*sspam;
	spam = smalloc(sizeof(*spam) * spam_size);
	if (!sspam) {
	    close_db(f);
	    return;
	}
	for (i = 0; i < sspam; i++) {
	    SAFE(read_int32(&spam[i].zenb, f));
	    SAFE(read_string(&spam[i].hitza, f));
	    SAFE(read_buffer(spam[i].zeinek, f));
	}
	break;

      default:
	fatal("Unsupported version (%d) on %s", i, SpamDBName);
    } /* switch (ver) */

    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {						\
    if ((x) < 0) {						\
	restore_db(f);						\
	log_perror("Write error on %s", SpamDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {		\
	    canalopers(NULL, "Write error on %s: %s", SpamDBName,	\
			strerror(errno));			\
	    lastwarn = time(NULL);				\
	}							\
	return;							\
    }								\
} while (0)

void save_spam()
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_OperServ, SpamDBName, "w")))
	return;
    SAFE(write_int16(sspam, f));
    for (i = 0; i < sspam; i++) {
	SAFE(write_int32(spam[i].zenb, f));
	SAFE(write_string(spam[i].hitza, f));
	SAFE(write_buffer(spam[i].zeinek, f));
    }
    close_db(f);
}

#undef SAFE



void do_spam(User *u)
{
    int is_servadmin = is_services_admin(u);
    char *cmd = strtok(NULL, " ");
    char *typename;
    int *msgs;



    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "LIST") == 0) {
	do_spam_list(u);

    } else if (stricmp(cmd, "ADD") == 0) {
	if (is_servadmin)
	    do_spam_add(u);
	else
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);

    } else if (stricmp(cmd, "DEL") == 0) {
	if (is_servadmin)
	    do_spam_del(u);
	else
	    notice_lang(s_OperServ, u, PERMISSION_DENIED);

    } else {
        #ifdef IRC_UNDERNET_P10
	privmsg(s_OperServ, u->numerico, "Sintaxis: SPAM ADD/DEL/LIST [param]");
	#else
	privmsg(s_ChanServ, u->nick,"Sintaxis: SPAM ADD/DEL/LIST [param]");
	#endif
    }
}

static int del_spam(int num)
{
    int i;
    int count = 0;

    for (i = 0; i < sspam; i++) {
	if (num == 0 || spam[i].zenb == num) {
	    free(spam[i].hitza);
	    count++;
	    sspam--;
	    if (i < sspam)
		memcpy(spam+i, spam+i+1, sizeof(*spam) * (sspam-i));
	    i--;
	}
    }
    return count;
}


static void do_spam_del(User *u)
{
    char *text = strtok(NULL, " ");

    if (!text) {
	char buf[32];
	#ifdef IRC_UNDERNET_P10
	         privmsg(s_OperServ, u->numerico, "Sintaxis: SPAM ADD/DEL/LIST [param]");
                
#else
                 privmsg(s_ChanServ, u->nick, "Sintaxis: SPAM ADD/DEL/LIST [param]");
#endif
	
    } else {
    
	if (stricmp(text, "ALL") != 0) {
	    int num = atoi(text);
	    if (num > 0 && del_spam(num))
	      #ifdef IRC_UNDERNET_P10
		privmsg(s_OperServ, u->numerico, "Entrada eliminada.");
		#else
		privmsg(s_OperServ, u->nick,"Entrada eliminada.");
		#endif
	    else
	        #ifdef IRC_UNDERNET_P10
		privmsg(s_OperServ, u->numerico, "Numero incorrecto.");
		#else
		privmsg(s_OperServ, u->nick,"Numero incorrecto.");
		#endif
	} else {
	    if (del_spam(0))
	         #ifdef IRC_UNDERNET_P10
		privmsg(s_OperServ, u->numerico, "Se han eliminado todas las entradas");
		#else
		privmsg(s_OperServ, u->nick, "Se han eliminado todas las entradas");
		#endif
	    else
	         #ifdef IRC_UNDERNET_P10	        
		privmsg(s_OperServ, u->numerico, "No se ha eliminado nada");
		#else
		privmsg(s_OperServ, u->nick,"No se ha eliminado nada");
		#endif
	}
	if (readonly)
	    notice_lang(s_OperServ, u, READ_ONLY_MODE);
    }
}

static int add_spam(User *u, const char *text)
{
    int i, num;

    if (sspam >= 32767)
	return -1;

    if (sspam >= spam_size) {
	if (spam_size < 8)
	    spam_size = 8;
	else
	    spam_size *= 2;
	spam = srealloc(spam, sizeof(*spam) * spam_size);
    }
    num = 0;
    for (i = sspam-1; i >= 0; i--) {
	
	    num = spam[i].zenb;
	    break;
	
    }
    

    spam[sspam].zenb = num+1;
    spam[sspam].hitza = sstrdup(text);
    strscpy(spam[sspam].zeinek, u->nick, NICKMAX);
    sspam++;
    return num+1;
}

static void do_spam_add(User *u)
{
    char *text = strtok(NULL, "");

    if (!text) {
	char buf[32];
	#ifdef IRC_UNDERNET_P10
	privmsg(s_OperServ, u->numerico, "Sintaxis: SPAM ADD/DEL/LIST [param]");
	#else
	privmsg(s_OperServ, u->nick,"Sintaxis: SPAM ADD/DEL/LIST [param]");
	#endif
    } else {
	int n = add_spam(u, text);
	if (n < 0)
	    #ifdef IRC_UNDERNET_P10
	    privmsg(s_OperServ, u->numerico, "La lista esta llena.");
	   #else
	    privmsg(s_OperServ, u->nick,"La lista esta llena.");
	    #endif
	else
	     #ifdef IRC_UNDERNET_P10
	    privmsg(s_OperServ, u->numerico, "Se ha añadido a la lista de spams.");
	    #else
	    privmsg(s_OperServ, u->nick, "Se ha añadido a la lista de spams.");
	     #endif
	    
	if (readonly)
	    notice_lang(s_OperServ, u, READ_ONLY_MODE);
    }
}

static void do_spam_list(User *u)
{
    int i, count = 0;
    char timebuf[64];
    struct tm *tm;

    for (i = 0; i < sspam; i++) {
	    if (count == 0)
	     #ifdef IRC_UNDERNET_P10
	privmsg(s_OperServ, u->numerico, "ID :: Texto :: Agregado por");

 privmsg(s_OperServ, u->numerico, "%d - '%s' - '%s'", spam[i].zenb, spam[i].hitza, *spam[i].zeinek ? spam[i].zeinek : "<unknown>");
         #else
	 privmsg(s_OperServ, u->nick,"ID :: Texto :: Agregado por");
	 privmsg(s_OperServ, u->nick,"%d - '%s' - '%s'", spam[i].zenb, spam[i].hitza, *spam[i].zeinek ? spam[i].zeinek : "<unknown>");
	 #endif
	    count++;
    }
    if (count == 0)
        #ifdef IRC_UNDERNET_P10
	privmsg(s_OperServ, u->numerico, "La lista esta vacia.");
	#else
	 privmsg(s_OperServ, u->nick,"La lista esta vacia.");
	 #endif
	 
	
}

// Kanaletik sartu edo irten
void spam_ikusi(Channel *ci)
{
char adm_kanala[BUFSIZE];
char opr_kanala[BUFSIZE];
snprintf(adm_kanala, sizeof(adm_kanala), "#%s", CanalAdmins);
snprintf(opr_kanala, sizeof(opr_kanala), "#%s", CanalOpers);


if (SpamUsers == 0)
return;

if (!strcmp(ci->name, adm_kanala) || !strcmp(ci->name, opr_kanala)) return;

//pertsonak = atoi(ci->erab);
if (ci->erab == 0)
 canaladmins(s_SpamServ, "Saliendo de %s, usuarios: %d", ci->name, ci->erab);
if (ci->erab == 0) 
#ifdef IRC_UNDERNET_P10
send_cmd(s_SpamServ, "L %s", ci->name);

 #else
send_cmd(s_SpamServ, "PART %s", ci->name);
#endif

else if (ci->erab >= SpamUsers) {
 #ifdef IRC_UNDERNET_P10
 send_cmd(s_SpamServ, "J %s", ci->name);
 #else
send_cmd(s_SpamServ, "JOIN %s", ci->name);
#endif

if (ci->erab == SpamUsers)
 canaladmins(s_OperServ, "5<%s> 12,15 %s  3 Usuarios: 2 %d", s_SpamServ,ci->name, ci->erab);
}
else 
{ 
if (ci->erab < SpamUsers)

#ifdef IRC_UNDERNET_P10
send_cmd(s_SpamServ, "L %s", ci->name);
#else
send_cmd(s_SpamServ, "PART %s", ci->name);
#endif
if   (ci->erab  == SpamUsers-1)
 canaladmins(s_OperServ, "5<%s> 4,15%s   3 Usuarios: 2 %d", s_SpamServ,ci->name, ci->erab);
} 



}


// Spam detektorea
void eskaneatu_kanala(char *zein,char *kanala, char *testua)
{
int i, count = 0;
User *erab;
erab = finduser(zein);


char adm_kanala[BUFSIZE];
char opr_kanala[BUFSIZE];
snprintf(adm_kanala, sizeof(adm_kanala), "#%s", CanalAdmins);
snprintf(opr_kanala, sizeof(opr_kanala), "#%s", CanalOpers);
 
   
if (SpamUsers == 0)
return;
 
if (!strcmp(kanala, adm_kanala) || !strcmp(kanala, opr_kanala)) return;


 for (i = 0; i < sspam; i++) {

        if (strstr(testua, spam[i].hitza)) {
           canaladmins(s_SpamServ, "[SPAM] User: 2%s@12%s , Canal: %s '5%s'", zein, erab->host, kanala, testua);
            break;
            }
            count ++;
        }
}



 

 void antispamc(const char *source,const char *chan, char *buf)
 {
 int i;
 char adm_canal[BUFSIZE];
char op_canal[BUFSIZE];
snprintf(adm_canal, sizeof(adm_canal), "#%s", CanalAdmins);
snprintf(op_canal, sizeof(op_canal), "#%s", CanalOpers);
  User *u = finduser(source);
   
if (SpamUsers == 0)
return;
 
if (!strcmp(chan, adm_canal) || !strcmp(chan, op_canal)) return;


for (i = 0; i < sspam; i++) {
 	if (strstr(buf, spam[i].hitza)) {
	       
		if   (is_services_root(u) || is_services_admin(u) || is_services_cregadmin(u) || is_services_devel(u) || is_services_oper(u))  {
 		 privmsg(s_SpamServ, chan, "El Representante de Red 12%s  dijo 4%s !! ", source,spam[i].hitza);
		return;
		}
		 privmsg(s_SpamServ, chan, "Ey! %s dijo 4%s !!", source,spam[i].hitza);
		send_cmd(s_SpamServ, "MODE %s +b :%s", chan , source);
	        send_cmd(s_SpamServ, "KICK %s %s : _antispam 4(Publicidad No Autorizada.Forma Parte Del Listado Censurado Por La Red)", chan, source);
		send_cmd(ServerName, "SVSJOIN  %s #%s", source,CanalSpamers);
		return;
		}
	
	}
 }
