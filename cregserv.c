/* Funciones CregServ.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * Modulo CregServ hecho por zoltan <zolty@ctv.es>
 *
 * Basado en CReG de iRC-Hispano <irc.irc-hispano.org>
 */
 
/*************************************************************************/
 
#include "services.h"
#include "pseudo.h"
 
/*************************************************************************/

#ifdef CREGSERV

static CregInfo *creglists[256]; 


static void do_credits(User *u);
static void do_help(User *u);
static void do_registra(User *u);
static void do_cancela(User *u);
static void do_apoya(User *u);
static void do_apolla(User *u);
static void do_estado(User *u);
static void do_apoyos(User *u);
static void do_lista(User *u);
static void do_info(User *u);
static void do_suspend(User *u);
static void do_unsuspend(User *u);
static void do_marca(User *u);
static void do_ignore(User *u);
static void do_anula(User *u);
static void do_acepta(User *u);
static void do_restaura(User *u);
static void do_regcanal(User *u);
static void do_rechaza(User *u);
static void do_deniega(User *u);
static void do_drop(User *u);
static void do_prohibe(User *u);
static void do_permite(User *u);
static void do_activa(User *u);
static void do_desactiva(User *u);


static Command cmds[] = {
    { "CREDITS",  do_credits,    NULL,  -1,                      -1,-1,-1,-1 },
    { "CREDITOS", do_credits,    NULL,  -1,                      -1,-1,-1,-1 },        
    { "HELP",       do_help,     NULL,  -1,                      -1,-1,-1,-1 },
    { "AYUDA",      do_help,     NULL,  -1,                      -1,-1,-1,-1 },    
    { "SHOWCOMMANDS",  do_help,  NULL,  -1,                      -1,-1,-1,-1 },
    { ":?",         do_help,     NULL,  -1,                      -1,-1,-1,-1 },
    { "?",          do_help,     NULL,  -1,                      -1,-1,-1,-1 },            
    { "REGISTRA",   do_registra, NULL,  CREG_HELP_REGISTRA,      -1,-1,-1,-1 },
    { "CANCELA",    do_cancela,  NULL,  CREG_HELP_CANCELA,       -1,-1,-1,-1 },
    { "APOYA",      do_apoya,    NULL,  CREG_HELP_APOYA,         -1,-1,-1,-1 },
    { "APOLLA",     do_apolla,   NULL,  -1,                      -1,-1,-1,-1 },       
    { "ESTADO",     do_estado,   NULL,  CREG_HELP_ESTADO,        -1,-1,-1,-1 },
    { "APOYOS",     do_apoyos,   NULL,  CREG_HELP_APOYOS,        -1,-1,-1,-1 },        
    { "LISTA",      do_lista,    is_services_oper, -1            -1,-1,-1,-1 }, 
    { "INFO",       do_info,     is_services_oper, -1            -1,-1,-1,-1 },
    { "SUSPEND",    do_suspend,  is_services_oper, -1            -1,-1,-1,-1 },
    { "UNSUSPEND",  do_unsuspend, is_services_oper, -1           -1,-1,-1,-1 },
    { "MARCA",      do_marca,    is_services_oper, -1            -1,-1,-1,-1 },
    { "IGNORE",     do_ignore,   is_services_oper, -1            -1,-1,-1,-1 },
    { "ANULA",      do_anula,    is_services_oper, -1            -1,-1,-1,-1 },
    { "ACEPTA",     do_acepta,   is_services_admin, -1           -1,-1,-1,-1 },
    { "RESTAURA",   do_restaura, is_services_admin, -1           -1,-1,-1,-1 },
    { "REGCANAL",   do_regcanal, is_services_admin, -1           -1,-1,-1,-1 },
    { "RECHAZA",    do_rechaza,  is_services_admin, -1           -1,-1,-1,-1 },
    { "DENIEGA",    do_deniega,  is_services_admin, -1           -1,-1,-1,-1 },
    { "DROP",       do_drop,     is_services_admin, -1           -1,-1,-1,-1 },
    { "PROHIBE",    do_prohibe,  is_services_admin, -1           -1,-1,-1,-1 },
    { "PERMITE",    do_permite,  is_services_admin, -1           -1,-1,-1,-1 },
    { "ACTIVA",     do_activa,   is_services_admin, -1           -1,-1,-1,-1 },
    { "DESACTIVA",  do_desactiva, is_services_admin, -1          -1,-1,-1,-1 },
                                                                
    { NULL }
};

/*************************************************************************/
/*************************************************************************/

/* Mostrar el total de la informacion de creg... bla bla bla */
 
void listcregs(int count_only, const char *creg)
{
    int count = 0;
    CregInfo *cr;
    int i;
                
    if (count_only) {
    
        for (i = 0; i < 256; i++) {
            for (cr = creglists[i]; cr; cr = cr->next)
                count++;
        }
        printf("%d Canales en la base de datos.\n", count);

    } else if (creg) {                                                        
        printf("En construccion");
        
    }
}

/*************************************************************************/

/* Return information on memory use.  Assumes pointers are valid. */

void get_cregserv_stats(long *nrec, long *memuse)
{
    long count = 0, mem = 0;
    int i;
    CregInfo *cr;
                        
    for (i = 0; i < 256; i++) {
        for (cr = creglists[i]; cr; cr = cr->next) {
            count++;
            mem += sizeof(*cr);
            if (cr->nickoper)
                mem += strlen(cr->nickoper)+1;
            if (cr->motivo)
                mem += strlen(cr->motivo)+1;
            mem += cr->apoyoscount * sizeof(ApoyosCreg);
            mem += cr->historycount * sizeof(HistoryCreg);
                                  
        }
    }
    *nrec = count;
    *memuse = mem;
}            
/*************************************************************************/
/*************************************************************************/

/* ChanServ initialization. */

void cr_init(void)
{
    Command *cmd;
    
    cmd = lookup_cmd(cmds, "APOYA");
    if (cmd)
         cmd->help_param1 = s_ChanServ;
}                                                                                                                                            

/*************************************************************************/

/* Main CregServ routine. */

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
            notice_lang(s_CregServ, u, NICK_NOT_REGISTERED_HELP, s_NickServ);
        else
            run_cmd(s_CregServ, u, cmds, cmd);
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

      case 7:
      case 6:
      case 5:
   
        for (i = 0; i < 256 && !failed; i++) {
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
/*                
                SAFE(read_string(&cr->nickoper, f));
                if (!cr->nickoper)
                    cr->nickoper =""
                SAFE(read_string(&cr->motivo, f));                    
                SAFE(read_int32(&tmp32, f));
                cr->time_motivo = tmp32;                                                                                                
                                */
                SAFE(read_int32(&cr->estado, f));                                                
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
                SAFE(read_int16(&cr->historycount, f));
                if (cr->historycount) {
                    cr->history = scalloc(cr->historycount, sizeof(HistoryCreg));
                    for (j = 0; j < cr->historycount; j++) {
                        SAFE(read_string(&cr->history[j].nickoper, f));
                        SAFE(read_string(&cr->history[j].marca, f));
                        SAFE(read_int32(&tmp32, f));
                        cr->history[j].time_marca = tmp32;
                    }
                } else {
                    cr->history = NULL;
                } 
            }  /* while (getc_db(f) != 0) */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
             *last = NULL;

        }  /* for (i) */ 
        break;                                                                                                   
     default:
        fatal("Unsupported version number (%d) on %s", ver, CregDBName);

    }  /* switch (version) */
    
    close_db(f);

/********* Poner aqui una funcion para hacer volcados de db :) ******/                      

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
           
    for (i = 0; i < 256; i++) {
                                            
        for (cr = creglists[i]; cr; cr = cr->next) {
            SAFE(write_int8(1, f));
            SAFE(write_buffer(cr->name, f));
            SAFE(write_string(cr->founder, f));
                                
            SAFE(write_buffer(cr->founderpass, f));
            SAFE(write_string(cr->desc, f));
            SAFE(write_string(cr->email, f));
                                                                                                    
            SAFE(write_int32(cr->time_peticion, f));
/*            SAFE(write_string(cr->nickoper, f));
            SAFE(write_string(cr->motivo, f));
            SAFE(write_int32(cr->time_motivo, f));
 */
            SAFE(write_int32(cr->estado, f));                                                                                                                
            SAFE(write_int16(cr->apoyoscount, f));
            for (j = 0; j < cr->apoyoscount; j++) {            
                SAFE(write_string(cr->apoyos[j].nickapoyo, f));
                SAFE(write_string(cr->apoyos[j].emailapoyo, f));
                SAFE(write_int32(cr->apoyos[j].time_apoyo, f));
            }                     
            SAFE(write_int16(cr->historycount, f));
            for (j = 0; j < cr->historycount; j++) {
                SAFE(write_string(cr->history[j].nickoper, f));
                SAFE(write_string(cr->history[j].marca, f));
                SAFE(write_int32(cr->history[j].time_marca, f));                                                   
            }                                                                                                                           
        } /* for (creglists[i]) */
        
        SAFE(write_int8(0, f));
                
    } /* for (i) */
                    
    close_db(f);
}

#undef SAFE

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
/*********************** CregServ private routines ***********************/
/*************************************************************************/

/* Insert a channel alphabetically into the database. */

static void alpha_insert_creg(CregInfo *cr)
{
    CregInfo *ptr, *prev;
    char *chan = cr->name;
        
    for (prev = NULL, ptr = creglists[tolower(chan[1])];
                        ptr != NULL && stricmp(ptr->name, chan) < 0;
                        prev = ptr, ptr = ptr->next)
        ;
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
    strscpy(cr->name, chan, CHANMAX);
    alpha_insert_creg(cr);
    return cr;
}

/*************************************************************************/

/* Remove a channel from the ChanServ database.  Return 1 on success, 0
 * otherwise. */
/*                                                                                                                                                                            
static int delcreg(CregInfo *cr)
{
}            
  */  
/*************************************************************************/

static void do_credits(User *u)
{
    notice_lang(s_CregServ, u, SERVICES_CREDITS);
}
                       
/*************************************************************************/
                       
                                                                                                                                                                    
/* Return a help message. */

static void do_help(User *u)
{
    char *cmd = strtok(NULL, "");
    
    if (!cmd) {
        notice_help(s_CregServ, u, CREG_HELP);
        
  /*      if (is_services_oper(u))
        notice_help(s_CregServ, u, CREG_SERVADMIN_HELP); */
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
    CregInfo *cr, *cr2;

    char passtemp[255];
//    cr = cr_findcreg(chan);

    if (readonly) {
        notice_lang(s_CregServ, u, CHAN_REGISTER_DISABLED);
        return;
    }
                        
    if (!desc) {
/*         syntax_error(s_CregServ, u, "REGISTRA", CREG_REGISTER_SYNTAX); */
        privmsg(s_CregServ, u->nick, "REGISTRA <canal> <password> <descripcion>");
    } else if (!ni) {
        notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
        notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK,
                     s_NickServ, s_NickServ);                                                    
    } else if (*chan == '&') {
        notice_lang(s_CregServ, u, CHAN_REGISTER_NOT_LOCAL);
    } else if (!(*chan == '#')) {
        privmsg(s_CregServ, u->nick, "Canal no valido para registrar");                       
    } else if ((cr2 = cr_findcreg(chan))) { 
        privmsg(s_CregServ, u->nick, "El canal esta en proceso de reg");     
    } else if (!(cr = makecreg(chan))) {
        log("%s: makecreg() failed for REGISTER %s", s_CregServ, chan);
        notice_lang(s_CregServ, u, CHAN_REGISTRATION_FAILED);                             
    } else { 
        cr = makecreg(chan);
        
        cr->founder =  u->nick; 
        if (strlen(pass) > PASSMAX-1) /* -1 for null byte */
            notice_lang(s_CregServ, u, PASSWORD_TRUNCATED, PASSMAX-1);
        strscpy(cr->founderpass, pass, PASSMAX);
                                    
        cr->email = ni->email;
        cr->desc = sstrdup(desc);

        cr->time_peticion = time(NULL);
        cr->estado = CR_PROCESO_REG;

        srand(time(NULL));
        sprintf(passtemp, "%05u",1+(int)(rand()%99999) );
        strscpy(cr->passapoyo, passtemp, PASSMAX);

        cr->time_lastapoyo = time(NULL);

        privmsg(s_CregServ, u->nick, "Canal 12%s aceptado para el proceso de registro", chan);
        canalopers(s_CregServ, "12%s ha solicitado el registro del canal 12%s", u->nick, chan);

    } 
}                                                                        
                                                        


/*************************************************************************/

static void do_cancela(User *u)
{
    char *chan = strtok(NULL, " ");

    NickInfo *ni = u->ni;
    
         
    if (!chan) {
/*         syntax_error(s_CregServ, u, "CANCELA", CHAN_REGISTER_SYNTAX); */
       privmsg(s_CregServ, u->nick, "CANCELA <canal>");
    } else if (!ni) {
         notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
         notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK,
                              s_NickServ, s_NickServ);
    } else                                            
                                               
        privmsg(s_CregServ, u->nick, "Comando deshabilitado");
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
/*       syntax_error(s_CregServ, u, "APOYA", CHAN_REGISTER_SYNTAX); */
       privmsg(s_CregServ, u->nick, "APOYA <canal>");
    } else if (!ni) {
        notice_lang(s_CregServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_identified(u)) {
        notice_lang(s_CregServ, u, CHAN_MUST_IDENTIFY_NICK,
                       s_NickServ, s_NickServ);
    } else if (!(cr = cr_findcreg(chan))) {
        privmsg(s_CregServ, u->nick, "El canal 12%s no esta en proceso de registro", chan);
    } else if (!(cr->estado & CR_PROCESO_REG)) {                                           
        privmsg(s_CregServ, u->nick, "El canal 12%s no esta en proceso de registro1", chan); 
    } else {
/* Implementar control de tiempo! */

        if (!cr->passapoyo) {
            srand(time(NULL));
            sprintf(passtemp, "%05u",1+(int)(rand()%99999) );
            strscpy(cr->passapoyo, passtemp, PASSMAX);
        }    
                                  
        if (stricmp(u->nick, cr->founder) == 0) {
            privmsg(s_CregServ, u->nick, "Ya apoyaste como founder");
            return;
        }

        for (apoyos = cr->apoyos, i = 0; i < cr->apoyoscount; apoyos++, i++) {
            if (stricmp(u->nick, apoyos->nickapoyo) == 0) {
                privmsg(s_CregServ, u->nick, "Ya has apoyado al canal");
                return;
            }
        }
        if (!pass) { 
 privmsg(s_CregServ, u->nick, "No apoye el canal solamente porque le hayan dicho que"); 
 privmsg(s_CregServ, u->nick, "escriba 12/msg %s APOYA %s. Infórmese sobre la temática", s_CregServ, chan); 
 privmsg(s_CregServ, u->nick, "del canal, sus usuarios, etc.");
 privmsg(s_CregServ, u->nick, "Si decide finalmente apoyar al canal 12%s hágalo escribiendo:", chan);
 privmsg(s_CregServ, u->nick, "12/msg %s APOYA %s %s", s_CregServ, chan, cr->passapoyo); 
           return;
        } 
        if (stricmp(pass, cr->passapoyo) != 0) {
            privmsg(s_CregServ, u->nick, "Clave inválida");
            privmsg(s_CregServ, u->nick, "Para confirmar tu apoyo al canal 12%s hágalo escribiendo:", chan);
            privmsg(s_CregServ, u->nick, "12/msg %s APOYA %s %s", s_CregServ, chan, cr->passapoyo);
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

        if ((cr->apoyoscount) >= 2) {
            cr->estado &= ~CR_PROCESO_REG;
            cr->estado |= CR_PENDIENTE;
        } else {
            srand(time(NULL));
            sprintf(passtemp, "%05u",1+(int)(rand()%99999) );
            strscpy(cr->passapoyo, passtemp, PASSMAX);                                                                 
        }
    } 
}                                                                                                                                               
                                                        
/*************************************************************************/

static void do_apolla(User *u)
{
  
        privmsg(s_CregServ, u->nick, "No, que duele mucho (Prueba con  APOYA)");
        canalopers(s_CregServ, "12%s ha ejecutado el comando APOLLA, hehehe", u->nick);
}
        
/*************************************************************************/

static void do_estado(User *u)
{
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
    CregInfo *cr;

    if (!chan) {
/*         syntax_error(s_CregServ, u, "ESTADO", CHAN_REGISTER_SYNTAX); */
       privmsg(s_CregServ, u->nick, "ESTADO <canal>");
         return;
    }                  
    if (!(ci = cs_findchan(chan))) { 
         notice_lang(s_CregServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (!(cr = cr_findcreg(chan))) {    
         privmsg(s_CregServ, u->nick, "Canal 12%s no está registrado en la db", chan);
    } else if (cr->estado & CR_PROCESO_REG) {
         privmsg(s_CregServ, u->nick, "El canal 12%s está en proceso de registro.", chan);                           
    } else {
         privmsg(s_CregServ, u->nick, "No hay info");
    }     
}
                
/*************************************************************************/

static void do_apoyos(User *u)
{

    char *param = strtok(NULL, " ");
    
/**   NickInfo *ni; **/
  
  
    if (!param) {
/*         syntax_error(s_CregServ, u, "APOYOS", CHAN_REGISTER_SYNTAX); */
       privmsg(s_CregServ, u->nick, "APOYOS <canal>");
         return;
    }                          
    
    if (param && *param == '#') {    
        privmsg(s_CregServ, u->nick, "No hay información disponible del canal");        

    } else if (param && strchr(param, '@')) {              
        if (is_services_oper(u)) {
           privmsg(s_CregServ, u->nick, "Información de los apoyos del mail 12%s:", param);
           privmsg(s_CregServ, u->nick, "No ha realizado apoyos hasta la fecha");
        } else 
           notice_lang(s_CregServ, u, ACCESS_DENIED);      
    } else {
        if ((nick_identified(u) && (stricmp(u->nick, param) == 0)) ||
                       is_services_oper(u)) {
           privmsg(s_CregServ, u->nick, "Información de los apoyos de 12%s:", param);
           privmsg(s_CregServ, u->nick, "No ha realizado apoyos hasta la fecha");
        }   
        else
           notice_lang(s_CregServ, u, ACCESS_DENIED);
                                                                           
    }
}

/*************************************************************************/

static void do_lista(User *u)
{
}                            

/*************************************************************************/

static void do_info(User *u)
{
    char *chan = strtok(NULL, " ");
//    ChannelInfo *ci;
//    NickInfo *ni; 
    CregInfo *cr;
    ApoyosCreg *apoyos; 

    char buf[BUFSIZE];
    struct tm *tm;
    int i;        

    if (!chan) {
/*        syntax_error(s_CregServ, u, "INFO", CHAN_INFO_SYNTAX); */
       privmsg(s_CregServ, u->nick, "INFO <canal>");
    } else if (!(cr = cr_findcreg(chan))) {
        notice_lang(s_CregServ, u, CHAN_X_NOT_REGISTERED, chan); 
    } else {
        privmsg(s_CregServ, u->nick, "Informacion de canal 12%s", cr->name);
        
        privmsg(s_CregServ, u->nick, "Fundador: 12%s", cr->founder); 
        privmsg(s_CregServ, u->nick, "Email: 12%s", cr->email);
        privmsg(s_CregServ, u->nick, "Descripcion: %s", cr->desc);
        
        tm = localtime(&cr->time_peticion);
        strftime(buf, sizeof(buf), getstring(NULL,STRFTIME_DATE_TIME_FORMAT), tm);
        privmsg(s_CregServ, u->nick, "Fecha: 12%s", buf);
        
        if (cr->estado & CR_PROCESO_REG) 
            privmsg(s_CregServ, u->nick, "ESTADO: Pendiente de apoyos.");
            
        if (cr->estado & CR_EXPIRADO)
            privmsg(s_CregServ, u->nick, "ESTADO: Ha expirado la petición.");

        if (cr->estado & CR_PENDIENTE)
            privmsg(s_CregServ, u->nick, "ESTADO: Pendiente de aprobacion por CRED.");

        if (cr->estado & CR_DENEGADO)
            privmsg(s_CregServ, u->nick, "ESTADO: La peticion ha sido denegada.");
                                                                        
        if (cr->estado & CR_PROHIBIDO)
            privmsg(s_CregServ, u->nick, "ESTADO: El canal está prohiibido.");
                                
        if (cr->estado & CR_DROPADO)
            privmsg(s_CregServ, u->nick, "ESTADO: Pendiente de apoyos");
                                
        if (cr->estado & CR_SUSPENDIDO)
            privmsg(s_CregServ, u->nick, "ESTADO: Canal suspendido temporalmente.");
                    
        if (cr->estado & CR_DESCONOCIDO)
            privmsg(s_CregServ, u->nick, "ESTADO: Canal registrado pero no lo conoce CHaN");
                                                    
        if ((cr->estado & CR_ACEPTADO) || (cr->estado & CR_REGISTRADO))
            privmsg(s_CregServ, u->nick, "ESTADO: Canal Registrado");
            
        if (cr->estado & CR_ESPECIAL)
            privmsg(s_CregServ, u->nick, "ESTADO: Canal para usos de la red");

        privmsg(s_CregServ, u->nick, "APOYOS:");
        for (apoyos = cr->apoyos, i = 0; i < cr->apoyoscount; apoyos++, i++) {           
              tm = localtime(&apoyos->time_apoyo);
              strftime(buf, sizeof(buf), getstring(NULL,STRFTIME_DATE_TIME_FORMAT), tm);                      
              privmsg(s_CregServ, u->nick, "  %s  %s  %s", apoyos->nickapoyo, apoyos->emailapoyo, buf);
          } 
        privmsg(s_CregServ, u->nick, "Histórico:");
    }
 
}                       

/*************************************************************************/

static void do_suspend(User *u)
{
}



/*************************************************************************/

static void do_unsuspend(User *u)
{
}


/*************************************************************************/

static void do_marca(User *u)
{
}


/*************************************************************************/

static void do_ignore(User *u)
{
}

/*************************************************************************/

static void do_anula(User *u)
{
}
         

/*************************************************************************/

static void do_acepta(User *u)
{
}

/*************************************************************************/

static void do_restaura(User *u)
{
}

/*************************************************************************/

static void do_regcanal(User *u)
{
    char *chan = strtok(NULL, " ");
    char *founder = strtok(NULL, " ");
    char *email = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");    
    char *desc = strtok(NULL, "");
    
    CregInfo *cr;    
    
    if (readonly) {
        notice_lang(s_CregServ, u, CHAN_REGISTER_DISABLED);
        return;
    }
                   
    if (!desc) {
 /*         syntax_error(s_CregServ, u, "REGISTRA", CREG_REGISTER_SYNTAX); */
        privmsg(s_CregServ, u->nick, "REGCANAL <canal> <founder> <password> <descripcion>");
    } else if (*chan == '&') {
        notice_lang(s_CregServ, u, CHAN_REGISTER_NOT_LOCAL);
    } else if (!(*chan == '#')) {
        privmsg(s_CregServ, u->nick, "Canal no valido para registrar");    
    } else if (!(cr = makecreg(chan))) {
        log("%s: makecreg() failed for REGISTER %s", s_CregServ, chan);
        notice_lang(s_CregServ, u, CHAN_REGISTRATION_FAILED);
    } else {
        cr = makecreg(chan);
                                               
        cr->founder = sstrdup(founder);
        if (strlen(pass) > PASSMAX-1) /* -1 for null byte */
            notice_lang(s_CregServ, u, PASSWORD_TRUNCATED, PASSMAX-1);
        strscpy(cr->founderpass, pass, PASSMAX);        
        cr->email = sstrdup(email);
        cr->desc = sstrdup(desc);
        
        cr->time_peticion = time(NULL);
        cr->estado = CR_ESPECIAL;
        
        privmsg(s_CregServ, u->nick, "Canal 12%s registrado con status especial.", chan);
        canalopers(s_CregServ, "12%s ha registrado el canal especial 12%s.", u->nick, chan);
   }                                                   
}
                               
/*************************************************************************/

static void do_rechaza(User *u)
{
}
                      
/*************************************************************************/

static void do_deniega(User *u)
{
}

/*************************************************************************/

static void do_drop(User *u)
{
}
                   
/*************************************************************************/

static void do_prohibe(User *u)
{
}
                      
/*************************************************************************/
       
static void do_permite(User *u)
{
}

/*************************************************************************/

static void do_activa(User *u)
{
}

/*************************************************************************/

static void do_desactiva(User *u)
{
}
                                                                                                                                                                                                                                      
#endif /* CREGSERV */