/* Declarations for CregServ.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * DarkuBots es una adaptación de Javier Fernández Viña, ZipBreake.
 * E-Mail: javier@jfv.es || Web: http://jfv.es/
 *
 */
          
typedef struct {
    char *nickapoyo;
    char *emailapoyo;
    time_t time_apoyo;
} ApoyosCreg;
            

typedef struct creginfo_ CregInfo;

struct creginfo_ {
    CregInfo *next, *prev;
    char name[CHANMAX];          /* Nombre del canal */
    char *founder;               /* Nick del founder */
            
    char founderpass[PASSMAX];   /* Password de founder */
    int32 seccion;            /* Seccion del canal */ /*SOCiedad  INFormatica CIencia AYUda ADULtos*/ 
    char *desc;                  /* Descripcion del canal */
    
    int32 tipo;                     /*COMercial  OFIcial REPresentantes */
    char *email;                 /* Email del founder */
    time_t time_peticion;        /* Hora de la peticion */
              
    char *nickoper;              /* Nick del OPER */
    time_t time_motivo;          /* Hora del cambio de estado del canal */
    char *motivo;                /* Motivo de la suspension, etc.. */
                                        
    char passapoyo[PASSMAX];     /* Contraseña de apoyo */
    time_t time_lastapoyo;       /* Hora del ultimo apoyo realizado */
                                                
    int32 estado;   		/* Estado del canal */

    int16 apoyoscount;           /* Contador del numero de apoyos */
    ApoyosCreg *apoyos;          /* Lista de los apoyos realizados */
};
                
                
#define CR_PROCESO_REG  0x00000001
#define CR_EXPIRADO     0x00000002
#define CR_RECHAZADO    0x00000004
#define CR_DROPADO      0x00000008
#define CR_ACEPTADO     0x00000010
#define CR_REGISTRADO   0x00000020
#define CR_SUSPENDIDO   0x00000040
#define CR_MARCADO      0x00000080

/* estan en int32 seccion;*/





#define CR_PRO   0x00000002	/*De Proyectos,Investigaciones*/
#define CR_AYU   0x00000004	/*De Ayuda*/
#define CR_JUE   0x00000010      /*Juegos y Clanes*/
#define	CR_CUH   0x00000020     /*Cultura y Humanidades*/
#define CR_SOC   0x00000040	/*Sociedad*/
#define CR_CIE   0x00000080	/*Ciencias*/
#define CR_DEP   0x00000200        /*Deportes*/
#define CR_INF   0x00000800	   /*Informatica*/
#define	CR_LCT   0x00002000        /*Literatura, Cine y TV*/
#define	CR_MUS   0x00004000        /*Musica*/
#define	CR_OCI   0x00008000        /*Ocio*/
#define	CR_PAI   0x00020000        /*Paises y continentes*/
#define	CR_PRF   0x00040000        /*Profesiones-Oficios*/
#define CR_ADU   0x00080000       /*Contenido para Adultos*/
#define CR_AMO   0x00100000      /*Amor y amistad*/
#define CR_SEX   0x00200000     /*Sexo*/



/* estan en int32 tipo;*/
#define CR_COM     0x00000001     /*Canales Comerciales*/
#define CR_OFI    0x00000002	  /*Canales Oficiales*/
#define CR_REP     0x00000004	  /*Canales De Representantes*/



