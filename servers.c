/* Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * Modulo de control de servidores hecho por zoltan <zolty@ctv.es>
 * Julio 2000
 *
 */

#include "services.h"
#include "pseudo.h"

int nodos;
static Server *serverlist; 
static Server *lastserver = NULL;


/*************************************************************************/
/**************************** Funciones Internas *************************/
/*************************************************************************/

void del_server(Server *server)
{
    if (debug >= 2)
        log("Servers: llamada del_server()");
            
    if (server == lastserver)
        lastserver = NULL;

    servercnt--;
    
    del_users_server(server);
    
    free(server->name);
    if (server->prev)
        server->prev->next = server->next;
    else
        serverlist = server->next;
    if (server->next)
        server->next->prev = server->prev;
    free(server);    

    if (debug >= 2)
        log("Servers: del_server() OK.");
}    
/*************************************************************************/
void control_del_server(Server *padre, const char *razon)
{
    Server *server, *nextserver;
    
    server = padre->hijo;
    if (debug)
        log("Control borrado servers, HUB: %s", padre->name);
    while (server) {
        if (server->hijo)
            control_del_server(server, razon);
        if (debug)
            log("Control borrado servers, Leaf: %s", server->name);
        nextserver = server->rehijo;
        usercnt = usercnt - server->users;
        del_server(server);
        server = nextserver;
    }
}

/*************************************************************************/
Server *add_server(const char *servername)
{
    Server *server;
    
    servercnt++;
    server = scalloc(sizeof(Server), 1);
    server->name = sstrdup(servername);
                
    server->next = serverlist;
    if (server->next)
        server->next->prev = server;
    serverlist = server;
                                 
    return server;
}

/*************************************************************************/
Server *find_servername(const char *servername)
{
    Server *server;
    
    if (!servername)
        return NULL;
                
    if (lastserver && stricmp(servername, lastserver->name) == 0)
        return lastserver;
    for (server = serverlist; server; server = server->next) {
       if (stricmp(servername, server->name) == 0) {
           lastserver = server;
           return server;
       }
    }
                                        
    return NULL;
}
                            
/*************************************************************************/
Server *find_servernumeric(const char *numerico)
{
    Server *server;
    
    if (!numerico)
        return NULL;
                       
    if (lastserver && strncmp(numerico, lastserver->numerico,1) == 0)
        return lastserver;

    for (server = serverlist; server; server = server->next) {
        if (strncmp(numerico, server->numerico,1) == 0) {
            lastserver = server;
            return server;
        }
    }
                                                                                                     
    return NULL;
}

/*************************************************************************/

 /* Salir mensaje en el Canal de Control los servers ke van entrando
  * Zoltan Julio 2000
  */

void do_server(const char *source, int ac, char **av)
{
          
/* Handle a server SERVER command.
 *      source = Server hub (si no tiene source, es su hub)
 *      av[0] = Nuevo Server
 *      av[1] = hop count
 *      av[2] = signon time
 *      av[3] = signon
 *      av[4] = protocolo
 *      av[5] = numerico
 *      av[6] = user's server
 *      av[7] = Descripcion
 */

    Server *server, *tmpserver;
    
    server = add_server(av[0]);
    server->numerico = sstrdup(av[5]);
    server->hijo = NULL;
    server->rehijo = NULL;
#ifdef IRC_UNDERNET_P10
    server->hub = find_servernumeric(source);
#else
    server->hub = find_servername(source);
#endif        
/******** a mi me parece ke ta mal     
    if (!*source) {
        ServerHUB = sstrdup(server->name);
        server->hub = find_servername(av[0]);
        return;
    }    
    ******/
    if (!*source) {
        server->name = sstrdup(ServerHUB);
        server->hub = find_servername(av[0]);
    }    

    if (!server->hub) {
     /*
      * BASURILLA: En un futuro, eliminar del codigo.
      */
        log("Server: No puedo encontrar el HUB %s de %s", source, av[0]);
        return;
    }    
  
    if (!server->hub->hijo) {
        server->hub->hijo = server;
    } else {
        tmpserver = server->hub->hijo;
        while (tmpserver->rehijo)
        tmpserver = tmpserver->rehijo;
        tmpserver->rehijo = server;
    }
    canalopers(s_OperServ, "SERVER 12%s Numeric 12%s entra en la RED.", av[0], sstrdup(av[5]));
    return;
}    


/*************************************************************************/

 /* Salir mensaje en el canal de control los servers ke salen de la red
  * Zoltan Julio 2000
  */

void do_squit(const char *source, int ac, char **av)
{ 

/* Handle a server SQUIT command.
 *       soruce =  nick/server
 *      av[0] = Nuevo Server
 *      av[1] = signon time
 *      av[2] = motivo
 */

    Server *server, *tmpserver;
    
    server = find_servername(av[0]);
        
    if (server) {
        control_del_server(server, av[1]);
        if (server->hub) {
            if (server->hub->hijo == server) {
                server->hub->hijo = server->rehijo;
            } else {
                for (tmpserver = server->hub->hijo; tmpserver->rehijo;
                        tmpserver = tmpserver->rehijo) {
                     if (tmpserver->rehijo == server) {
                         tmpserver->rehijo = server->rehijo;
                         break;
                     }
                }
            }    
            if (nodos < 1)
                canalopers(s_OperServ, "SQUIT de 12%s, llevandose %d usuarios", server->name, server->users);
            else {
                canalopers(s_OperServ, "SQUIT de 12%s, llevandose %d nodos y %d usuarios", server->name, nodos, server->users);    
                canalopers(s_OperServ, "SQUIT de 12%s por C12%sC Motivo: %s", av[0], source, av[2]);
            }
            log("Eliminando servidor %s", server->name);
            del_server(server);
            }                                            
    } else {        
        log("Server: Tratando de eliminar el servidor inexsistente: %s", av[0]);
        return;
    }     
}

/*************************************************************************/
void do_servers(User *u)
{
     Server *server;
     int porcentaje = 0, media = 0;
     privmsg(s_OperServ, u->nick, "Nombre servidor                   Numerico   Usuarios  Porcentaje");
     for (server = serverlist; server; server = server->next) {     
         if (server->name)
              porcentaje = ((server->users * 100) / usercnt);
              privmsg(s_OperServ, u->nick, "12%-25s             12%-8s  %-8d  %d",
                server->name, server->numerico, server->users, porcentaje);
     }
     privmsg(s_OperServ, u->nick, "Hay 12%d usuarios en 12%d servidores conectados.", usercnt, servercnt);
     media = (usercnt / servercnt);
     privmsg(s_OperServ, u->nick, "Media de usuarios por servidor: %d", media);
}