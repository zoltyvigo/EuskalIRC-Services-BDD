/* prueba */
/* #ifdef A_TOMAR_POR_EL_CULO */
#include "services.h"


#ifdef SMTP
int check_smtp(char buf[BUFSIZE]);
#endif


/**************** enviar_correo *******************************
Esta funcion envia un correo con el email configurado del source k sea
con el subject y body pertinentes al destino indicado.
*************************************************************/
int enviar_correo(const char * destino, const char *subject, const char *body)
{
#ifdef SMTP
    int smtpsock = -1;
    char bufer[BUFSIZE];
    char tmp[BUFSIZE];
            
//    smtpsock = conn(ServerSMTP, PortSMTP, "localhost", 0);
    smtpsock = conn("smtp.eresmas.com", 25, "localhost", 0);
    if (smtpsock < 0) {
        log("SMTP Error :no se pudo abrir socket");
        return 0;
    }

    sgets(bufer, sizeof(bufer), smtpsock);
    if (!check_smtp(bufer)) {
        disconn(smtpsock);
        return 0;
    }

    sputs("HELO 216-VIGO-X13.libre.retevision.es\r\n",smtpsock);
    sgets(bufer, sizeof(bufer), smtpsock);
    if (!check_smtp(bufer)) {
        disconn(smtpsock);
        return 0;
    }

    sprintf(tmp,"MAIL FROM: %s\n",SendFrom);
    sputs(tmp,smtpsock);
    sgets(bufer, sizeof(bufer), smtpsock);
    if (!check_smtp(bufer)) {
        disconn(smtpsock);
        return 0;
    }
    
    sprintf(tmp,"RCPT TO: %s\n",destino);
    sputs(tmp,smtpsock);
    sgets(bufer, sizeof(bufer), smtpsock);
    if (!check_smtp(bufer)) {
        disconn(smtpsock);
        return 0;
    }
    
    sputs("DATA\n",smtpsock);
    sgets(bufer, sizeof(bufer), smtpsock);
    if (!check_smtp(bufer)) {
        disconn(smtpsock);
        return 0;
    }        

    sprintf(tmp,"From: %s\n",SendFrom);
    sputs(tmp,smtpsock);
    sprintf(tmp,"To: %s\n",destino);
    sputs(tmp,smtpsock);
    sprintf(tmp,"Subject: %s\n",subject);
    sputs(tmp,smtpsock);
             
    sprintf(tmp,"%s\n\n",body);
    sputs(tmp,smtpsock);    
    
    sputs(".\n",smtpsock);
    sgets(bufer, sizeof(bufer), smtpsock);
    if (!check_smtp(bufer)) {
        disconn(smtpsock);
        return 0;
    }    

    sputs("QUIT\n",smtpsock);
        
    disconn(smtpsock);
    smtpsock = -1;
                
    return 1;
}    
#endif
#ifdef SENDMAIL
    FILE *p;
    char sendmail[PATH_MAX];
                                                                              
    snprintf(sendmail, sizeof(sendmail), SendMailPatch, destino);
                                                                                           
    if (!(p = popen(sendmail, "w"))) {
//        privmsg(s_NickServ, u->nick, "mail jodido");
        log("mail jodio");
        return 0;
    }

    fprintf(p, "From: %s\n", SendFrom);
    fprintf(p, "To: %s\n", destino);
    fprintf(p, "Subject: %s\n", subject);    
    fprintf(p, "%s\n", body);

    fprintf(p, "\n.\n");
         
    pclose(p);    
    
    return 1;    
}    
#endif



#ifdef SMTP
int check_smtp(char buf[BUFSIZE])
{
/** Esta funcion recibe la contestacion k recibimos del smtp y nos
devuelve si se ha producido un erro o no. Caso de ser asi, logea el
error. ***/

    char cod[5];
    int codigo;
        
  //log("SMTP = %s",buf);
    strncpy(cod,buf,4);
  //log("strncpy....");
    codigo = strlen(cod);
    cod[codigo] = '\0';
  //log("asignado cod..");
    if (cod) codigo = atoi(cod);
    else codigo = 500;
    
  //log("Codigo SMTP = %d",codigo);

    switch(codigo) {
        case 220:
            return 1;
            break;
                 
        case 221:
            return 1;
            break;    

        case 250:
            return 1;
            break;
                                
        case 354:
            return 1;
            break;            
            
        default:
            log("SMTP ERROR(%d) : %s",codigo,buf);
            return 0;
    }            
    return 0;
}
#endif

// #endif /* #ifdef A_TOMAR_POR_EL_CULO */