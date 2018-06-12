/* CregServ functions.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * DarkuBots es una adaptaci�n de Javier Fern�ndez Vi�a, ZipBreake.
 * E-Mail: javier@jfv.es || Web: http://jfv.es/
 *
 * Modificaciones -donostiarra- admin.euskalirc@gmail.com   
 *					
 *						*/


#include "services.h"
#include "pseudo.h"
#if defined(SOPORTE_JOOMLA15)
#include <mysql.h>
#endif
#define canales 6000
static CregInfo *creglists[canales]; 

static MemoInfo *getmemoinfo(const char *name, int *ischan);
static void do_help(User *u);
static void do_sec(User *u);
static void do_sec_geo(User *u);
static void do_registra(User *u);
static void do_reg(User *u);
static void do_acepta(User *u);
static void do_drop(User *u);
static void do_marcar(User *u);
static void do_suspend(User *u);
static void do_unsuspend(User *u);
static void do_desmarcar(User *u);
static void do_deniega(User *u);
static void do_cancela(User *u);
static void do_apoya(User *u);
static void do_estado(User *u);
static void do_apoyos(User *u);
static void do_fuerza(User *u);
static void do_list(User *u);
static void do_info(User *u);


static Command cmds[] = {
    { "HELP",       do_help,     NULL,  -1,                 -1,-1,-1,-1 },
    { "AYUDA",      do_help,     NULL,  -1,                 -1,-1,-1,-1 },
 /* 
listado de secciones admitidos para clasificar un canal*/
     { "SECCION",      do_sec,     NULL,  -1,                 -1,-1,-1,-1 },
    { "GEO",      do_sec_geo,     NULL,  -1,                 -1,-1,-1,-1 },
    { "REGISTRA",   do_registra, NULL,  CREG_HELP_REGISTRA, -1,-1,-1,-1 },
    { "REGISTER",   do_registra, NULL,  CREG_HELP_REGISTRA, -1,-1,-1,-1 },
    { "CANCELA",    do_cancela,  NULL,  CREG_HELP_CANCELA,  -1,-1,-1,-1 },
    { "APOYA",      do_apoya,    NULL,  CREG_HELP_APOYA,    -1,-1,-1,-1 },
    { "ESTADO",     do_estado,   NULL,  CREG_HELP_ESTADO,   -1,-1,-1,-1 },
    { "APOYOS",     do_apoyos,   NULL,  CREG_HELP_APOYOS,   -1,-1,-1,-1 },        

    { "INFO",     do_info,    is_services_cregadmin,   -1,-1,-1,-1,-1 },        
    { "LISTA",    do_list,    is_services_cregadmin,   -1,-1,-1,-1,-1 },
    { "LIST",     do_list,    is_services_cregadmin,   -1,-1,-1,-1,-1 },
    { "REG",     do_reg,    is_services_cregadmin,   -1,-1,-1,-1,-1 },  

 /*  donostiarra-2009-
COMercial OFIcial REPresentantes de Red -para registro de canales especiales
    */
    { "ACEPTA",   do_acepta,  is_services_cregadmin, -1,-1,-1,-1,-1 },
    { "DENIEGA",  do_deniega, is_services_cregadmin, -1,-1,-1,-1,-1 },
    { "DROP",     do_drop,    is_services_cregadmin, -1,-1,-1,-1,-1 },
 /*para vigilar canales */
    { "MARCAR",     do_marcar,    is_services_cregadmin, -1,-1,-1,-1,-1 },
     { "DESMARCAR",     do_desmarcar,    is_services_cregadmin, -1,-1,-1,-1,-1 },
/*
     donostiarra-2009-

las suspensiones y unsuspensiones tambien las recoge chan*/
  
    { "SUSPEND",     do_suspend,    is_services_cregadmin, -1,-1,-1,-1,-1 },
    { "UNSUSPEND",     do_unsuspend,    is_services_cregadmin, -1,-1,-1,-1,-1 },
    { "FUERZA",   do_fuerza,  is_services_cregadmin,      -1,-1,-1,-1,-1 },
    { NULL }
};

static void alpha_insert_creg(CregInfo *cr);
static int delcreg(CregInfo *cr);

/***********************************************************************/
/*                  RUTINAS INTERNAS DEL BOT CREG                      */ 
/***********************************************************************/

/* Devuelve la informaci�n de la memoria utilizada. */

void get_cregserv_stats(long *nrec, long *memuse)
{
    long count = 0, mem = 0;
    int i;
    CregInfo *cr;
                        
    for (i = 0; i < canales; i++) {
        for (cr = creglists[i]; cr; cr = cr->next) {
            count++;
            mem += sizeof(*cr);
            if (cr->nickoper)
                mem += strlen(cr->nickoper)+1;
            if (cr->motivo)
                mem += strlen(cr->motivo)+1;
            mem += cr->apoyoscount * sizeof(ApoyosCreg);
        }
    }
    *nrec = count;
    *memuse = mem;
}            
/*************************************************************************/
/* Inicializaci�n del bot creg. */

void cr_init(void)
{
    Command *cmd;
    
    cmd = lookup_cmd(cmds, "APOYA");
    if (cmd)
         cmd->help_param1 = s_CregServ;
}                                                                                                                                            

/****************CUANDO EXPIRAN LOS CANALES*************************/
void expire_creg()
{
  CregInfo *cr, *next;
  int i;
  time_t now = time(NULL);

    for (i = 0; i < canales; i++) {
	for (cr = creglists[i]; cr; cr = next) {
	  next = cr->next;
	  if (now - cr->time_peticion >= 604800 && (cr->estado & CR_PROCESO_REG)) {
	   canalopers(s_CregServ, "Expirado el proceso de registro de %s", cr->name);
	#if defined(SOPORTE_JOOMLA15)
MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_chans where canal ='%s';",cr->name);
if (mysql_query(conn,modifica)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
	   delcreg(cr);
	
	  }
          if (now - cr->time_motivo >= 172800 && !(cr->estado & (CR_PROCESO_REG | CR_REGISTRADO | CR_ACEPTADO))) {
	   canalopers(s_CregServ, "Expirado el canal no registrado %s", cr->name);
	#if defined(SOPORTE_JOOMLA15)
MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_chans where canal ='%s';",cr->name);
if (mysql_query(conn,modifica)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
	   delcreg(cr);
	  }
	}
     }
}

/*************************************************************************/

/* Load/save data files. */


#define SAFE(x) do {                                    \
    if ((x) < 0) {                                      \
        if (!forceload)                                 \
            fatal("Error de Lectura en %s", CregDBName);      \
        failed = 1;                                     \
        break;                                          \
    }                                                   \
} while (0)
 
                                            
void load_cr_dbase(void)
{
    dbFILE *f;
    int ver, i, j, c;
    CregInfo *cr, **last, *prev;
    int failed = 0;
                
    if (!(f = open_db(s_CregServ, CregDBName, "r")))
        return;
                            
   switch (ver = get_file_version(f)) {
      case 9:
      case 8:
        for (i = 0; i < canales && !failed; i++) {
            int32 tmp32;
            last = &creglists[i];
            prev = NULL;
            while ((c = getc_db(f)) != 0) {
                if (c != 1)
                    fatal("Invalid format in %s", CregDBName); 
                cr = smalloc(sizeof(CregInfo));
                *last = cr;
                last = &cr->next;
                cr->prev = prev;
                prev = cr;
                SAFE(read_buffer(cr->name, f));
                SAFE(read_string(&cr->founder, f));

                SAFE(read_buffer(cr->founderpass, f));
                SAFE(read_string(&cr->desc, f));
                SAFE(read_string(&cr->email, f));
                                                                                                                                                                                    
                SAFE(read_int32(&tmp32, f));
                cr->time_peticion = tmp32;                

                SAFE(read_string(&cr->nickoper, f));
                SAFE(read_string(&cr->motivo, f));                    
                SAFE(read_int32(&tmp32, f));
                cr->time_motivo = tmp32;                                                                                                

                SAFE(read_int32(&cr->estado, f));   
	        SAFE(read_int32(&cr->seccion, f));       /*las secciones principales que se cargan*/    
		 SAFE(read_int32(&cr->geo, f));       /*las secciones geogr�ficas que se cargan*/ 
		  SAFE(read_int32(&cr->tipo, f));       /*los tipos especiales COM,OFI,REP de secciones que se cargan*/                               
                SAFE(read_int16(&cr->apoyoscount, f));
                if (cr->apoyoscount) {
                    cr->apoyos = scalloc(cr->apoyoscount, sizeof(ApoyosCreg));
                    for (j = 0; j < cr->apoyoscount; j++) {                        
                        SAFE(read_string(&cr->apoyos[j].nickapoyo, f));
                        SAFE(read_string(&cr->apoyos[j].emailapoyo, f));                               
                        SAFE(read_int32(&tmp32, f));
                        cr->apoyos[j].time_apoyo = tmp32;                                                              
                    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
                } else {
                    cr->apoyos = NULL;
                }                
            }  /* while (getc_db(f) != 0) */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
             *last = NULL;

        }  /* for (i) */ 
        break;                                                                                                   
     default:
        fatal("Unsupported version number (%d) on %s", ver, CregDBName);

    }  /* switch (version) */
    
    close_db(f);
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {                                            \
    if ((x) < 0) {                                              \
        restore_db(f);                                          \
        log_perror("Write error on %s", CregDBName);            \
        if (time(NULL) - lastwarn > WarningTimeout) {           \
            canalopers(NULL, "Write error on %s: %s", CregDBName,  \
                           strerror(errno));                       \
            lastwarn = time(NULL);                              \
        }                                                       \
        return;                                                 \
    }                                                           \
} while (0)
                                                                                                                                            
void save_cr_dbase(void)
{
    dbFILE *f;
    int i, j;
    CregInfo *cr;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_CregServ, CregDBName, "w")))
        return;
           
    for (i = 0; i < canales; i++) {
                                            
        for (cr = creglists[i]; cr; cr = cr->next) {
            SAFE(write_int8(1, f));
            SAFE(write_buffer(cr->name, f));
            SAFE(write_string(cr->founder, f));
                                
            SAFE(write_buffer(cr->founderpass, f));
            SAFE(write_string(cr->desc, f));
            SAFE(write_string(cr->email, f));
                                                                                                    
            SAFE(write_int32(cr->time_peticion, f));
            SAFE(write_string(cr->nickoper, f));
            SAFE(write_string(cr->motivo, f));
            SAFE(write_int32(cr->time_motivo, f));

            SAFE(write_int32(cr->estado, f));   
	    SAFE(write_int32(cr->seccion, f));  /*las secciones principales que se graban*/    
	     SAFE(write_int32(cr->geo, f));  /*las secciones geogr�ficas que se graban*/ 
	    SAFE(write_int32(cr->tipo, f));  /*los tipos especiales COM,OFI,REP de secciones que se graban*/                                                                                          
            SAFE(write_int16(cr->apoyoscount, f));
            for (j = 0; j < cr->apoyoscount; j++) {            
                SAFE(write_string(cr->apoyos[j].nickapoyo, f));
                SAFE(write_string(cr->apoyos[j].emailapoyo, f));
                SAFE(write_int32(cr->apoyos[j].time_apoyo, f));
            }                     
        } /* for (creglists[i]) */
        
        SAFE(write_int8(0, f));
                
    } /* for (i) */
                    
    close_db(f);
}

#undef SAFE

/*************************************************************************/


/*************************************************************************/


void cregserv(const char *source, char *buf)
{
    char *cmd, *s;
    User *u = finduser(source);
        
    if (!u) {
        logeo("%s: user record for %s not found", s_CregServ, source);
        notice(s_CregServ, source,
            getstring((NickInfo *)NULL, USER_RECORD_NOT_FOUND));
        return;
    }
    
    /*if (!is_moder(u->nick))
        return;*/

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
            notice_lang(s_CregServ, u, NICK_NOT_REGISTERED_HELP, s_NickServ);
        else
            run_cmd(s_CregServ, u, cmds, cmd);
    }
}

/*************************************************************************/

/* Return the ChannelInfo structure for the given channel, or NULL if the
 * channel isn't registered. */

CregInfo *cr_findcreg(const char *chan)
{
     CregInfo *cr;
     
     for (cr = creglists[tolower(chan[1])]; cr; cr = cr->next) {
         if (stricmp(cr->name, chan) == 0)
             return cr;
     }
     return NULL;
}

/*************************************************************************/

/*************************************************************************/
/*********************** CregServ command routines ***********************/
/*************************************************************************/
/* Insert a channel alphabetically into the database. */

static void alpha_insert_creg(CregInfo *cr)
{
    CregInfo *ptr, *prev;
    char *chan = cr->name;
        
    for (prev = NULL, ptr = creglists[tolower(chan[1])];
                        ptr != NULL && stricmp(ptr->name, chan) < 0;
                        prev = ptr, ptr = ptr->next);
    cr->prev = prev;
    cr->next = ptr;
    if (!prev)
        creglists[tolower(chan[1])] = cr;
    else
        prev->next = cr;
    if (ptr)
        ptr->prev = cr;
}

/*************************************************************************/

/* Add a channel to the database.  Returns a pointer to the new ChannelInfo
 * structure if the channel was successfully registered, NULL otherwise.
  * Assumes channel does not already exist. */
  
static CregInfo *makecreg(const char *chan)
{
    CregInfo *cr;
    
    cr = scalloc(sizeof(CregInfo), 1);
    /*strscpy(cr->name, chan, CHANMAX);*/
    strscpy(cr->name, chan, CHANMAX);
    alpha_insert_creg(cr);
    return cr;
}

/*************************************************************************/

/* Remove a channel from the ChanServ database.  Return 1 on success, 0
 * otherwise. */
                                                                                                                                                                         
static int delcreg(CregInfo *cr)
{
  
    if (cr->next)
	cr->next->prev = cr->prev;
    if (cr->prev)
	cr->prev->next = cr->next;
    else
	creglists[tolower(cr->name[1])] = cr->next;
    /*if (cr->founder)
	free(cr->founder);*/
    if (cr->desc)
	free(cr->desc);
    if (cr->email)
	free(cr->email);
    /*if (cr->nickoper)
	free(cr->nickoper);*/
    if (cr->motivo)
	free(cr->motivo);
    if (cr->apoyos)
	free(cr->apoyos);
    cr = NULL; 
 
    free(cr);

    return 1;
    
}            
/*************************************************************************/

/* Return a help message. */
static void do_help(User *u)
{
    char *cmd = strtok(NULL, "");
    
    if (!cmd) {
        notice_help(s_CregServ, u, CREG_HELP,Net,CregApoyos,WebNetwork);
        
    if (is_services_cregadmin(u))
        notice_help(s_CregServ, u, CREG_SERVADMIN_HELP,CregApoyos);
    } else {
        help_cmd(s_CregServ, u, cmds, cmd);
    }
}

/*************************************************************************/

/*************************************************************************/

/*Te Ayuda diciendo las secciones de Creg. */
static void do_sec(User *u)
{
    char *cmd = strtok(NULL, "");
    char *param = strtok(NULL, "");
    if ((!cmd) && (!param)) {
        notice_help(s_CregServ, u, CREG_HELP_SEC,Net);
         
    if (is_services_cregadmin(u))
        notice_help(s_CregServ, u, CREG_SERVADMIN_HELP_SEC);
      } else {
        help_cmd(s_CregServ, u, cmds, cmd);
    }
}
static void do_sec_geo(User *u)
{
    char *cmd = strtok(NULL, "");
 
    if (!cmd) {
        notice_help(s_CregServ, u, CREG_HELP_GEO,Net);
          
       } else {
        help_cmd(s_CregServ, u, cmds, cmd);
    }
}


/*************************************************************************/

static void do_cancela(User *u)
{
    char *chan = strtok(NULL, " ");
    CregInfo *cr;
    NickInfo *ni = u->ni;
    
         
    if (!chan) {
       privmsg(s_CregServ, u->nick, "Sintaxis: 12CANCELA <canal>");
    } else if (!ni) {
         notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
         notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK, s_NickServ, s_NickServ);
    } else {
       if (!(cr = cr_findcreg(chan))) {
           privmsg(s_CregServ, u->nick, "El canal 12%s no existe", chan);  
       } else {
           if (!(cr->estado & CR_PROCESO_REG)) {
               privmsg(s_CregServ, u->nick, "El canal 12%s no puede ser eliminado. Solicitelo a un Administrador u Operador", chan);
               return;
           }
           if (stricmp(u->nick, cr->founder) == 0) {
		#if defined(SOPORTE_JOOMLA15)
MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_chans where canal ='%s';",chan);
if (mysql_query(conn,modifica)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
               delcreg(cr);                                      
               privmsg(s_CregServ, u->nick, "El canal 12%s ha sido cancelado", chan);
               canalopers(s_CregServ, "12%s ha cancelado la peticion de registro para 12%s", u->nick, chan);
           } else {
               privmsg(s_CregServ, u->nick, "4Acceso denegado!!");
	   }
       } 
    }
}

                                                        
/*************************************************************************/
static void do_apoya(User *u)
{
    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " "); 
    NickInfo *ni = u->ni;
    CregInfo *cr; 
    ApoyosCreg *apoyos; 
    int i; 
        
    char passtemp[255];            
    
    if (!chan) { 
       privmsg(s_CregServ, u->nick, "Sintaxis: 12APOYA <canal>");
    } else if (!ni) {
        notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
        notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK, s_NickServ, s_NickServ);
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� en proceso de registro", chan);
    } else if (!(cr->estado & CR_PROCESO_REG)) {                                           
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� pendiente de apoyos", chan); 
    } else {
        /* Controles de apoyos por nick/e-mail */
        if (stricmp(u->nick, cr->founder) == 0) {
            privmsg(s_CregServ, u->nick, "Ya eres el fundador del canal");
            return;
        }
        for (apoyos = cr->apoyos, i = 0; i < cr->apoyoscount; apoyos++, i++) {
            if (stricmp(u->nick, apoyos->nickapoyo) == 0) {
                privmsg(s_CregServ, u->nick, "Ya has apoyado este canal una vez.");
                return;
            }
        }
        for (apoyos = cr->apoyos, i = 0; i < cr->apoyoscount; apoyos++, i++) {
            if (stricmp(ni->email, apoyos->emailapoyo) == 0) {
                privmsg(s_CregServ, u->nick, "Ya ha apoyado alguien este canal con su cuenta de correo.");
                return;
            } if (stricmp(ni->email, cr->email) == 0) {
                privmsg(s_CregServ, u->nick, "Ya ha apoyado alguien este canal con su cuenta de correo.");
                return;
            }
        }
        
        if (!pass || !u->creg_apoyo) {
             srand(time(NULL));
             sprintf(passtemp, "%05u",1+(int)(rand()%99999) );
             strscpy(u->creg_apoyo, passtemp, PASSMAX);
	     privmsg(s_CregServ, u->nick, "No apoye el canal solamente porque le hayan dicho que");
	     privmsg(s_CregServ, u->nick, "escriba 12/msg %s APOYA %s. Inf�rmese sobre la tem�tica", s_CregServ, chan); 
	     privmsg(s_CregServ, u->nick, "del canal, sus usuarios, etc.");
	      privmsg(s_CregServ, u->nick, "Su Promotor es:4 %s",cr->founder);
	     privmsg(s_CregServ, u->nick, "La descripci�n del canal es:12 %s",cr->desc);
	     privmsg(s_CregServ, u->nick, "Si decide finalmente apoyar al canal 12%s h�galo escribiendo:", chan);
	     privmsg(s_CregServ, u->nick, "12/msg %s APOYA %s %s", s_CregServ, chan, u->creg_apoyo); 
             return;
        } else if (stricmp(pass, u->creg_apoyo) != 0) {
            privmsg(s_CregServ, u->nick, "4Clave inv�lida");
            privmsg(s_CregServ, u->nick, "Para confirmar tu apoyo al canal 12%s h�galo escribiendo:", chan);
            privmsg(s_CregServ, u->nick, "12/msg %s APOYA %s %s", s_CregServ, chan, u->creg_apoyo);
            return;
        }

        cr->apoyoscount++;
        cr->apoyos = srealloc(cr->apoyos, sizeof(ApoyosCreg) * cr->apoyoscount);
        apoyos = &cr->apoyos[cr->apoyoscount-1];
        apoyos->nickapoyo = sstrdup(u->nick);
        apoyos->emailapoyo = sstrdup(ni->email);
        apoyos->time_apoyo = time(NULL); 
        cr->time_lastapoyo = time(NULL);
        privmsg(s_CregServ, u->nick, "Anotado tu apoyo al canal 12%s", chan);             

        if ((cr->apoyoscount) >= CregApoyos) {
            cr->time_motivo = time(NULL);
            cr->estado = 0;
            cr->estado |= CR_ACEPTADO;
            canalopers(s_CregServ, "12%s ya tiene todos los apoyos", chan);
        }
    } 
}                                                                                                                          
                                                        
/*************************************************************************/

static void do_estado(User *u)
{
    char *chan = strtok(NULL, " ");
    CregInfo *cr;
    ChannelInfo *cr2;

    if (!chan) {
       privmsg(s_CregServ, u->nick, "Sintaxis: 12ESTADO <canal>");
       return;
    }                  
    if (!(cr = cr_findcreg(chan))) {
        if (!(cr2 = cs_findchan(chan)))
          privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado", chan);
        else
         if (cr2->flags & CI_VERBOTEN)
          privmsg(s_CregServ, u->nick, "El canal 12%s no puede ser registrado ni utilizado.", chan);
         else if (cr2->flags & CI_SUSPEND)
          privmsg(s_CregServ, u->nick, "El canal 12%s est� 12SUSPENDIDO temporalmente", chan);
         else
          privmsg(s_CregServ, u->nick, "El canal 12%s existe en 12%s pero no est� registrado con 12%s", chan, s_ChanServ, s_CregServ);
    
    } else if (cr->estado & CR_PROCESO_REG) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� pendiente de 12APOYOS", chan);
    } else if (cr->estado & CR_ACEPTADO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� pendiente de 12APROBACION por parte de la administraci�n de la red", chan);
    } else if (cr->estado & CR_DROPADO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� dropado de %s", chan, s_ChanServ);
    } else if (cr->estado & CR_RECHAZADO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s ha sido 12DENEGADO", chan);
        if (cr->motivo)        
             privmsg(s_CregServ, u->nick, "Motivo: 12%s", cr->motivo);

      /*para que lo vean todos los helpers ,que ha sido marcado el canal y su motivo*/

    } else if ((cr->estado & CR_MARCADO) &&  is_services_oper(u)){
        privmsg(s_CregServ, u->nick, "El canal 12%s ha sido 12MARCADO", chan);
        if (cr->motivo)        
             privmsg(s_CregServ, u->nick, "Motivo: 12%s", cr->motivo);
        /*un usuario normal ve el canal marcado,como registrado*/
    } else if  (cr->estado & CR_MARCADO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� 12REGISTRADO", chan);
        
    } else if (cr->estado & CR_SUSPENDIDO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s ha sido 12SUSPENDIDO", chan);
        if (cr->motivo)        
             privmsg(s_CregServ, u->nick, "Motivo: 12%s", cr->motivo);
    } else if (cr->estado & CR_REGISTRADO) {
        if ((cr2 = cs_findchan(chan))) {
             if (cr2->flags & CI_SUSPEND)
          	privmsg(s_CregServ, u->nick, "El canal 12%s est� 12SUSPENDIDO temporalmente", chan);
             else        
          	privmsg(s_CregServ, u->nick, "El canal 12%s est� 12REGISTRADO", chan);
        } else {
             privmsg(s_CregServ, u->nick, "El canal 12%s est� actualmente en estado 12DESCONOCIDO", chan);
	}
    } else if (cr->estado & CR_EXPIRADO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s es un canal 12EXPIRADO en %s", chan, s_ChanServ);
      } else {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� actualmente en estado 12DESCONOCIDO", chan);
    }
    if ((cr = cr_findcreg(chan))) {
        if (cr->seccion & CR_SOC) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de SOCIEDAD", chan);
	} 
	else if (cr->seccion & CR_INF ) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de INFORMATICA", chan);
	}
	else if (cr->seccion & CR_CIE ) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de CIENCIAS", chan);
	}
	else if (cr->seccion & CR_AYU) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de AYUDA", chan);
	}
	else if (cr->seccion & CR_ADU) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de ADULTOS", chan);
	}
	else if (cr->seccion & CR_PRO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de PROYECTOS e INVESTIGACIONES", chan);
	}
	else if (cr->seccion & CR_CUH) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de CULTURA y HUMANIDADES", chan);
	}
	else if (cr->seccion & CR_DEP) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de DEPORTES", chan);
	}
	else if (cr->seccion & CR_LCT) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de LITERATURA-CINE-TV-COMUNICACIONES", chan);
	}
	else if (cr->seccion & CR_MUS) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de MUSICA", chan);
	}
	else if (cr->seccion & CR_OCI) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de OCIO", chan);
	}
	else if (cr->seccion & CR_PAI) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de PAISES y CONTINENTES", chan);
	}
	else if (cr->seccion & CR_PRF) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de PROFESIONALES-OFICIOS", chan);
	}
	else if (cr->seccion & CR_SEX) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de SEXO", chan);
	}
	else if (cr->seccion & CR_RAD) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de Radios-Djs", chan);
	}
	else if (cr->seccion & CR_AMO) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de AMOR-AMISTAD", chan);
	}
	else if (cr->seccion & CR_JUE) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n de JUEGOS-CLANES", chan);
	}

        if (cr->geo & CR_AND) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Andaluc�a", chan);
	} 
	else if (cr->geo & CR_ARA ) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Arag�n", chan);
	}  else if (cr->geo & CR_AST ) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Principado de Asturias", chan);
	} else if (cr->geo & CR_BAL) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Islas Baleares", chan);
	} else if (cr->geo & CR_ICA) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Islas Canarias", chan);
	} else if (cr->geo & CR_CAN) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Cantabria", chan);
	} else if (cr->geo & CR_CAT) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Catalunya", chan);
	} else if (cr->geo & CR_CAM) {
       privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Castilla-La Mancha", chan);
	} else if (cr->geo & CR_CAL) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Castilla-Le�n", chan);
	}
	else if (cr->geo & CR_CEM) {
       privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Ceuta-Melilla", chan);
	} else if (cr->geo & CR_EUS) {
       privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Euskadi-Pa�s Vasco", chan);
	} else if (cr->geo & CR_EXT) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Extremadura", chan);
	} else if (cr->geo & CR_GAE) {
      privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Galicia", chan);
	} else if (cr->geo & CR_LRI) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de La Rioja", chan);
	} else if (cr->geo & CR_MAD) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Madrid", chan);
	} else if (cr->geo & CR_MUR) {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Murcia", chan);
	} else if (cr->geo & CR_NAV) {
    	 privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Navarra", chan);
    } else if (cr->geo & CR_VAL) {
    	 privmsg(s_CregServ, u->nick, "El canal 12%s est� en la Secci�n Geogr�fica de Valencia", chan);
	}

       
	if (cr->tipo & CR_COM ) {
        privmsg(s_CregServ, u->nick, "El canal 12%s es COMERCIAL", chan);
	} else if (cr->tipo & CR_OFI ) {
	       privmsg(s_CregServ, u->nick, "El canal 12%s es OFICIAL de la Red", chan);
	       } else if (cr->tipo & CR_REP ) {
	          privmsg(s_CregServ, u->nick, "El canal 12%s es de REPRESENTANTES de la Red", chan);
		  }
        
    }
}
                
/*************************************************************************
  Gestion para la visualizaci�n de apoyos de nicks y canales
    
               Implementado por ZipBreake :)
************************************************************************/
static void do_apoyos(User *u)
{
    char *chan = strtok(NULL, " ");
    CregInfo *cr; 
    ApoyosCreg *apoyos; 
    int i, f;
    struct tm *tm;
    char buf[BUFSIZE];
    int numero = 0; 
        
    if (!chan) { 
       privmsg(s_CregServ, u->nick, "Sintaxis: 12APOYOS <canal> | <nick>");
       return;
    } else if (!nick_identified(u)) {
        notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK, s_NickServ, s_NickServ);
        return;
    }
    
    if (*chan == '#') {

    if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado ni en proceso de registro por CReG",chan);
    } else {

                                  
        if ((stricmp(u->nick, cr->founder) == 0) || is_services_cregadmin(u)) {
	    privmsg(s_CregServ, u->nick, "Lista de apoyos de 12%s", chan);
	    for (apoyos = cr->apoyos, i = 0; i < cr->apoyoscount; apoyos++, i++) {
	          tm = localtime(&apoyos->time_apoyo);
	          strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
	          privmsg(s_CregServ, u->nick, "- %s (%s)", apoyos->nickapoyo, buf);
	          numero++;
	    }
	    privmsg(s_CregServ, u->nick, "Actualmente 12%d apoyos", numero);
        }
        else 
           privmsg(s_CregServ, u->nick, "4Acceso denegado!!");
    }        
    } else {

    if (!(stricmp(chan, u->nick) == 0) && !is_services_cregadmin(u)) {
	privmsg(s_CregServ, u->nick, "4Acceso denegado!!");
	return;
    }

    privmsg(s_CregServ, u->nick, "Lista de apoyos para 12%s", chan);
      
       for (i = 0; i < canales; i++) {
	    for (cr = creglists[i]; cr; cr = cr->next) {

            if (stricmp(chan, cr->founder) == 0) {
            privmsg(s_CregServ, u->nick, "- %s (Como fundador)", cr->name);
            }
            for (apoyos = cr->apoyos, f = 0; f < cr->apoyoscount; apoyos++, f++) {
             if (stricmp(chan, apoyos->nickapoyo) == 0) {
        tm = localtime(&apoyos->time_apoyo);
        strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
        
             privmsg(s_CregServ, u->nick, "- %s (%s)", cr->name, buf);
             }
            }
           }
       }	

           privmsg(s_CregServ, u->nick, "Fin de la lista");
    }
}

/*************************************************************************/


static void do_registra(User *u)
{
    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
   char  *seccion = strtok(NULL, " ");
    char *desc = strtok(NULL, "");
     
    NickInfo *ni = u->ni;
    CregInfo *cr, *cr2;
    ChannelInfo *ci;
     
    char passtemp[255];

                        
    if (!desc) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12REGISTRA <Canal> <Clave> <Secci�n> <Descripci�n>");
	privmsg(s_CregServ, u->nick, "teclea \00312/msg CREG seccion\003 para ver las secciones disponibles");
    } else if ((*chan == '&') || (*chan == '+')) {
        notice_lang(s_ChanServ, u, CHAN_REGISTER_NOT_LOCAL);
    } else if (!(*chan == '#')) {
        privmsg(s_CregServ, u->nick, "Canal no valido para registrar");
    } else if (!ni) {
        notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
        notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK, s_NickServ, s_NickServ);                                                    
    } else if ((ci = cs_findchan(chan)) != NULL) {
        if (ci->flags & CI_VERBOTEN) {
            notice_lang(s_CregServ, u, CHAN_MAY_NOT_BE_REGISTERED, chan);
        } else {
            notice_lang(s_CregServ, u, CHAN_ALREADY_REGISTERED, chan);
        }
    } else if ((cr2 = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal ya existe en %s. Teclee \00312/msg %s ESTADO %s\003 para ver su estado.", s_CregServ, s_CregServ, chan);     
    } else if ((stricmp(seccion, "SOC") != 0) &&  (stricmp(seccion, "INF") != 0) &&  (stricmp(seccion, "CIE") != 0) 
    &&  (stricmp(seccion, "AYU") != 0) && (stricmp(seccion, "ADU") != 0) &&  (stricmp(seccion, "PRO") != 0) 
    &&  (stricmp(seccion, "CUH") != 0) &&  (stricmp(seccion, "DEP") != 0) &&  (stricmp(seccion, "LCT") != 0)
    &&  (stricmp(seccion, "MUS") != 0) &&  (stricmp(seccion, "OCI") != 0) &&  (stricmp(seccion, "PAI") != 0) 
    &&  (stricmp(seccion, "PRF") != 0) &&  (stricmp(seccion, "AMO") != 0) &&  (stricmp(seccion, "SEX") != 0) 
    &&  (stricmp(seccion, "RAD") != 0) &&  (stricmp(seccion, "JUE") != 0) 
    &&  (stricmp(seccion, "AND") != 0) 
    &&  (stricmp(seccion, "ARA") != 0) &&  (stricmp(seccion, "AST") != 0) &&  (stricmp(seccion, "BAL") != 0)
    &&  (stricmp(seccion, "ICA") != 0) &&  (stricmp(seccion, "CAN") != 0) &&  (stricmp(seccion, "CAT") != 0) 
    &&  (stricmp(seccion, "CAM") != 0) &&  (stricmp(seccion, "CAL") != 0) &&  (stricmp(seccion, "CEM") != 0)
    &&  (stricmp(seccion, "EUS") != 0) &&  (stricmp(seccion, "EXT") != 0) &&  (stricmp(seccion, "GAE") != 0) 
    &&  (stricmp(seccion, "LRI") != 0) &&  (stricmp(seccion, "MAD") != 0) &&  (stricmp(seccion, "MUR") != 0) 
    &&  (stricmp(seccion, "NAV") != 0) && (stricmp(seccion, "VAL") != 0)) {
       privmsg(s_CregServ, u->nick, "Seccion InCorrecta,teclea \00312/msg CREG seccion\003 para ver las disponibles"); 
       } /*
   else if (ni->channelmax > 0 && ni->channelcount >= ni->channelmax) {
        notice_lang(s_CregServ, u, ni->channelcount > ni->channelmax ? CHAN_EXCEEDED_CHANNEL_LIMIT : CHAN_REACHED_CHANNEL_LIMIT, ni->channelmax);

    } */else if (!(cr = makecreg(chan))) {
        logeo("%s: makecreg() failed for REGISTER %s", s_CregServ, chan);
        notice_lang(s_CregServ, u, CHAN_REGISTRATION_FAILED);                             
    }  
    
       
	else    {
        cr->founder =  ni->nick; 
        if (strlen(pass) > PASSMAX-1) /* -1 for null byte */
            notice_lang(s_CregServ, u, PASSWORD_TRUNCATED, PASSMAX-1);
        strscpy(cr->founderpass, pass, PASSMAX);
                               
        cr->time_lastapoyo = time(NULL);
        cr->email = sstrdup(ni->email);
	if (stricmp(seccion, "SOC") == 0) 
	   {  cr->seccion = CR_SOC; }
	     else if (stricmp(seccion, "INF") == 0)
	       { cr->seccion = CR_INF; }
	         else if (stricmp(seccion, "CIE") == 0)
	      		  { cr->seccion = CR_CIE; }
			   else if (stricmp(seccion, "AYU") == 0)
	      		  	{ cr->seccion = CR_AYU; }
				 else if (stricmp(seccion, "ADU") == 0)
	      		  		{ cr->seccion = CR_ADU; }
					 else if (stricmp(seccion, "PRO") == 0)
	      		  		{ cr->seccion = CR_PRO; }
					else if (stricmp(seccion, "CUH") == 0)
	      		  		{ cr->seccion = CR_CUH; }
					else if (stricmp(seccion, "DEP") == 0)
	      		  		{ cr->seccion = CR_DEP; }
					else if (stricmp(seccion, "LCT") == 0)
	      		  		{ cr->seccion = CR_LCT; }
					else if (stricmp(seccion, "MUS") == 0)
	      		  		{ cr->seccion = CR_MUS; }
					else if (stricmp(seccion, "OCI") == 0)
	      		  		{ cr->seccion = CR_OCI; }
					else if (stricmp(seccion, "PAI") == 0)
	      		  		{ cr->seccion = CR_PAI; }
					else if (stricmp(seccion, "PRF") == 0)
	      		  		{ cr->seccion = CR_PRF; }
					else if (stricmp(seccion, "AMO") == 0)
	      		  		{ cr->seccion = CR_AMO; }
					else if (stricmp(seccion, "SEX") == 0)
	      		  		{ cr->seccion = CR_SEX; }
					else if (stricmp(seccion, "RAD") == 0)
	      		  		{ cr->seccion = CR_RAD; }
					else if (stricmp(seccion, "JUE") == 0)
	      		  		{ cr->seccion = CR_JUE; }
		     			else if (stricmp(seccion, "AND") == 0) {
	      			 	cr->geo = CR_AND; }
					else if (stricmp(seccion, "ARA") == 0)
	      		  		{ cr->geo = CR_ARA; }
					else if (stricmp(seccion, "AST") == 0)
	      		  		{ cr->geo = CR_AST; }
					else if (stricmp(seccion, "BAL") == 0)
	      		  		{ cr->geo = CR_BAL; }
					else if (stricmp(seccion, "ICA") == 0)
	      		  		{ cr->geo = CR_ICA; }
					else if (stricmp(seccion, "CAN") == 0)
	      		  		{ cr->geo = CR_CAN; }
					else if (stricmp(seccion, "CAT") == 0)
	      		  		{ cr->geo = CR_CAT; }
					else if (stricmp(seccion, "CAM") == 0)
	      		  		{ cr->geo = CR_CAM; }
					else if (stricmp(seccion, "CAL") == 0)
	      		  		{ cr->geo = CR_CAL; }
					else if (stricmp(seccion, "CEM") == 0)
	      		  		{ cr->geo = CR_CEM; }
					else if (stricmp(seccion, "EUS") == 0)
	      		  		{ cr->geo = CR_EUS; }
					else if (stricmp(seccion, "EXT") == 0)
	      		  		{ cr->geo = CR_EXT; }
					else if (stricmp(seccion, "GAE") == 0)
	      		  		{ cr->geo = CR_GAE; }
					else if (stricmp(seccion, "LRI") == 0)
	      		  		{ cr->geo = CR_LRI; }
					else if (stricmp(seccion, "MAD") == 0)
	      		  		{ cr->geo = CR_MAD; }
					else if (stricmp(seccion, "MUR") == 0)
	      		  		{ cr->geo = CR_MUR; }
					else if (stricmp(seccion, "NAV") == 0)
	      		  		{ cr->geo = CR_NAV; }
	      		  	else if (stricmp(seccion, "VAL") == 0)
	      		  		{ cr->geo = CR_VAL; }
					
					
	#if defined(SOPORTE_JOOMLA15)
/*id           | int(11)      | NO   | PRI | NULL       | auto_increment |
| seccion      | varchar(3)      | NO   |     | 0          |                |
| canal        | varchar(150) | NO   | MUL |            |                |
| username     | varchar(150) | NO   | MUL |            |                |
| registerDate | varchar(100) | NO   |     |            |                |
| status       | varchar(100) |
			*/

MYSQL *conn;
char inserta[BUFSIZE], buf[BUFSIZE];
 conn = mysql_init(NULL);
struct tm *tm;
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }

tm = localtime(&cr->time_lastapoyo);
strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
if (CregApoyos) {
snprintf(inserta, sizeof(inserta), "INSERT INTO jos_chans VALUES('','%s','%s','%s','%s','PROCESO')", seccion,chan,cr->founder,buf);
} else {
 snprintf(inserta, sizeof(inserta), "INSERT INTO jos_chans VALUES('','%s','%s','%s','%s','ACEPTADO')", seccion,chan,cr->founder,buf);
        	
	}

if (mysql_query(conn,inserta)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif				
					    
       cr->desc = sstrdup(desc);
        cr->time_peticion = time(NULL);

        srand(time(NULL));
        sprintf(passtemp, "%05u",1+(int)(rand()%99999) );
        strscpy(cr->passapoyo, passtemp, PASSMAX);

       	privmsg(s_CregServ, u->nick, "El canal 12%s ha sido aceptado para el registro.", chan); 
        if (CregApoyos) {
	        cr->estado = CR_PROCESO_REG;
        	privmsg(s_CregServ, u->nick,"En los proximos 7 d�as necesitara %i apoyos para que sea registrado.", CregApoyos);
	} else {
                cr->estado = CR_ACEPTADO;
        	privmsg(s_CregServ, u->nick,"En breve la comisi�n de canales lo evaluar� y registrar� o denegar�.");
	}
       	canalopers(s_CregServ, "12%s ha solicitado el registro del canal 12%s", u->nick, chan);
    } 
}                                                                        
                                                        


/*************************************************************************/


static void do_reg(User *u)
{

    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
   char  *tipo = strtok(NULL, " ");
    char *desc = strtok(NULL, "");
     
    NickInfo *ni = u->ni;
    CregInfo *cr, *cr2;
    ChannelInfo *ci;
     
    char passtemp[255];
    
 
                            
    if (!desc) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12REG <Canal> <Clave> <Secci�n> <Descripci�n>");
	  privmsg(s_CregServ, u->nick, "teclea \00312/msg CREG seccion\003 para ver las secciones disponibles"); 
       
    } else if ((*chan == '&') || (*chan == '+')) {
        notice_lang(s_ChanServ, u, CHAN_REGISTER_NOT_LOCAL);
    } else if (!(*chan == '#')) {
        privmsg(s_CregServ, u->nick, "Canal no valido para registrar");
    } else if (!ni) {
        notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
        notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK, s_NickServ, s_NickServ);                                                    
    } else if ((ci = cs_findchan(chan)) != NULL) {
        if (ci->flags & CI_VERBOTEN) {
            notice_lang(s_CregServ, u, CHAN_MAY_NOT_BE_REGISTERED, chan);
        } else {
            notice_lang(s_CregServ, u, CHAN_ALREADY_REGISTERED, chan);
        }
    } else if ((cr2 = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal ya existe en %s. Teclee \00312/msg %s ESTADO %s\003 para ver su estado.", s_CregServ, s_CregServ, chan);     
    } else if ((stricmp(tipo, "COM") != 0) &&  (stricmp(tipo, "OFI") != 0) &&  (stricmp(tipo, "REP") != 0 )) {
       privmsg(s_CregServ, u->nick, "Seccion InCorrecta,teclea \00312/msg CREG seccion\003 para ver las disponibles"); 
       }
    else if (ni->channelmax > 0 && ni->channelcount >= ni->channelmax) {
        notice_lang(s_CregServ, u, ni->channelcount > ni->channelmax ? CHAN_EXCEEDED_CHANNEL_LIMIT : CHAN_REACHED_CHANNEL_LIMIT, ni->channelmax);

    } else if (!(cr = makecreg(chan))) {
        logeo("%s: makecreg() failed for REGISTER %s", s_CregServ, chan);
        notice_lang(s_CregServ, u, CHAN_REGISTRATION_FAILED);                             
    }  
    
       
	else    {
        cr->founder =  ni->nick; 
        if (strlen(pass) > PASSMAX-1) /* -1 for null byte */
            notice_lang(s_CregServ, u, PASSWORD_TRUNCATED, PASSMAX-1);
        strscpy(cr->founderpass, pass, PASSMAX);
                               
        cr->time_lastapoyo = time(NULL);
        cr->email = sstrdup(ni->email);
	if (stricmp(tipo, "COM") == 0) 
	   {  cr->tipo = CR_COM; }
	     else if (stricmp(tipo, "OFI") == 0)
	       { cr->tipo = CR_OFI; }
	         else if (stricmp(tipo, "REP") == 0)
	      		  { cr->tipo = CR_REP; }
			   					    
	#if defined(SOPORTE_JOOMLA15)
/*id           | int(11)      | NO   | PRI | NULL       | auto_increment |
| seccion (*)  | varchar(3)   | NO   |     | 0          |                |
| canal        | varchar(150) | NO   | MUL |            |                |
| username     | varchar(150) | NO   | MUL |            |                |
| registerDate | varchar(100) | NO   |     |            |                |
| status (*)   | varchar(100) |

(*)�nica diferencia es que son 3 tipos especiales de canal en lugar de las secciones gen�ricas
   y es Aceptado en espera de ser Registrado
			*/



MYSQL *conn;
char inserta[BUFSIZE], buf[BUFSIZE];
 conn = mysql_init(NULL);
struct tm *tm;
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }

tm = localtime(&cr->time_lastapoyo);
strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
snprintf(inserta, sizeof(inserta), "INSERT INTO jos_chans VALUES('','%s','%s','%s','%s','ACEPTADO')", tipo,chan,cr->founder,buf);

if (mysql_query(conn,inserta)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif							  
			
        cr->desc = sstrdup(desc);
        cr->time_peticion = time(NULL);

        srand(time(NULL));
        sprintf(passtemp, "%05u",1+(int)(rand()%99999) );
        strscpy(cr->passapoyo, passtemp, PASSMAX);

       	privmsg(s_CregServ, u->nick, "El canal 12%s ha sido aceptado para el registro.", chan); 
        

                cr->estado = CR_ACEPTADO;
        	
    } 
}                                                                                 
/*******************************FIN APOYOS***************************/

static void do_list(User *u)
{
    char *pattern = strtok(NULL, " ");
    char *keyword;
    CregInfo *cr;
    int nchans, i;
    char buf[BUFSIZE];
    int is_servoper = is_services_oper(u);
    int16 matchflags = 0;

    if (!pattern) {
	syntax_error(s_CregServ, u, "LIST", CHAN_LIST_SYNTAX);
    } else {
	nchans = 0;
	
	while (is_servoper && (keyword = strtok(NULL, " "))) {
	    if (stricmp(keyword, "ENREGISTRO") == 0)
	        matchflags |= CR_PROCESO_REG;
	    if (stricmp(keyword, "REGISTRADO") == 0)
	        matchflags |= CR_REGISTRADO;
	    if (stricmp(keyword, "PENDIENTE") == 0)
	        matchflags |= CR_ACEPTADO;
	   if (stricmp(keyword, "DENEGADO") == 0)
	        matchflags |= CR_RECHAZADO;
	    if (stricmp(keyword, "MARCADO") == 0)
	        matchflags |= CR_MARCADO;
	     if (stricmp(keyword, "SUSPENDIDO") == 0)  
	        matchflags |=CR_SUSPENDIDO;
	    if (stricmp(keyword, "EXPIRADO") == 0)
	        matchflags |= CR_EXPIRADO;
	 
	}                
	notice_lang(s_CregServ, u, CHAN_LIST_HEADER, pattern);
	for (i = 0; i < canales; i++) {
	    for (cr = creglists[i]; cr; cr = cr->next) {
		if (!is_servoper & ((cr->flags & CR_PROCESO_REG)
		                              || (cr->flags & CR_REGISTRADO)
		                              || (cr->flags & CR_ACEPTADO)
		                              || (cr->flags & CR_DROPADO)
		                              || (cr->flags & CR_RECHAZADO)
		                              || (cr->flags & CR_MARCADO)
		                              || (cr->flags & CR_EXPIRADO)
		                              	                              
		                              ))
		    continue;
		if ((matchflags != 0) && !(cr->estado & matchflags))
		    continue;
		        
		snprintf(buf, sizeof(buf), "%-20s  %s", cr->name,
						cr->desc ? cr->desc : "");
		if (stricmp(pattern, cr->name) == 0 ||
					match_wild_nocase(pattern, buf)) {
		    if (++nchans <= CSListMax) {
			if (cr->estado & CR_ACEPTADO) {
		            snprintf(buf, sizeof(buf), "%-20s [PENDIENTE] %s",
		                        cr->name, cr->desc);
		        }
                        if (cr->estado & CR_PROCESO_REG) {
                            snprintf(buf, sizeof(buf), "%-20s [P.APOYOS] %s",
                                        cr->name, cr->desc);
                        }		                       
			if (cr->estado & CR_REGISTRADO) {
                            snprintf(buf, sizeof(buf), "%-20s [REGISTRADO] %s",
                                        cr->name, cr->desc);
                        }		                       
			if (cr->estado & CR_RECHAZADO) {
                            snprintf(buf, sizeof(buf), "%-20s [RECHAZADO] %s",
                                        cr->name, cr->desc);
                                       
                        }
             if (cr->estado & CR_RECHAZADO) {
                            snprintf(buf, sizeof(buf), "%-20s [RECHAZADO] %s",
                                        cr->name, cr->desc);
              }	
			if (cr->estado & CR_SUSPENDIDO) {
                            snprintf(buf, sizeof(buf), "%-20s [SUSPENDIDO] %s",
                                        cr->name, cr->desc);
                        }		                       
			if (cr->estado & CR_DROPADO) {
                            snprintf(buf, sizeof(buf), "%-20s [DROPADO] %s",
                                        cr->name, cr->desc);
                        }		                       
			if (cr->estado & CR_EXPIRADO) {
                            snprintf(buf, sizeof(buf), "%-20s [EXPIRADO] %s",
                                        cr->name, cr->desc);
                        }
			if (cr->tipo & CR_COM) {
                            snprintf(buf, sizeof(buf), "%-20s [COMERCIAL] %s",
                                        cr->name, cr->desc);
                        }
			if (cr->tipo & CR_OFI) {
                            snprintf(buf, sizeof(buf), "%-20s [OFICIAL] %s",
                                        cr->name, cr->desc);
                        }
			if (cr->tipo & CR_REP) {
                            snprintf(buf, sizeof(buf), "%-20s [REPRESENTANTES] %s",
                                        cr->name, cr->desc);
                        }				                   	                       
			privmsg(s_CregServ, u->nick, "  %s", buf);
		    }
		}
	    }
	}
	notice_lang(s_CregServ, u, CHAN_LIST_END,
			nchans>CSListMax ? CSListMax : nchans, nchans);
    }
}


/************************INFORMACION DE CANALES*********************/
static void do_info(User *u)
{
    char *canal = strtok(NULL, " ");
    char *statu;
    CregInfo *cr;
    ChannelInfo *ci;
    ApoyosCreg *apoyos; 
    int i; 
    char buf[BUFSIZE];
    struct tm *tm;
    NickInfo *ni;

  
    if (!canal) {
        privmsg(s_CregServ, u->nick, "Sintaxis 12INFO <canal>");
        return;
    } else if (!(cr = cr_findcreg(canal))) {
        if (!(ci = cs_findchan(canal)))
          privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado", canal);
        else {
         if (ci->flags & CI_VERBOTEN)
          privmsg(s_CregServ, u->nick, "El canal 12%s no puede ser registrado ni utilizado", canal);
         else              
          privmsg(s_CregServ, u->nick, "El canal 12%s existe en 12CHaN pero no est� registrado con 12CReG", canal);
        }
    } else {
      cr=cr_findcreg(canal);

        if (cr->estado & CR_PROCESO_REG) {
               statu="PENDIENTE DE APOYOS";
        } else if (cr->estado & CR_REGISTRADO) {
         if (!(ci = cs_findchan(canal))) {
             statu="DESCONOCIDO";
         } else {
           if (ci->flags & CI_SUSPEND) {
               statu="SUSPENDIDO";
           } else {    
               statu="REGISTRADO";
           }
         }
        } else if (cr->estado & CR_ACEPTADO) {
               statu="PENDIENTE DE APROBACION";
        } else if (cr->estado & CR_RECHAZADO) {
               statu="DENEGADO";
	} else if (cr->estado & CR_MARCADO) {
               statu="MARCADO";
	} else if (cr->estado & CR_SUSPENDIDO) {
               statu="SUSPENDIDO";
        } else if (cr->estado & CR_DROPADO) {
               statu="DROPADO EN CHAN";
        } else if (cr->estado & CR_EXPIRADO) {
               statu="EXPIRO EN CHAN";
        } else {
               statu="DESCONOCIDO";
        }
        privmsg(s_CregServ, u->nick, "Informaci�n del canal 12%s", canal);

          if ((ni = findnick(cr->founder)))
	      privmsg(s_CregServ, u->nick, "Fundador: 12%s (12%s) - IP: (12%s)", cr->founder, cr->email, ni->last_usermask);
          else
	      privmsg(s_CregServ, u->nick, "Fundador: 12%s (12%s) - Nick no registrado", cr->founder, cr->email);
        
	tm = localtime(&cr->time_peticion);
        strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
        privmsg(s_CregServ, u->nick, "Fecha petici�n: 12%s", buf);



        privmsg(s_CregServ, u->nick, "Estado: 12%s", statu);
        if (cr->estado & CR_PROCESO_REG) {       
	tm = localtime(&cr->time_lastapoyo);
        strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
        privmsg(s_CregServ, u->nick, "Ultimo apoyo: 12%s", buf);
        }
         if ((cr->estado & CR_MARCADO) || (cr->estado & CR_SUSPENDIDO)) {
       		if (cr->motivo)        
             		privmsg(s_CregServ, u->nick, "Motivo: 12%s", cr->motivo);
        	 }

        if (cr->nickoper)        
             privmsg(s_CregServ, u->nick, "Ultimo cambio por: 12%s", cr->nickoper);
        if (cr->time_motivo) {
	   tm = localtime(&cr->time_motivo);
           strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
           privmsg(s_CregServ, u->nick, "Fecha cambio: 12%s", buf);
	}
        privmsg(s_CregServ, u->nick, "Descripci�n: 12%s", cr->desc);
        privmsg(s_CregServ, u->nick, "Apoyos:");
        for (apoyos = cr->apoyos, i = 0; i < cr->apoyoscount; apoyos++, i++) {
          if ((ni = findnick(apoyos->nickapoyo)))
              privmsg(s_CregServ, u->nick, "12%s (12%s) - IP: (12%s)", apoyos->nickapoyo, apoyos->emailapoyo, ni->last_usermask);
          else
              privmsg(s_CregServ, u->nick, "12%s (12%s) - Nick  no registrado", apoyos->nickapoyo, apoyos->emailapoyo);
        }
        privmsg(s_CregServ, u->nick, "Fin del INFO de CReG.");
    }
}
static MemoInfo *getmemoinfo(const char *name, int *ischan)
{
    if (*name == '#') {
	ChannelInfo *ci;
	if (ischan)
	    *ischan = 1;
	ci = cs_findchan(name);
	if (ci)
	    return &ci->memos;
	else
	    return NULL;
    } else {
	NickInfo *ni;
	if (ischan)
	    *ischan = 0;
	ni = findnick(name);
	if (ni)
	    return &getlink(ni)->memos;
	else
	    return NULL;
    }
}
/******************ACEPTACION DE UN CANAL Y REG EN CHAN**********/
static void do_acepta(User *u)
{
    char *chan = strtok(NULL, " ");
    CregInfo *cr;
    NickInfo *ni;

    if (!chan) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12ACEPTA <canal>");
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no registrado en CReG", chan);
    } else if (!(ni = findnick(cr->founder))) {
        privmsg(s_CregServ, u->nick, "El fundador del canal 12%s no tiene el nick registrado", chan);
    } else if (!(cr->estado & CR_ACEPTADO)) {                                           
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� pendiente de aprobacion", chan); 
    /*} else if (!is_chanop(u->nick, chan)) {                                           
        privmsg(s_CregServ, u->nick, "Debes estar en el canal 12%s y tener OP para aceptarlo", chan);
    */} else if (!registra_con_creg(u, ni, cr->name, cr->founderpass, cr->desc)) {
        privmsg(s_CregServ, u->nick, "El registro del canal 12%s en %s ha fallado", chan, s_ChanServ); 
    } else {
        cr=cr_findcreg(chan);
        cr->time_motivo = time(NULL);
        cr->nickoper = sstrdup(u->nick);
        cr->estado = 0;
        cr->estado |= CR_REGISTRADO;
#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
char actualiza[BUFSIZE];
 conn = mysql_init(NULL);
 char buf[BUFSIZE];
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(actualiza, sizeof(actualiza), "update jos_chans set status ='Registrado' where canal ='%s';",chan);

if (mysql_query(conn,actualiza)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
         
 
        canalopers(s_CregServ, "4%s ha aceptado el canal 12%s", u->nick, chan);
        privmsg(s_CregServ, u->nick, "12%s ha sido aceptado en %s", chan, s_ChanServ); 
	send_cmd(s_CregServ,"TOPIC %s :Este canal ha sido 12ACEPTADO en su Registro", chan);

       	/*soporte envio de memos a los founders de los nuevos canales registrados que han sido aceptados*/
          
       MemoInfo *mi;
       Memo *m;
	int ischan;
        /*if (!(mi = getmemoinfo(cr->founder, &ischan))) 
	notice_lang(s_MemoServ, u,
		ischan ? CHAN_X_NOT_REGISTERED : NICK_X_NOT_REGISTERED, cr->founder);*/
        mi = getmemoinfo(cr->founder, &ischan);
        time_t now = time(NULL);
        u->lastmemosend = now;
	char *source =  u->nick;
        mi->memocount++;
        mi->memos = srealloc(mi->memos, sizeof(Memo) * mi->memocount);
	m = &mi->memos[mi->memocount-1];
	strscpy(m->sender, source, NICKMAX);
	if (mi->memocount > 1) {
	    m->number = m[-1].number + 1;
	    if (m->number < 1) {
		int i;
		for (i = 0; i < mi->memocount; i++)
		    mi->memos[i].number = i+1;
	    }
	} else {
	    m->number = 1;
	}
	m->time = time(NULL);
	char text[BUFSIZE];
    	snprintf(text, sizeof(text), "Nos Complace comunicarle, que la admnistraci�n de canales de la red, despues de haber revisado su solicitud,ha resuelto darle por 3ACEPTADO su petici�n de registro, de su nuevo canal 2%#s .No dude si lo considera necesario, solicitar soporte en el canal 4#%s .Un Saludo", chan,CanalAyuda);
	m->text =sstrdup(text);
	m->flags = MF_UNREAD;
  

	    if (ni->flags & NI_MEMO_RECEIVE) {
		if (MSNotifyAll) {
		    for (u = firstuser(); u; u = nextuser()) {
			if (u->real_ni == ni) {
			    notice_lang(s_MemoServ, u, MEMO_NEW_MEMO_ARRIVED,
					source, s_MemoServ, m->number);
			}
		    }
		
		} /* if (MSNotifyAll) */
	    } /* if (flags & MEMO_RECEIVE) */
    }
}


/******************ACEPTACION DE UN CANAL Y REG EN CHAN**********/
static void do_deniega(User *u)
{
    char *chan = strtok(NULL, " ");
    char *razon = strtok(NULL, "");
    CregInfo *cr;
    NickInfo *ni;
     if ( (!chan) || (!razon)) {
	
       privmsg(s_CregServ, u->nick, " Sint�xis: 12DENIEGA <canal> <raz�n>");
    	return;
    
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no registrado en CReG", chan);
    } else if (!(ni = findnick(cr->founder))) {
        privmsg(s_CregServ, u->nick, "El fundador del canal 12%s no tiene el nick registrado", chan);
    } else if (!(cr->estado & CR_ACEPTADO)) {                                           
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� pendiente de aprobaci�n", chan); 
    /*} else if (!is_chanop(u->nick, chan)) {                                           
        privmsg(s_CregServ, u->nick, "Debes estar en el canal 12%s y tener OP para aceptarlo", chan);
    */
    } else {
        cr=cr_findcreg(chan);
        cr->time_motivo = time(NULL);
        cr->nickoper = sstrdup(u->nick);
        cr->estado = 0;
        cr->estado |= CR_RECHAZADO;
#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
char actualiza[BUFSIZE];
 conn = mysql_init(NULL);
 char buf[BUFSIZE];
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(actualiza, sizeof(actualiza), "update jos_chans set status ='Denegado' where canal ='%s';",chan);

if (mysql_query(conn,actualiza)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
         
 
        canalopers(s_CregServ, "4%s ha denegado el canal 12%s", u->nick, chan);
        privmsg(s_CregServ, u->nick, "12%s ha sido denegado en %s", chan, s_ChanServ); 
	send_cmd(s_CregServ,"TOPIC %s :Este canal ha sido 5RECHAZADO en su Registro ( 12%s )", chan,razon);

       	/*soporte envio de memos a los founders de los nuevos canales registrados que han sido aceptados*/
          
       MemoInfo *mi;
       Memo *m;
	int ischan;
        /*if (!(mi = getmemoinfo(cr->founder, &ischan))) 
	notice_lang(s_MemoServ, u,
		ischan ? CHAN_X_NOT_REGISTERED : NICK_X_NOT_REGISTERED, cr->founder);*/
        mi = getmemoinfo(cr->founder, &ischan);
        time_t now = time(NULL);
        u->lastmemosend = now;
	char *source =  u->nick;
        mi->memocount++;
        mi->memos = srealloc(mi->memos, sizeof(Memo) * mi->memocount);
	m = &mi->memos[mi->memocount-1];
	strscpy(m->sender, source, NICKMAX);
	if (mi->memocount > 1) {
	    m->number = m[-1].number + 1;
	    if (m->number < 1) {
		int i;
		for (i = 0; i < mi->memocount; i++)
		    mi->memos[i].number = i+1;
	    }
	} else {
	    m->number = 1;
	}
	m->time = time(NULL);
	char text[BUFSIZE];
    	snprintf(text, sizeof(text), "Lamentamos comunicarle,que la admnistraci�n de canales de la red, despues de haber revisado su solicitud,ha resuelto darle por 4DENEGADO su petici�n de registro,del canal 2%#s .Motivo 5%s.Un Saludo", chan,razon);
	m->text =sstrdup(text);
	m->text =sstrdup(text);
	m->flags = MF_UNREAD;
  

	    if (ni->flags & NI_MEMO_RECEIVE) {
		if (MSNotifyAll) {
		    for (u = firstuser(); u; u = nextuser()) {
			if (u->real_ni == ni) {
			    notice_lang(s_MemoServ, u, MEMO_NEW_MEMO_ARRIVED,
					source, s_MemoServ, m->number);
			}
		    }
		
		} /* if (MSNotifyAll) */
	    } /* if (flags & MEMO_RECEIVE) */
    }
}




/******************MARCADO DE UN CANAL *********/
static void do_marcar(User *u)
{
   char *chan = strtok(NULL, " ");
    char *razon = strtok(NULL, "");
    CregInfo *cr;
     NickInfo *ni;
    if ( (!chan) || (!razon)) {
      privmsg(s_CregServ, u->nick, "Sintaxis: 12MARCAR <canal> 2 <motivo>");
    	return;
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado en CReG", chan);
    } else if ((cr->estado & CR_PROCESO_REG))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� en proceso de registro con CReG", chan);
    } else if ((cr->estado & CR_EXPIRADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� expirado con CReG", chan);
     } else if ((cr->estado & CR_RECHAZADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� rechazado con CReG", chan);
      } else if ((cr->estado & CR_DROPADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� dropado con CReG", chan);
      } else if ((cr->estado & CR_ACEPTADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� pendiente de 12APROBACION por parte de la administraci�n de la red", chan);

    } else if ((cr->estado & CR_MARCADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s Ya est� MARCADO  en CReG", chan);
     } else if ((cr->estado & CR_SUSPENDIDO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s est� SUSPENDIDO  en CReG", chan);
      /*si esta suspendido el canal ,no necesario marcarlo porque ya tiene su razon*/
     } else {
      
        cr->motivo = sstrdup(razon);
        cr->time_motivo = time(NULL);
        cr->nickoper = sstrdup(u->nick);
        cr->estado = 0;
        cr->estado |= CR_MARCADO;
        privmsg(s_CregServ, u->nick, "Al canal 12%s se le ha MARCADO", chan);
        canaladmins(s_CregServ, "4%s ha MARCADO el Canal  12%s", u->nick, chan);; 
    }
}
/******************DESMARCAR UN CANAL *********/
static void do_desmarcar(User *u)
{
   char *chan = strtok(NULL, " ");
      CregInfo *cr;
     NickInfo *ni;
  
    if (!chan) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12DESMARCAR <canal>");
        return;
      }
    
    if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado en CReG", chan);
         return;
        
    } else if (!(cr->estado & CR_MARCADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� MARCADO  en CReG", chan);
     
     } else {
        if (cr->motivo)
	free(cr->motivo);
        cr->nickoper = sstrdup(u->nick);
        cr->estado  &= ~CR_MARCADO;
        cr->estado |= CR_REGISTRADO;
        privmsg(s_CregServ, u->nick, "Al canal 12%s se le ha DESMARCADO", chan);
        canaladmins(s_CregServ, "4%s ha DESMARCADO el Canal  12%s", u->nick, chan);; 
    }
}


/******************SUSPENSION DE UN CANAL *********/
static void do_suspend(User *u)
{
   char *chan = strtok(NULL, " ");
    char *razon = strtok(NULL, "");
    CregInfo *cr;
     NickInfo *ni;

    if (!razon) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12SUSPEND <canal> 2 <motivo>");
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado en CReG", chan);
    } else if ((cr->estado & CR_SUSPENDIDO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s Ya est� SUSPENDIDO  en CReG", chan);
      } else if (!suspende_con_creg(u, chan,  razon)) {
        privmsg(s_CregServ, u->nick, "La Suspensi�n del canal 12%s en %s ha fallado", chan, s_ChanServ);
     } else {
        if (cr->motivo)
	free(cr->motivo);
        cr->motivo = sstrdup(razon);
        cr->time_motivo = time(NULL);
        cr->nickoper = sstrdup(u->nick);
        cr->estado = 0;
        cr->estado |= CR_SUSPENDIDO;
        privmsg(s_CregServ, u->nick, "Al canal 12%s se le ha SUSPENDIDO", chan);
        canaladmins(s_CregServ, "4%s ha SUSPENDIDO el Canal  12%s", u->nick, chan);
#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
char actualiza[BUFSIZE];
 conn = mysql_init(NULL);
 char buf[BUFSIZE];
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(actualiza, sizeof(actualiza), "update jos_chans set status ='SUSPENDIDO' where canal ='%s';",chan);

if (mysql_query(conn,actualiza)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
    }
}
/******************UNSUSPENSION DE UN CANAL *********/
static void do_unsuspend(User *u)
{
   char *chan = strtok(NULL, " ");
      CregInfo *cr;
     NickInfo *ni;
  
    if (!chan) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12UNSUSPEND <canal>");
        return;
      }
    
    if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� registrado en CReG", chan);
         return;
        
    } else if ((cr->estado & CR_REGISTRADO))  {
        privmsg(s_CregServ, u->nick, "El canal 12%s no est� SUSPENDIDO  en CReG", chan);
      } else if (!reactiva_con_creg(u, chan)) {
        privmsg(s_CregServ, u->nick, "La unSuspensi�n del canal 12%s en %s ha fallado", chan, s_ChanServ);
     } else {
        if (cr->motivo)
	free(cr->motivo);
        cr->nickoper = sstrdup(u->nick);
        cr->estado = 0;
        cr->estado |= CR_REGISTRADO;
        privmsg(s_CregServ, u->nick, "Al canal 12%s se le ha REACTIVADO", chan);
        canaladmins(s_CregServ, "4%s ha REACTIVADO el Canal  12%s", u->nick, chan);
	
 #if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
char actualiza[BUFSIZE];
 conn = mysql_init(NULL);
 char buf[BUFSIZE];
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(actualiza, sizeof(actualiza), "update jos_chans set status ='REGISTRADO' where canal ='%s';",chan);

if (mysql_query(conn,actualiza)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
    }
}




static void do_drop(User *u)
{
    char *chan = strtok(NULL, " ");
    CregInfo *cr;
    ChannelInfo *cr2;

    if (!chan) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12DROP <canal>");
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no registrado en CReG", chan);
    } else if ((cr2 = cs_findchan(chan))) {                                           
        privmsg(s_CregServ, u->nick, "El canal 12%s est� registrado en CHaN y no puede ser dropado por CReG", chan); 

    } else {
        cr=cr_findcreg(chan);
        delcreg(cr);    
        privmsg(s_CregServ, u->nick, "El canal 12%s ha sido dropado de CReG", chan); 
        canalopers(s_CregServ, "4%s ha dropado el canal 12%s", u->nick, chan);
	#if defined(SOPORTE_JOOMLA15)
MYSQL *conn;
char modifica[BUFSIZE];
 conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(modifica, sizeof(modifica), "delete from jos_chans where canal ='%s';",chan);
if (mysql_query(conn,modifica)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif
      
    }
}


static void do_fuerza(User *u)
{
    char *chan = strtok(NULL, " ");
    CregInfo *cr;
    NickInfo *ni;
  

    if (!chan) {
        privmsg(s_CregServ, u->nick, "Sintaxis: 12FUERZA <canal>");
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no registrado en CReG", chan);
    } else if (!(ni = findnick(cr->founder))) {
        privmsg(s_CregServ, u->nick, "El fundador del canal 12%s no tiene el nick registrado", chan);
    } /*else if (!((cr->estado & CR_RECHAZADO) || (cr->estado & CR_PROCESO_REG))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no puede ser registrado de forma forzada", chan); 
    } else if (!is_chanop(u->nick, chan)) {              
        privmsg(s_CregServ, u->nick, "Debes estar en el canal 12%s y tener OP para aceptarlo", chan); 
     }*/ else if (!registra_con_creg(u, ni, cr->name, cr->founderpass, cr->desc)) {
        privmsg(s_CregServ, u->nick, "El registro del canal 12%s en %s ha fallado", chan, s_ChanServ); 
    } else {
        cr=cr_findcreg(chan);
        cr->time_motivo = time(NULL);
        cr->nickoper = sstrdup(u->nick);
        cr->estado = 0;
        cr->estado |= CR_REGISTRADO;
	send_cmd(s_CregServ, "PRIVMSG  %s  :Felicidades Por el Registro De Su Nuevo Canal %s",cr->founder,chan);
        send_cmd(s_CregServ,"TOPIC %s :Este canal ha sido ACEPTADO en su Registro", chan);
      
	canalopers(s_CregServ, "4%s ha forzado la aceptaci�n del canal 12%s", u->nick, chan);
        privmsg(s_CregServ, u->nick, "12%s ha sido registrado en %s", chan, s_ChanServ);

#if defined(SOPORTE_JOOMLA15)
 MYSQL *conn;
char actualiza[BUFSIZE];
 conn = mysql_init(NULL);
 char buf[BUFSIZE];
   /* Connect to database */
   if (!mysql_real_connect(conn, MYSQL_SERVER,
         MYSQL_USER,MYSQL_PASS,MYSQL_DATABASE, 0, NULL, 0)) {
       canaladmins(s_CregServ,"%s\n", mysql_error(conn));
         return;
   }
snprintf(actualiza, sizeof(actualiza), "update jos_chans set status ='Registrado' where canal ='%s';",chan);

if (mysql_query(conn,actualiza)) 
canaladmins(s_CregServ, "%s\n", mysql_error(conn));
 mysql_close(conn);

#endif

/*soporte envio de memos a los founders de los nuevos canales registrados que han sido forzados*/
          
       MemoInfo *mi;
       Memo *m;
	int ischan;
        /*if (!(mi = getmemoinfo(cr->founder, &ischan))) 
	notice_lang(s_MemoServ, u,
		ischan ? CHAN_X_NOT_REGISTERED : NICK_X_NOT_REGISTERED, cr->founder);*/
	mi = getmemoinfo(cr->founder, &ischan);
        time_t now = time(NULL);
        u->lastmemosend = now;
	char *source =  u->nick;
        mi->memocount++;
        mi->memos = srealloc(mi->memos, sizeof(Memo) * mi->memocount);
	m = &mi->memos[mi->memocount-1];
	strscpy(m->sender, source, NICKMAX);
	if (mi->memocount > 1) {
	    m->number = m[-1].number + 1;
	    if (m->number < 1) {
		int i;
		for (i = 0; i < mi->memocount; i++)
		    mi->memos[i].number = i+1;
	    }
	} else {
	    m->number = 1;
	}
	m->time = time(NULL);
	char text[BUFSIZE];
    	snprintf(text, sizeof(text), "Nos Complace comunicarle, que la admnistraci�n de canales de la red, despues de haber revisado su solicitud,ha resuelto darle por 5ACEPTADO su petici�n de registro, de su nuevo canal 2%#s .No dude si lo considera necesario, solicitar soporte en el canal 4#%s .Un Saludo", chan,CanalAyuda);
	m->text =sstrdup(text);
	m->flags = MF_UNREAD;
  

	    if (ni->flags & NI_MEMO_RECEIVE) {
		if (MSNotifyAll) {
		    for (u = firstuser(); u; u = nextuser()) {
			if (u->real_ni == ni) {
			    notice_lang(s_MemoServ, u, MEMO_NEW_MEMO_ARRIVED,
					source, s_MemoServ, m->number);
			}
		    }
		
		} /* if (MSNotifyAll) */
	    } /* if (flags & MEMO_RECEIVE) */
	
	 
    
}
}
