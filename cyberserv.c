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

#define HASH(host)      (((host)[0]&31)<<5 | ((host)[1]&31))

static Clones *cloneslist[1024];
static int32 nclones = 0;
static Clones *findclones(const char *host);

static IlineInfo *iline = NULL;
static int16 nilines = 0;

static int iline_add(const char *host, const char *admin, const int limite,
  const char *motivo, const char *operwho, const time_t time_expiracion);


static void do_credits(User *u);
static void do_help(User *u);
static void do_cyberkill(User *u);
static void do_cybergline(User *u);
static void do_cyberunban(User *u);
static void do_clones(User *u);
static void do_clonesadmin(User *u);
static void do_iline(User *u);
static void do_killclones(User *u);

static Command cmds[] = {
    { "CREDITS",    do_credits,    NULL,  -1,                    -1,-1,-1,-1 },
    { "CREDITOS",   do_credits,    NULL,  -1,                    -1,-1,-1,-1 },        
    { "HELP",       do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "AYUDA",      do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "SHOWCOMMANDS",  do_help,    NULL,  -1,                    -1,-1,-1,-1 },
    { ":?",         do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "?",          do_help,       NULL,  -1,                    -1,-1,-1,-1 },
    { "KILL",       do_cyberkill,  NULL,  CYBER_HELP_KILL,       -1,-1,-1,-1 },
    { "GLINE",      do_cybergline, NULL,  CYBER_HELP_GLINE,      -1,-1,-1,-1 },
    { "UNBAN",      do_cyberunban, NULL,  CYBER_HELP_UNBAN,      -1,-1,-1,-1 },
    { "CLONES",     do_clones,     NULL,  CYBER_HELP_CLONES,     -1,-1,-1,-1 },
    { "CLONESLIST", do_clonesadmin, is_services_admin,  -1,
            CYBER_HELP_CLONES, OPER_HELP_SESSION,
            OPER_HELP_SESSION, OPER_HELP_SESSION }, 
    { "ILINE",      do_iline,     is_services_admin,
            OPER_HELP_EXCEPTION, -1,-1,-1, -1 },
    { "KILLCLONES", do_killclones, is_services_admin,
            OPER_HELP_KILLCLONES, -1,-1,-1, -1 },            
    { NULL }
};


/*************************************************************************/
/*************************************************************************/
/* Mostrar el total de la informacion de los ilines de cyber */

void listcybers(int count_only, const char *cyber)
{
/*************
    int count = 0;
    IlineInfo *iline;
    int i;
    
    if (count_only) {
        for (i = 0; i < 256; i++) {
            for (iline = ilinelists[i]; iline; iline = iline->next)
                count++;
        }
        printf("%d Ilines en la base de datos.\n", count);
    } else if (cyber) {
        printf("En construccion");
    }    
    *********/
}
    
/*************************************************************************/
/****************************** Statistics *******************************/
/*************************************************************************/

void get_clones_stats(long *nrec, long *memuse)
{
    Clones *clones;
    long mem;
    int i;
            
    mem = sizeof(Clones) * nclones;
    for (i = 0; i < 1024; i++) {
        for (clones = cloneslist[i]; clones; clones = clones->next) {
            mem += strlen(clones->host)+1;
        }
    }
                                          
    *nrec = nclones;
    *memuse = mem;
}
 
void get_iline_stats(long *nrec, long *memuse)
{
    long mem = 0;
    int i;
    
    mem = sizeof(IlineInfo) * nilines;
    for (i = 0; i < nilines; i++) {
        mem += strlen(iline[i].ip)+1;
//        mem += strlen(iline[i].admin)+1;
        mem += strlen(iline[i].motivo)+1;
    }
    *nrec = nilines;   
    *memuse = mem;
}            


/*************************************************************************/
/********************* Clones, Funciones Internas ************************/
/*************************************************************************/

static Clones *findclones(const char *host)
{
    Clones *clones;
    int i;
       
    if (!host)
        return NULL;
                   
    for (i = 0; i < 1024; i++) {
        for (clones = cloneslist[i]; clones; clones = clones->next) {
            if (stricmp(host, clones->host) == 0) {
                return clones;
            }
        }
    }

    return NULL;
}

int add_clones(const char *nick, const char *host)
{
    Clones *clones, **list;
    IlineInfo *iline;
    int limiteclones = 0;
            
    clones = findclones(host);    

    if (clones) {
        iline = iline_find_host(host);
        limiteclones = iline ? iline->limite : LimiteClones;    
        
        if (limiteclones != 0 && clones->numeroclones >= limiteclones) {            
            if (MensajeClones)
                notice(s_CyberServ, nick, MensajeClones, host);
            if (WebClones)
                notice(s_CyberServ, nick, WebClones);
                                   
#ifdef IRC_UNDERNET_P10
        /* Debido a que en UNDERNET_P10, para killear a un usuario
         * se necesita saber el numerico, he modificado la rutina
         * do_nick para ke pille los trios primero, y luego el control
         * de clones
         */         
            char motivo[NICKMAX*2+32];
            User *user;
         
            user = finduser(nick);
         
            if (u) {
                snprintf(motivo, sizeof(motivo), "En esta red solo se permiten"
                  " %d clones para ti ip (%s)", limiteclones, host);
                kill_user(s_CyberServ, u->numerico, motivo);             
            }    
            return 0;
          /* ke pasaria si no se encuentra el usuario.... */
                return 0; /* se supone ke no esta..*/    
#else                                   
        /* Para killear, no usamos el kill_user() porque el control de 
         * clones se realiza antes de ke pillemos los datos del usuario;                                                                        
         * usaremos el kill directamente
         */
                send_cmd(s_CyberServ, "KILL %s :%s (En esta red solo se permiten %d clones"
                    " para tu IP (%s))",  nick, s_CyberServ, limiteclones, host);
                return 0;         
#endif         
        } else {
            clones->numeroclones++;
            return 1;
        }
    }
    nclones++;
    clones = scalloc(sizeof(Clones), 1);
    clones->host = sstrdup(host);
    list = &cloneslist[HASH(clones->host)];
    clones->next = *list;
    if (*list)
        (*list)->prev = clones;
    *list = clones;
    clones->numeroclones = 1;
                            
    return 1;
}

void del_clones(const char *host)
{
    Clones *clones;
    
    if (debug >= 2)
        log("debug: del_session() called");
               
    clones = findclones(host);

    if (!clones) {
        canalopers(s_CyberServ,
          "4ATENCION: Intento de borrar clones no existentes: 12%s", host);
        log("Clones: Intento de borrar clones no existentes: %s", host);
        return;
    }  
    
    if (clones->numeroclones > 1) {
        clones->numeroclones--;
        return;
    }

    if (clones->prev)
        clones->prev->next = clones->next;
    else
        cloneslist[HASH(clones->host)] = clones->next;
    if (clones->next)
        clones->next->prev = clones->prev;
                
    if (debug >= 2)
        log("debug: del_session(): free session structure");                                                    
      
    free(clones->host);
    free(clones);
        
    nclones--;
            
    if (debug >= 2)
        log("debug: del_session() done");
} 
                        
/*************************************************************************/
/*************************************************************************/

/* CyberServ initialization. */

void iline_init(void)
{
}

/*************************************************************************/

/* Main CyberServ routine. */

void cyberserv(const char *source, char *buf)
{
    char *cmd, *s;
    User *u = finduser(source);
        
    if (!u) {
        log("%s: Registro del usuario %s no encontrado", s_CyberServ, source);
        privmsg(s_CyberServ, source,
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

/* Load/save data files. */

#define SAFE(x) do {                                    \
    if ((x) < 0) {                                      \
        if (!forceload)                                 \
            fatal("Error de Lectura en %s", IlineDBName);      \
        nilines = 1;                                     \
        break;                                          \
    }                                                   \
} while (0) 

void load_iline_dbase(void)
{
    dbFILE *f;
    int i, ver;
    int16 n;
    int16 tmp16;   
    int32 tmp32;
    char *s;
            
    if (!(f = open_db(s_CyberServ, IlineDBName, "r")))
        return;

    switch (ver = get_file_version(f)) {
    
      case 7:

        SAFE(read_int16(&n, f));
        nilines = n;
        iline = smalloc(sizeof(IlineInfo) * nilines);
        if (!nilines) {
            close_db(f);
            return;
        }
        for (i = 0; i < nilines; i++) {
            SAFE(read_string(&iline[i].ip, f));
            SAFE(read_string(&s, f));
            if (s)
                iline[i].admin = findnick(s);
            else
                iline[i].admin = NULL;
            SAFE(read_string(&iline[i].motivo, f));    
            SAFE(read_int16(&tmp16, f));
            iline[i].limite = tmp16;
            SAFE(read_buffer(iline[i].operwho, f));
            SAFE(read_int32(&tmp32, f));
            iline[i].time_concesion = tmp32;              
            SAFE(read_int32(&tmp32, f));
            iline[i].time_expiracion = tmp32;                                                                                
            SAFE(read_int16(&tmp16, f));
            iline[i].record_clones = tmp16;
            SAFE(read_int32(&tmp32, f));
            iline[i].time_record = tmp32;                                
            SAFE(read_int16(&tmp16, f));
            iline[i].estado = tmp16;
            iline[i].num = i;       
        }  /* for (i) */
        break;
        
      default:
        fatal("Version (%d) no soportada en %s", ver, IlineDBName);
        
    }  /* switch (version) */
            
    close_db(f);
    /* Chequeo para los ilines sin admin */
    for (i = 0; i < nilines; i++) {
        if (!(iline[i].admin)) {
            canalopers(s_CyberServ, "El iline 12%s no tiene admin. Borrando...", iline[i].ip);
            log("%s: Carga DB: Borrando iline %s por no tener admin",
                                   s_CyberServ, iline[i].ip);
//             iline_del(i);
            
        }
    }          
}

#undef SAFE

/*************************************************************************/

#define SAFE(x) do {                                            \
    if ((x) < 0) {                                              \
        restore_db(f);                                          \
        log_perror("Error de Escritura en %s", IlineDBName);    \
        if (time(NULL) - lastwarn > WarningTimeout) {           \
            canalopers(NULL, "Error de Escritura en %s: %s", IlineDBName,  \
                        strerror(errno));                       \
            lastwarn = time(NULL);                              \
        }                                                       \
        return;                                                 \
    }                                                           \
} while (0)
    
void save_iline_dbase(void)
{
    dbFILE *f;
    int i;
    static time_t lastwarn = 0;
              
    if (!(f = open_db(s_CyberServ, IlineDBName, "w")))
        return;
    SAFE(write_int16(nilines, f));    
    for (i = 0; i < nilines; i++) {
        SAFE(write_string(iline[i].ip, f));
        if (iline->admin)
            SAFE(write_string(iline[i].admin->nick, f));
        else
            SAFE(write_string(NULL, f));
        SAFE(write_string(iline[i].motivo, f));
        SAFE(write_buffer(iline[i].operwho, f));
        SAFE(write_int16(iline[i].limite, f));
        SAFE(write_int32(iline[i].time_concesion, f));
        SAFE(write_int32(iline[i].time_expiracion, f));
        SAFE(write_int16(iline[i].record_clones, f));
        SAFE(write_int32(iline[i].time_record, f));
        SAFE(write_int16(iline[i].estado, f));                           
    } /* for (i) */
               
    close_db(f);  
}
                        
#undef SAFE

/*************************************************************************/

/* Borrar todos los ilines cuando estan expirados. */

void expire_ilines()
{
    
    int i;
    time_t now = time(NULL);    
    
//    if (!IlineExpire)
//        return;    

    for (i = 0; i < nilines; i++) {
        if (iline[i].time_expiracion == 0 || iline[i].time_expiracion > now) 
            continue;
        log("Expirando Iline a ip %s, admin %s", iline[i].ip, iline[i].admin->nick);
         canalopers(s_CyberServ, "Expirando iline %s, admin %s",
                    iline[i].ip, iline[i].admin->nick);

        free(iline[i].ip);
        free(iline[i].admin);
        free(iline[i].motivo);
        nilines--;
        memmove(iline+i, iline+i+1,
                 sizeof(IlineInfo) * (nilines-i));
        iline = srealloc(iline,
                 sizeof(IlineInfo) * nilines);
        i--;
   }
}                                

/*************************************************************************/

/* Borrar Nick (borrado o expirado) de los ilines. */

void iline_remove_nick(const NickInfo *ni)
{
    int i;
    
    for (i = 0; i < nilines; i++) {    
        if (iline[i].admin == ni) {
            canalopers(s_CyberServ, "Borrando iline %s propiedad del"
                           " nick %s", iline[i].ip, iline[i].admin->nick);
            log("%s: Borrando Iline %s propiedad del nick borrado %s",
                               s_CyberServ, iline[i].ip, iline[i].admin->nick);
//            iline_del(i);
                        
        }
    }    
}        

/*************************************************************************/

/* Buscar si un host tiene concedido clones, devuelve el registro si tiene
 * concesion de clones, y devuelve NULL si no tiene 
 * Zoltan - 27/09/2000 
 */
 
IlineInfo *iline_find_host(const char *host)
{
    int i;

    for (i = 0; i < nilines; i++) {     
        if (match_wild_nocase(iline[i].ip, host)) {
            return &iline[i];
         }
    }    
    return NULL;
}

/*************************************************************************/

/* Lo mismo ke el anterior, pero por el nick del admin, en vez del host */

IlineInfo *iline_find_admin(const char *nick)
{
    int i;
    
    for (i = 0; i < nilines; i++) {    
         if (stricmp(iline[i].admin->nick, nick) == 0)
             return &iline[i];
    }
    return NULL;
}

/*************************************************************************/

/* Es un admin de cyber ??? */

int is_cyber_admin(User *u)
{
    IlineInfo *iline;
    
    /* Doy acceso cyber a los admins **/
    if (is_services_admin(u))
        return 1;
        
    iline = iline_find_admin(u->nick);
    
    if (iline)
        if (stricmp(iline->admin->nick, u->nick) == 0)
            if (nick_identified(u))
                return 1;       
            return 0;
        return 0;
               
    return 1;
}        

/*************************************************************************/

/* Añadir un iline a la DB. */

static int iline_add(const char *host, const char *admin, const int limite,
  const char *motivo, const char *operwho, const time_t time_expiracion)
{
    int i;
  
    /* Check if an exception already exists for this mask */
    for (i = 0; i < nilines; i++)
        if (stricmp(host, iline[i].ip) == 0)
            return 0;
                          
    nilines++;
    iline = srealloc(iline,
                     sizeof(IlineInfo) * nilines);  
                   
    iline[nilines-1].ip = sstrdup(host);
    iline[nilines-1].admin = findnick(admin);
    iline[nilines-1].limite = limite;
    iline[nilines-1].motivo = sstrdup(motivo);
    iline[nilines-1].time_concesion = time(NULL);
    strscpy(iline[nilines-1].operwho, operwho, NICKMAX);
    iline[nilines-1].time_expiracion = time_expiracion;
    iline[nilines-1].record_clones = 0;
    iline[nilines-1].time_record = time(NULL);    
    iline[nilines-1].num = nilines-1;                   
    
    return 1;
}

/*************************************************************************/

/* Borrar un iline de la DB. */
static int iline_del(const int index)
{    
    if (index < 0 || index >= nilines)
        return 0;
            
    free(iline[index].ip);
    free(iline[index].admin);
    free(iline[index].motivo);
    nilines--;
    memmove(iline+index, iline+index+1,
             sizeof(IlineInfo) * (nilines-index));
    iline = srealloc(iline,
             sizeof(IlineInfo) * nilines);
                                                                
    return 1;
}

/*************************************************************************/
static int iline_del_callback(User *u, int num, va_list args)
{
    int i;
    int *last = va_arg(args, int *);
        
    *last = num;
    for (i = 0; i < nilines; i++) {
        if (num-1 == iline[i].num)
            break;
    }
    if (i < nilines)
        return iline_del(i);
    else
        return 0;
}    

/*************************************************************************/
static int iline_list(User *u, const int index, int *sent_header)
{
    if (index < 0 || index >= nilines)
        return 0;
    if (!*sent_header) {
        notice_lang(s_CyberServ, u, OPER_EXCEPTION_LIST_HEADER);
        notice_lang(s_CyberServ, u, OPER_EXCEPTION_LIST_COLHEAD);
        *sent_header = 1;
    }
    notice_lang(s_CyberServ, u, OPER_EXCEPTION_LIST_FORMAT, index+1,
           iline[index].limite, iline[index].ip);
    return 1;
}    

/*************************************************************************/
static int iline_list_callback(User *u, int num, va_list args)
{
    int *sent_header = va_arg(args, int *);
    
    return iline_list(u, num-1, sent_header);
}
        
/*************************************************************************/
static int iline_view(User *u, const int index, int *sent_header)
{
    char timebuf[32], expirebuf[256];
    struct tm tm;
    time_t t = time(NULL);
            
    if (index < 0 || index >= nilines)
        return 0;
    if (!*sent_header) {
        notice_lang(s_CyberServ, u, OPER_EXCEPTION_LIST_HEADER);
        *sent_header = 1;
    }

    tm = *localtime(iline[index].time_concesion ? &iline[index].time_concesion : &t);
    strftime_lang(timebuf, sizeof(timebuf), u, STRFTIME_SHORT_DATE_FORMAT, &tm);
    if (iline[index].time_expiracion == 0) {
        snprintf(expirebuf, sizeof(expirebuf),
                        getstring(u->ni, OPER_AKILL_NO_EXPIRE));
    } else if (iline[index].time_expiracion <= t) {
        snprintf(expirebuf, sizeof(expirebuf),
                          getstring(u->ni, OPER_AKILL_EXPIRES_SOON));
    } else {        
        time_t t2 = iline[index].time_expiracion - t;
        t2 += 59;
        if (t2 < 3600) {
            t2 /= 60;
            if (t2 == 1)              
                snprintf(expirebuf, sizeof(expirebuf),
                             getstring(u->ni, OPER_AKILL_EXPIRES_1M), t2);
            else
                snprintf(expirebuf, sizeof(expirebuf),
                             getstring(u->ni, OPER_AKILL_EXPIRES_M), t2);
        } else if (t2 < 86400) {
            t2 /= 60;
            if (t2/60 == 1) {
                if (t2%60 == 1)
                    snprintf(expirebuf, sizeof(expirebuf),
                            getstring(u->ni, OPER_AKILL_EXPIRES_1H1M),
                            t2/60, t2%60);
                else
                    snprintf(expirebuf, sizeof(expirebuf),
                            getstring(u->ni, OPER_AKILL_EXPIRES_1HM),
                            t2/60, t2%60);
            } else {
                if (t2%60 == 1)
                    snprintf(expirebuf, sizeof(expirebuf),
                            getstring(u->ni, OPER_AKILL_EXPIRES_H1M),
                            t2/60, t2%60);                            
                else
                    snprintf(expirebuf, sizeof(expirebuf),
                            getstring(u->ni, OPER_AKILL_EXPIRES_HM),
                            t2/60, t2%60);
            }                            
        } else {
            t2 /= 86400;
            if (t2 == 1)
                snprintf(expirebuf, sizeof(expirebuf),
                              getstring(u->ni, OPER_AKILL_EXPIRES_1D), t2);
            else
                snprintf(expirebuf, sizeof(expirebuf),
                              getstring(u->ni, OPER_AKILL_EXPIRES_D), t2);
        }
                                                                                                                            
    }

    notice_lang(s_CyberServ, u, OPER_EXCEPTION_VIEW_FORMAT,
                 index+1, iline[index].ip,
               *iline[index].operwho ? iline[index].operwho : "<desconocido>",
                timebuf, expirebuf, iline[index].limite,
                iline[index].motivo);
    return 1;
}

/*************************************************************************/
static int iline_view_callback(User *u, int num, va_list args)
{
    int *sent_header = va_arg(args, int *);
    
    return iline_view(u, num-1, sent_header);
}
        
/*************************************************************************/
/*********************** CyberServ command routines **********************/
/*************************************************************************/

static void do_credits(User *u)
{
    notice_lang(s_CyberServ, u, SERVICES_CREDITS);
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
#ifdef IRC_UNDERNET_P10
       kill_user(s_CyberServ, u2->numerico, motivo);
#else
       kill_user(s_CyberServ, u2->nick, motivo); 
#endif       
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
       snprintf(buf, sizeof(buf), "%s -> G-Lined: %s", s_CyberServ, motivo);
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
#ifdef IRC_UNDERNET_P10
       kill_user(s_CyberServ, u2->numerico, motivo);
#else
       kill_user(s_CyberServ, u2->nick, motivo);
#endif       
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

/*************************************************************************/
static void do_clones(User *u)
{

    Clones *clones;
    IlineInfo *iline;
    
    if (!ControlClones) {
        notice_lang(s_CyberServ, u, OPER_SESSION_DISABLED);
        return;
    }

    clones = findclones(u->host);
    iline = iline_find_host(u->host);
    
    if (!iline) 
        notice_lang(s_CyberServ, u, CYBER_CLONES_NOT_IS_CYBER);
    else
        notice_lang(s_CyberServ, u, CYBER_CLONES_IS_CYBER, u->host,
                          clones->numeroclones, iline->limite);
}    

/*************************************************************************/
static void do_clonesadmin(User *u)
{
    char *cmd = strtok(NULL, " ");
    char *param = strtok(NULL, " ");

    Clones *clones;
    IlineInfo *iline;

    int mincount;
    int i;
    
    if (!ControlClones) {
        notice_lang(s_CyberServ, u, OPER_SESSION_DISABLED);
        return;
    }
                            
    if (!cmd) {
        syntax_error(s_CyberServ, u, "CLONESLIST", OPER_SESSION_SYNTAX);     
        return;
    }
    
    if (stricmp(cmd, "LIST") == 0) {
        if (!param) {
            syntax_error(s_CyberServ, u, "CLONES", OPER_SESSION_LIST_SYNTAX);
        } else if ((mincount = atoi(param)) <= 1) {
            notice_lang(s_CyberServ, u, OPER_SESSION_INVALID_THRESHOLD);                        
        } else {
            notice_lang(s_CyberServ, u, OPER_SESSION_LIST_HEADER, mincount);
            notice_lang(s_CyberServ, u, OPER_SESSION_LIST_COLHEAD);
            for (i = 0; i < 1024; i++) {
                for (clones = cloneslist[i]; clones; clones=clones->next) {
                    if (clones->numeroclones >= mincount)
                        notice_lang(s_CyberServ, u, OPER_SESSION_LIST_FORMAT,
                                     clones->numeroclones, clones->host);
                }
            }
        }                                                                  
    } else if (stricmp(cmd, "VIEW") == 0) {                   
        if (!param) {
            syntax_error(s_CyberServ, u, "CLONESLIST", OPER_SESSION_VIEW_SYNTAX);
                    
        } else {
            clones = findclones(param);
            if (!clones) {   
                notice_lang(s_CyberServ, u, OPER_SESSION_NOT_FOUND, param);
            } else {
                iline = iline_find_host(param);
                              
              /* If limit == 0, then there is no limit - reply must include
               * this information. e.g. "... with no limit.".
               */
                         
                 notice_lang(s_CyberServ, u, OPER_SESSION_VIEW_FORMAT,
                        param, clones->numeroclones,
                          iline ? iline->limite : LimiteClones);
            }
        }            
    }    
}                        
            
/*************************************************************************/
static void do_iline(User *u)
{
    char *comando = strtok(NULL, " ");
    char *host, *admin, *expiracion, *motivo, *limite;
    int limit, expires;
    int i = 0;    
    
    NickInfo *ni;
    IlineInfo *iline = NULL;
    IlineInfo *host2, *admin2;
    
    if (!ControlClones) {
        notice_lang(s_CyberServ, u, OPER_EXCEPTION_DISABLED);
        return;
    }    
    if (!comando)
        comando = "";    
/*        
    if (!comando) {
        syntax_error(s_CyberServ, u, "ILINE", OPER_EXCEPTION_SYNTAX);
        return;    
    } */
            
    if (stricmp(comando, "ADD") == 0) {
        if (nilines >= 32767) {
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_TOO_MANY);
            return;
        }   
        host = strtok(NULL, " ");
        admin  = strtok(NULL, " ");
        limite = strtok(NULL, " ");                
        expiracion = strtok(NULL, " ");
        motivo = strtok(NULL, "");

        if (!motivo) {
//            syntax_error(s_CyberServ, u, "ILINE", OPER_EXCEPTION_ADD_SYNTAX);
            privmsg(s_CyberServ, u->nick, "ILINE ADD host admin limite expiracion motivo");
            return;
        }                                          

        if (strchr(host, '!') || strchr(host, '@')) {
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_INVALID_HOSTMASK);
            return;
        } else {
            strlower(host); 
        }    

        ni = findnick(admin);
        if (!ni) {
            notice_lang(s_CyberServ, u, NICK_X_NOT_REGISTERED, admin);
            return;
        }    
        admin2 = iline_find_admin(admin);
        host2 = iline_find_host(host);
        if (!host2) {
            if (admin2) {
                privmsg(s_CyberServ, u->nick, "El nick %s ya tiene concedido iline", ni->nick);    
                return;
            }    
        }    

        limit = (limite && isdigit(*limite)) ? atoi(limite) : -1;        
        if (limit < 0 || limit > MaximoClones) {
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_INVALID_LIMIT,
                                   MaximoClones);
            return;    
        }    
 
        expires = expiracion ? dotime(expiracion) : ExpIlineDefault;
        if (expires < 0) {
            notice_lang(s_CyberServ, u, BAD_EXPIRY_TIME);
            return;
        } else if (expires > 0) {
            expires += time(NULL);
        }        
                    
        iline = iline_find_host(host);
        if (iline_add(host, admin, limit, motivo, u->nick, expires))
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_ADDED, host, limite);
        else
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_ALREADY_PRESENT,
                                                  host, limite);
        if (readonly)
            notice_lang(s_CyberServ, u, READ_ONLY_MODE);
        return;                   
    } else if (stricmp(comando, "DEL") == 0) {
        host = strtok(NULL, " ");

        if (!host) {
            syntax_error(s_CyberServ, u, "EXCEPTION", OPER_EXCEPTION_DEL_SYNTAX);
        }
                   
        if (isdigit(*host) && strspn(host, "1234567890,-") == strlen(host)) {
            int count, deleted, last = -1;
            deleted = process_numlist(host, &count, iline_del_callback, u,
                                                  &last);            
            if (!deleted) {
                if (count == 1) {
                    notice_lang(s_CyberServ, u, OPER_EXCEPTION_NO_SUCH_ENTRY,
                                    last);
                } else {
                    notice_lang(s_CyberServ, u, OPER_EXCEPTION_NO_MATCH);
                }                    
            } else if (deleted == 1) {
                notice_lang(s_CyberServ, u, OPER_EXCEPTION_DELETED_ONE);
            } else {
                notice_lang(s_CyberServ, u, OPER_EXCEPTION_DELETED_SEVERAL,
                        deleted);
            }                
        } else {
            for (i = 0; i < nilines; i++) {
                if (stricmp(host, iline[i].ip) == 0) {
                    iline_del(i);
                    notice_lang(s_CyberServ, u, OPER_EXCEPTION_DELETED, host);
                    break;
                }
            }    
            if (i == nilines)
                notice_lang(s_CyberServ, u, OPER_EXCEPTION_NOT_FOUND, host);
        }                    
        for (i = 0; i < nilines; i++)
            iline[i].num = i;
               
        if (readonly)
            notice_lang(s_CyberServ, u, READ_ONLY_MODE);        

    } else if (stricmp(comando, "MOVE") == 0) {
        IlineInfo *iline;
        char *n1str = strtok(NULL, " ");        /* From position */
        char *n2str = strtok(NULL, " ");        /* To position */
        int n1, n2;
                                    
        if (!n2str) {            
            syntax_error(s_CyberServ, u, "ILINE", OPER_EXCEPTION_MOVE_SYNTAX);
            return;
        }
                                                                                
        n1 = atoi(n1str) - 1;
        n2 = atoi(n2str) - 1;        

        if ((n1 >= 0 || n2 < nilines) && n1 != n2) {
            iline = scalloc(sizeof(IlineInfo), 1);
            memcpy(iline, &iline[n1], sizeof(IlineInfo));

        if (n1 < n2) {
            /* Shift upwards */
            memmove(&iline[n1], &iline[n1+1],
                            sizeof(IlineInfo) * (n2-n1));
            memmove(&iline[n2], iline, sizeof(IlineInfo));
        } else {
           /* Shift downwards */
            memmove(&iline[n2+1], &iline[n2],
                            sizeof(IlineInfo) * (n1-n2));
            memmove(&iline[n2], iline, sizeof(IlineInfo));
        }                 
            free(iline);
            
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_MOVED,
                          iline[n1].ip, n1+1, n2+1);
                                                
            /* Renumber the exception list. See the DEL block above for why. */
            for (i = 0; i < nilines; i++)
                iline[i].num = i;
                                    
            if (readonly)
                notice_lang(s_CyberServ, u, READ_ONLY_MODE);                                                                               
        } else {
            syntax_error(s_CyberServ, u, "ILINE", OPER_EXCEPTION_MOVE_SYNTAX);
        }                

    } else if (stricmp(comando, "LIST") == 0) {
        int sent_header = 0;
        expire_ilines();
        host = strtok(NULL, " ");
        if (host)
            strlower(host);        
        if (host && strspn(host, "1234567890,-") == strlen(host)) {
            process_numlist(host, NULL, iline_list_callback, u, &sent_header);
        } else {
            for (i = 0; i < nilines; i++) {
                if (!host || match_wild(host, iline[i].ip))
                    iline_list(u, i, &sent_header);
            }
        }        
        if (!sent_header)
             notice_lang(s_CyberServ, u, OPER_EXCEPTION_NO_MATCH);
                    
    } else if (stricmp(comando, "VIEW") == 0) {
        int sent_header = 0;
        expire_ilines();
        host = strtok(NULL, " ");
        if (host)
            strlower(host);                            

        if (host && strspn(host, "1234567890,-") == strlen(host)) {
            process_numlist(host, NULL, iline_view_callback, u, &sent_header);
        } else {
            for (i = 0; i < nilines; i++) {
                if (!host || match_wild(host, iline[i].ip))
                    iline_view(u, i, &sent_header);
            }
        }            
        if (!sent_header)
            notice_lang(s_CyberServ, u, OPER_EXCEPTION_NO_MATCH);
                             
    } else {
        syntax_error(s_CyberServ, u, "ILINE", OPER_EXCEPTION_SYNTAX);    
    }   
}
/*************************************************************************/

/* Kill all users matching a certain host. The host is obtained from the
 * supplied nick. The raw hostmsk is not supplied with the command in an effort
 * to prevent abuse and mistakes from being made - which might cause *.com to
 * be killed. It also makes it very quick and simple to use - which is usually
 * what you want when someone starts loading numerous clones. In addition to
 * killing the clones, we add a temporary AKILL to prevent them from
 * immediately reconnecting.
 * Syntax: KILLCLONES nick
 * -TheShadow (29 Mar 1999)
 */
 
static void do_killclones(User *u)
{
    char *clonenick = strtok(NULL, " ");
    int count=0;
    User *cloneuser, *user, *tempuser;
    char *clonemask, *akillmask;
    char killreason[NICKMAX+32];
    char akillreason[] = "GLINE temporal por CLONES."; 
    
    if (!clonenick) {
        notice_lang(s_CyberServ, u, OPER_KILLCLONES_SYNTAX);
            
    } else if (!(cloneuser = finduser(clonenick))) {
        notice_lang(s_CyberServ, u, OPER_KILLCLONES_UNKNOWN_NICK, clonenick);
                        
    } else {    
        clonemask = smalloc(strlen(cloneuser->host) + 5);
        sprintf(clonemask, "*!*@%s", cloneuser->host);
                
        akillmask = smalloc(strlen(cloneuser->host) + 3);
        sprintf(akillmask, "*@%s", strlower(cloneuser->host));    

        user = firstuser();
        while (user) {        
            if (match_usermask(clonemask, user) != 0) {
                tempuser = nextuser();
                count++;
                snprintf(killreason, sizeof(killreason), "Cloning [%d]", count);
#ifdef IRC_UNDERNET_P10
                kill_user(NULL, user->numerico, killreason);
#else
                kill_user(NULL, user->nick, killreason);
#endif                
                user = tempuser;
            } else {
                user = nextuser();
            }
        }        
        add_akill(akillmask, akillreason, u->nick,
                                time(NULL) + KillClonesAkillExpire);

        canalopers(s_CyberServ, "12%s ha usado KILLCLONES en %s killeando "
                           "12%d clones. Un AKILL temporal has sido añadido "
                           "en 12%s.", u->nick, clonemask, count, akillmask);
                                                                                                
        log("%s: KILLCLONES: %d clone(s) matching %s killed.",
                              s_CyberServ, count, clonemask);
                                
        free(akillmask);
        free(clonemask);
    }
}
                                                                                            