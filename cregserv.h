/* Declarations for CregServ.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * DarkuBots es una adaptaci�n de Javier Fern�ndez Vi�a, ZipBreake.
 * E-Mail: javier@jfv.es || Web: http://jfv.es/
 *
 * Modificaciones -donostiarra- admin.euskalirc@gmail.com   http://euskalirc.wordpress.com
 *					
 *						
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
    int32 seccion;            /* Seccion del canal Principales*/ 
    int32 geo;            /* Secciones geogr�ficas*/ 
    char *desc;                  /* Descripcion del canal */
    
    int32 tipo;                     /*COMercial  OFIcial REPresentantes */
    char *email;                 /* Email del founder */
    time_t time_peticion;        /* Hora de la peticion */
              
    char *nickoper;              /* Nick del OPER */
    time_t time_motivo;          /* Hora del cambio de estado del canal */
    char *motivo;                /* Motivo de la suspension, etc.. */
                                        
    char passapoyo[PASSMAX];     /* Contrase�a de apoyo */
    time_t time_lastapoyo;       /* Hora del ultimo apoyo realizado */
                                    
    int32 flags; /*para filtrado,listados*/ 
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
#define CR_JUE   0x00000010     /*Juegos y Clanes*/
#define	CR_CUH   0x00000020     /*Cultura y Humanidades*/
#define CR_SOC   0x00000040	/*Sociedad*/
#define CR_CIE   0x00000080	/*Ciencias*/
#define CR_DEP   0x00000200     /*Deportes*/
#define CR_INF   0x00000800	/*Informatica*/
#define	CR_LCT   0x00002000     /*Literatura, Cine y TV*/
#define	CR_MUS   0x00004000     /*Musica*/
#define	CR_OCI   0x00008000     /*Ocio*/
#define	CR_PAI   0x00020000     /*Paises y continentes*/
#define	CR_PRF   0x00040000     /*Profesiones-Oficios*/
#define CR_ADU   0x00080000     /*Contenido para Adultos*/
#define CR_AMO   0x00100000     /*Amor y amistad*/
#define CR_SEX   0x00200000     /*Sexo*/
#define CR_RAD   0x00800000     /*Radio*/

/* estan en int32 geo;*/
#define CR_AND   0x00000002	/*Andaluc�a*/
#define CR_ARA   0x00000004	/*Arag�n*/
#define CR_AST   0x00000010     /*Principado de Asturias*/
#define	CR_BAL   0x00000020     /*Islas Baleares*/
#define CR_ICA   0x00000040	/*Islas Canarias*/
#define CR_CAN   0x00000080	/*Cantabria*/
#define CR_CAT   0x00000200     /*Catalunya*/
#define CR_CAM   0x00000800	/*Castilla-La Mancha*/
#define	CR_CAL   0x00002000     /*Castilla-Le�n*/
#define	CR_CEM   0x00004000     /*Ceuta-Melilla*/
#define	CR_EUS   0x00008000     /*Euskadi-Pa�s Vasco*/
#define	CR_EXT   0x00020000     /*Extremadura*/
#define	CR_GAE   0x00040000     /*Galicia*/
#define CR_LRI   0x00080000     /*La Rioja*/
#define CR_MAD   0x00100000     /*Madrid*/
#define CR_MUR   0x00200000     /*Murcia*/
#define CR_NAV   0x00800000     /*Navarra*/
#define CR_VAL   0x02000000     /*Valencia*/



/* estan en int32 tipo;*/
#define CR_COM     0x00000001     /*Canales Comerciales*/
#define CR_OFI    0x00000002	  /*Canales Oficiales*/
#define CR_REP     0x00000004	  /*Canales De Representantes*/



