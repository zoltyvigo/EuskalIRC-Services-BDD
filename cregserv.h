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
          
#include "sysconf.h"
#include "config.h"


#ifdef CREGSERV
/* Estructura de peticiones de registro en canales en medio de CReG */

typedef struct {
    char *nickapoyo;
        char *emailapoyo;
            time_t time_apoyo;
            } ApoyosCreg;
            
            typedef struct {
                char *nickoper;
                    char *marca;
                        time_t time_marca;
                        } HistoryCreg;


typedef struct creginfo_ CregInfo;

struct creginfo_ {
    CregInfo *next, *prev;
        char name[CHANMAX];          /* Nombre del canal */
            char *founder;               /* Nick del founder */
            
                char founderpass[PASSMAX];   /* Password de founder */
                    char *desc;                  /* Descripcion del canal */
                        char *email;                 /* Email del founder */
                            time_t time_peticion;        /* Hora de la peticion */
                            
                                char *nickoper;              /* Nick del OPER */
                                    time_t time_motivo;          /* Hora del cambio de estado del canal */
                                        char *motivo;                /* Motivo de la suspension, etc.. */
                                        
                                            char passapoyo[PASSMAX];     /* Contraseña de apoyo */
                                                time_t time_lastapoyo;       /* Hora del ultimo apoyo realizado */
                                                
                                                    int32 estado;                /* Estado del canal :) CR_* */

    int16 apoyoscount;           /* Contador del numero de apoyos */
        ApoyosCreg *apoyos;          /* Lista de los apoyos realizados */
        
            int16 historycount;          /* Contador de Históricos */
                HistoryCreg *history;        /* Lista historico del canal */
                
                };
                
                
/* Estados de las peticiones del canal  */
/* Canal en proceso de registro */
#define CR_PROCESO_REG  0x00000001
/* Canal ha expirado */
#define CR_EXPIRADO     0x00000002
/* Canal pendiente de aceptacion */
#define CR_PENDIENTE    0x00000004
/* Canal denegado */
#define CR_DENEGADO     0x00000008
/* Canal rechazado */
#define CR_RECHAZADO    0x00000010
/* Canal prohibido */
#define CR_PROHIBIDO    0x00000020
/* Canal dropado */
#define CR_DROPADO      0x00000040
/* Canal suspendido */
#define CR_SUSPENDIDO   0x00000080
/* Canal en estado desconocido */
#define CR_DESCONOCIDO  0x00000100
/* Canal aceptado y registrado en Chanserv */
#define CR_ACEPTADO     0x00000200
/* Canal registrado especial */
#define CR_ESPECIAL     0x00000400
/* Canal registrado sin usar a creg */
#define CR_REGISTRADO   0x00000800

#endif /* CREGSERV * Migrar a Cregserv.h */                                                                                                      