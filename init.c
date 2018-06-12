/* Initalization and related routines.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"

/*************************************************************************/

/* Send a NICK command for the given pseudo-client.  If `user' is NULL,
 * send NICK commands for all the pseudo-clients. */

#if defined(IRC_UNDERNET_P10)
#  define NICK(nick,modos,numerico,name) \
    do { \
	send_cmd(NULL, "%c N %s 1 %lu %s %s %s AAAAAA %s :%s", convert2y[ServerNumerico], (nick), time(NULL),\
		ServiceUser, ServiceHost, (modos), (numerico), (name)); \
    } while (0)
# else
#  define NICK(nick,name) \
    do { \
        send_cmd(ServerName, "NICK %s 1 %ld %s %s %s :%s", (nick), time(NULL),\
                ServiceUser, ServiceHost, ServerName, (name)); \
    } while (0)

#  define CNICK(nick,name,user,host) \
            do { \
	    send_cmd(ServerName, "NICK %s 1 %ld %s %s %s :%s", (nick), time(NULL),\
                (user), (host), ServerName, (name)); \
            } while (0)
# endif    
#  define SPAM(nick,name) \
    do { \
        send_cmd(ServerName, "NICK %s 1 %ld %s %s %s :%s", (nick), time(NULL),\
                ServiceUserSpam, ServiceHostSpam, ServerName, (name)); \
    } while (0)
#  define euskalirc(nick,name) \
    do { \
        send_cmd(ServerName, "NICK %s 1 %ld %s %s %s :%s", (nick), time(NULL),\
                ServiceUserEuskalIRC, ServiceHostEuskalIRC, ServerName, (name)); \
    } while (0)

void introduce_user(const char *user)
{
#if defined(IRC_UNDERNET_P10)
    char *modos=NULL;
#endif    
    
    /* Watch out for infinite loops... */
#define LTSIZE 20
    static int lasttimes[LTSIZE];
    if (lasttimes[0] >= time(NULL)-3)
	fatal("introduce_user() loop detected");
    memmove(lasttimes, lasttimes+1, sizeof(lasttimes)-sizeof(int));
    lasttimes[LTSIZE-1] = time(NULL);
#undef LTSIZE

    
#if defined(IRC_UNDERNET_P10)
    if (!user || stricmp(user, s_NickServ) == 0 || stricmp(user, s_NickServP10) == 0) {
        s_NickServP10[0]=convert2y[ServerNumerico];
        s_NickServP10[1]='\0';
        strcat(s_NickServP10, "AA");
        modos="+okdrh";
        NICK(s_NickServ, modos, s_NickServP10, desc_NickServ);
        send_cmd(s_NickServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_NickServP10);
	send_cmd(s_NickServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_NickServP10);
    }    
    if (!user || stricmp(user, s_ChanServ) == 0 || stricmp(user, s_ChanServP10) == 0) {
        s_ChanServP10[0]=convert2y[ServerNumerico];
        s_ChanServP10[1]='\0';
        strcat(s_ChanServP10, "AB");
        modos="+Bkrhd";
        NICK(s_ChanServ, modos, s_ChanServP10, desc_ChanServ);
        send_cmd(s_ChanServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_ChanServP10);
	 send_cmd(s_ChanServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_ChanServP10);
    }    
    if (!user || stricmp(user, s_MemoServ) == 0 || stricmp(user, s_MemoServP10) == 0) {    
        s_MemoServP10[0]=convert2y[ServerNumerico];
        s_MemoServP10[1]='\0';
        strcat(s_MemoServP10, "AE");
        modos="+krhd";
        NICK(s_MemoServ, modos, s_MemoServP10, desc_MemoServ);
        send_cmd(s_MemoServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_MemoServP10);
	send_cmd(s_MemoServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_MemoServP10);
    }                                                                                                                                                                                                                                                                                                                
    if (!user || stricmp(user, s_OperServ) == 0 || stricmp(user, s_OperServP10) == 0) {    
        s_OperServP10[0]=convert2y[ServerNumerico];
        s_OperServP10[1]='\0';
        strcat(s_OperServP10, "AF");
        modos="+Bkorhdi";
        NICK(s_OperServ, modos, s_OperServP10, desc_OperServ);
        send_cmd(s_OperServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_OperServP10);
        send_cmd(s_OperServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_OperServP10);
    }
    if (!user || stricmp(user, s_XServ) == 0 || stricmp(user, s_XServP10) == 0) {    
        s_XServP10[0]=convert2y[ServerNumerico];
        s_XServP10[1]='\0';
        strcat(s_XServP10, "AF");
        modos="+Bkorhdi";
        NICK(s_XServ, modos, s_XServP10, desc_XServ);
        send_cmd(s_XServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_XServP10);
        send_cmd(s_XServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_XServP10);
    }
    if (!user || stricmp(user, s_NewsServ) == 0 || stricmp(user, s_NewsServP10) == 0) {
        s_NewsServP10[0]=convert2y[ServerNumerico];
        s_NewsServP10[1]='\0';
        strcat(s_NewsServP10, "AG");
        modos="+krd";
        NICK(s_NewsServ, modos, s_NewsServP10, desc_NewsServ);
        send_cmd(s_NewsServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_NewsServP10);
	send_cmd(s_NewsServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_NewsServP10);
    }   
      if (!user || stricmp(user, s_SpamServ) == 0 || stricmp(user, s_SpamServP10) == 0) {
        s_SpamServP10[0]=convert2y[ServerNumerico];
        s_SpamServP10[1]='\0';
        strcat(s_SpamServP10, "AH");
        modos="+kro";
        NICK(s_SpamServ, modos, s_SpamServP10, desc_SpamServ);
        send_cmd(s_SpamServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_SpamServP10);
	send_cmd(s_SpamServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_SpamServP10);

    }
    if (!user || stricmp(user, s_GlobalNoticer) == 0 || stricmp(user, s_GlobalNoticerP10) == 0) {
        s_GlobalNoticerP10[0]=convert2y[ServerNumerico];
        s_GlobalNoticerP10[1]='\0';
        strcat(s_GlobalNoticerP10, "AI");
        modos="+krhod";
        NICK(s_GlobalNoticer, modos, s_GlobalNoticerP10, desc_GlobalNoticer);
        send_cmd(s_GlobalNoticer, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_GlobalNoticerP10);
	 send_cmd(s_GlobalNoticer, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_GlobalNoticerP10);
    }    
    if (!user || stricmp(user, s_HelpServ) == 0 || stricmp(user, s_HelpServP10) == 0) {
        s_HelpServP10[0]=convert2y[ServerNumerico];
        s_HelpServP10[1]='\0';
        strcat(s_HelpServP10, "AJ");
        modos="+krd";
        NICK(s_HelpServ, modos, s_HelpServP10, desc_HelpServ);
        send_cmd(s_HelpServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_HelpServP10);
	send_cmd(s_HelpServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_HelpServP10);
    }
    if (s_IrcIIHelp && (!user || stricmp(user, s_IrcIIHelp) == 0 || stricmp(user, s_IrcIIHelpP10) == 0)) {
        s_IrcIIHelpP10[0]=convert2y[ServerNumerico];
        s_IrcIIHelpP10[1]='\0';
        strcat(s_IrcIIHelpP10, "AK");
        modos="+krd";
        NICK(s_IrcIIHelp, modos, s_IrcIIHelpP10, desc_IrcIIHelp);
        send_cmd(s_IrcIIHelp, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_IrcIIHelpP10);
	send_cmd(s_IrcIIHelp, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_IrcIIHelpP10);
    }    
	if (s_mIRCHelp && (!user || stricmp(user, s_mIRCHelp) == 0 || stricmp(user, s_mIRCHelpP10) == 0)) {
        s_IrcIIHelpP10[0]=convert2y[ServerNumerico];
        s_IrcIIHelpP10[1]='\0';
        strcat(s_IrcIIHelpP10, "AK");
        modos="+krd";
        NICK(s_mIRCHelp, modos, s_mIRCHelpP10, desc_mIRCHelp);
        send_cmd(s_IrcIIHelp, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_mIRCHelpP10);
	send_cmd(s_mIRCHelp, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_mIRCHelpP10);
    }    
   
   if (!user || stricmp(user, s_EuskalIRCServ) == 0 || stricmp(user, s_EuskalIRCServP10) == 0) {
        s_EuskalIRCServP10[0]=convert2y[ServerNumerico];
        s_EuskalIRCServP10[1]='\0';
        strcat(s_EuskalIRCServP10, "AH");
        modos="+kro";
        NICK(s_EuskalIRCServ, modos, s_EuskalIRCServP10, desc_EuskalIRCServ);
        send_cmd(s_EuskalIRCServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_EuskalIRCServP10);
	send_cmd(s_EuskalIRCServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_EuskalIRCServP10);

    }
if (!user || stricmp(user, s_CregServ) == 0 || stricmp(user, s_CregServP10) == 0) {
        s_CregServP10[0]=convert2y[ServerNumerico];
        s_CregServP10[1]='\0';
        strcat(s_CregServP10, "AJ");
        modos="+krd";
        NICK(s_CregServ, modos, s_CregServP10, desc_CregServ);
        send_cmd(s_CregServ, "J #%s", CanalOpers);
        send_cmd(ServerName, "M #%s +o %s", CanalOpers, s_CregServP10);
	send_cmd(s_CregServ, "J #%s", CanalAdmins);
        send_cmd(ServerName, "M #%s +o %s", CanalAdmins, s_CregServP10);
    }
	
#else
    if (!user || stricmp(user, s_ChanServ) == 0) {
       #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_ChanServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_ChanServ);
      #endif
	NICK(s_ChanServ, desc_ChanServ);
        #if defined (IRC_UNDERNET_P09) || defined (IRC_PATCHS_P09)
        send_cmd(s_ChanServ, "MODE %s +Bikdr", s_ChanServ);
        send_cmd(s_ChanServ, "JOIN #%s", CanalOpers);
	send_cmd(s_ChanServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_ChanServ);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_ChanServ);
       #endif
       #if defined(IRC_PATCHS_P09)
       	/*send_cmd(s_ChanServ, "MODE #%s +ntsO", CanalOpers); <--cuando se implemente este modo lo active desde
                                                    		 un service*/
        send_cmd(s_ChanServ, "MODE #%s +ntsil 1", CanalOpers);
       #else
       	send_cmd(s_ChanServ, "MODE #%s +ntsil 1", CanalOpers);
       #endif
    }
    if (!user || stricmp(user, s_NickServ) == 0) {
    #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_NickServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_NickServ);
      #endif
	
            NICK(s_NickServ, desc_NickServ);
        send_cmd(s_NickServ, "MODE %s +Biokdr", s_NickServ);
        send_cmd(s_NickServ, "JOIN #%s", CanalOpers);
	send_cmd(s_NickServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_NickServ);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_NickServ);
          }

    if (!user || stricmp(user, s_HelpServ) == 0) {
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_HelpServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_HelpServ);
      #endif
	NICK(s_HelpServ, desc_HelpServ);
        send_cmd(s_HelpServ, "MODE %s +Bikdr", s_HelpServ);
	send_cmd(s_HelpServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_HelpServ, "JOIN #%s", CanalAyuda);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAyuda, s_HelpServ);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_HelpServ);
       

    }
    
     if (s_IrcIIHelp && (!user || stricmp(user, s_IrcIIHelp) == 0)) {
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_IrcIIHelp);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_IrcIIHelp);
      #endif
	
	NICK(s_IrcIIHelp, desc_IrcIIHelp);
         send_cmd(s_IrcIIHelp, "JOIN #%s", CanalAdmins);
	  send_cmd(s_IrcIIHelp, "JOIN #%s", CanalAyuda);
        send_cmd(s_IrcIIHelp, "MODE %s +Bikdr", s_IrcIIHelp);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_IrcIIHelp);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAyuda, s_IrcIIHelp);
              
    }
    if (s_mIRCHelp && (!user || stricmp(user, s_mIRCHelp) == 0)) {
      
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_mIRCHelp);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_mIRCHelp);
      #endif
	NICK(s_mIRCHelp, desc_mIRCHelp);
         send_cmd(s_mIRCHelp, "JOIN #%s", CanalAdmins);
	send_cmd(s_mIRCHelp, "JOIN #%s", CanalAyuda);
        send_cmd(s_mIRCHelp, "MODE %s +Bikdr", s_mIRCHelp);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_mIRCHelp);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAyuda, s_mIRCHelp);
              
    }

    if (!user || stricmp(user, s_MemoServ) == 0) {
      
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_MemoServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_MemoServ);
      #endif

	NICK(s_MemoServ, desc_MemoServ);
	send_cmd(s_MemoServ, "MODE %s +Bikdr", s_MemoServ);
	send_cmd(s_MemoServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_MemoServ);
       
    }
    if (!user || stricmp(user, s_OperServ) == 0) {
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_OperServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_OperServ);
      #endif

	NICK(s_OperServ, desc_OperServ);
        #if defined (IRC_UNDERNET_P09) || defined (IRC_PATCHS_P09)
        send_cmd(s_OperServ, "MODE %s +Bikdor", s_OperServ);
        send_cmd(s_OperServ, "JOIN #%s", CanalAdmins);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_OperServ);
        send_cmd(s_OperServ, "JOIN #%s", CanalOpers);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_OperServ);
        #endif
        #if defined(IRC_PATCHS_P09)
       /*send_cmd(s_OperServ, "MODE #%s +ntsa", CanalAdmins); <--cuando se implemente este modo lo active desde
                                                                 un service*/
       send_cmd(s_OperServ, "MODE #%s +ntsil 1", CanalAdmins);
        #else
	send_cmd(s_OperServ, "MODE #%s +ntsil 1", CanalAdmins);
        #endif
    }
    if (!user || stricmp(user, s_XServ) == 0) {
       #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_XServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_XServ);
      #endif
	
	NICK(s_XServ, desc_XServ);
        #if defined (IRC_UNDERNET_P09) || defined (IRC_PATCHS_P09)
        send_cmd(s_XServ, "MODE %s +Bikdor", s_XServ);
        send_cmd(s_XServ, "JOIN #%s", CanalAdmins);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_XServ);
        send_cmd(s_XServ, "JOIN #%s", CanalOpers);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_XServ);
        #endif
        #if defined(IRC_PATCHS_P09)
       /*send_cmd(s_OperServ, "MODE #%s +ntsa", CanalAdmins); <--cuando se implemente este modo lo active desde
                                                                 un service*/
       send_cmd(s_XServ, "MODE #%s +ntsil 1", CanalAdmins);
        #else
	send_cmd(s_XServ, "MODE #%s +ntsil 1", CanalAdmins);
        #endif
    }
    if (!user || stricmp(user, s_CregServ) == 0) {
      
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_CregServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_CregServ);
      #endif

	NICK(s_CregServ, desc_CregServ);
        send_cmd(s_CregServ, "MODE %s +Bikdr", s_CregServ);
        send_cmd(s_CregServ, "JOIN #%s", CanalAdmins);
	 send_cmd(s_CregServ, "JOIN #%s", CanalOpers);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_CregServ);
	 send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_CregServ);
        
    }
     if (!user || stricmp(user, s_EuskalIRCServ) == 0) {
      
       #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_EuskalIRCServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_EuskalIRCServ);
      #endif
	
	euskalirc(s_EuskalIRCServ, desc_EuskalIRCServ);
        send_cmd(s_EuskalIRCServ, "MODE %s +Bobikr", s_EuskalIRCServ);
        send_cmd(s_EuskalIRCServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_EuskalIRCServ, "JOIN #%s", CanalOpers);
	send_cmd(s_EuskalIRCServ, "JOIN #%s", CanalAyuda);
        send_cmd(s_EuskalIRCServ, "MODE #%s +ntm", CanalAyuda);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAyuda, s_EuskalIRCServ);
        send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_EuskalIRCServ);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_EuskalIRCServ);
        
    }
    if (!user || stricmp(user, s_BddServ) == 0) {
      
     
       #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_BddServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_BddServ);
      #endif

        CNICK(s_BddServ, desc_BddServ, "-", "-base.de.datos-");
	send_cmd(s_BddServ, "MODE %s +iXkoBrd", s_BddServ);
	send_cmd(s_BddServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_BddServ);
	
    }
    
    if (!user || stricmp(user, s_GlobalNoticer) == 0) {
      
     #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_GlobalNoticer);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_GlobalNoticer);
      #endif
	
	NICK(s_GlobalNoticer, desc_GlobalNoticer);
	send_cmd(s_GlobalNoticer, "MODE %s +ikorBd", s_GlobalNoticer);
	send_cmd(s_GlobalNoticer, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_GlobalNoticer);
    }
    if (!user || stricmp(user, s_ShadowServ) == 0) {
      
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
			s_ShadowServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
			s_ShadowServ);
      #endif

        CNICK(s_ShadowServ, desc_ShadowServ, "-", "-");
	send_cmd(s_ShadowServ, "MODE %s +rokbXBd", s_ShadowServ);
	send_cmd(s_ShadowServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_ShadowServ);
                              
    }
/* Esto, alg�n d�a funcionar�... o eso espero ;) */
    if (!user || stricmp(user, s_IpVirtual) == 0) {
       #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
				s_IpVirtual);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
				s_IpVirtual);
      #endif

    	CNICK(s_IpVirtual, desc_IpVirtual, "ipvirtual", ServerName);
 	send_cmd(s_IpVirtual, "MODE %s +rodkhBX", s_IpVirtual);
	send_cmd(s_IpVirtual, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_IpVirtual);
    }
 
    if (!user || stricmp(user, s_NewsServ) == 0) {
      
             #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
				s_NewsServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
				s_NewsServ);
      #endif
	
        NICK(s_NewsServ, desc_NewsServ);
        send_cmd(s_NewsServ, "MODE %s +ikorBd", s_NewsServ);
	send_cmd(s_NewsServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_NewsServ);
    	                        
    }
   	 if (!user || stricmp(user, s_JokuServ) == 0) {
	  
   	   #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
				s_JokuServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
				s_JokuServ);
      #endif

        NICK(s_JokuServ, desc_JokuServ);
        send_cmd(s_JokuServ, "MODE %s +ikrBd", s_JokuServ);
	send_cmd(s_JokuServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_JokuServ);
    	                        
    }
     if (!user || stricmp(user, s_GeoIP) == 0) {
      
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
				s_GeoIP);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
				s_GeoIP);
      #endif

        NICK(s_GeoIP, desc_GeoIP);
        send_cmd(s_GeoIP, "MODE %s +ikrBd", s_GeoIP);
	send_cmd(s_GeoIP, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_GeoIP);
    	                        
    }
    if (!user || stricmp(user,  s_SpamServ) == 0) {
      
    #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
				s_SpamServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
				s_SpamServ);
      #endif

      SPAM(s_SpamServ, desc_SpamServ);
        send_cmd( s_SpamServ, "MODE %s +iokrB",  s_SpamServ);
	send_cmd( s_SpamServ, "JOIN #%s", CanalAdmins);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_SpamServ);
    	
                        
    }
     if (!user || stricmp(user,  s_StatServ) == 0) {
      
      #if defined (IRC_SVSNICK)
      send_cmd(ServerName,
			"SVSNICK %s", 
				s_StatServ);
      #else
	send_cmd(ServerName,
			"RENAME %s", 
				s_StatServ);
      #endif
	
       NICK(s_StatServ, desc_StatServ);
        send_cmd( s_StatServ, "MODE %s +ikrBd",  s_StatServ);
	send_cmd( s_StatServ, "JOIN #%s", CanalAdmins);
	send_cmd( s_StatServ, "JOIN #%s", CanalOpers);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalAdmins, s_StatServ);
	send_cmd(s_ChanServ, "MODE #%s +o %s", CanalOpers, s_StatServ);
    	send_cmd(s_BddServ, "STATS b");
                        
    }
   
#endif
}

#undef NICK

/*************************************************************************/

/* Set GID if necessary.  Return 0 if successful (or if RUNGROUP not
 * defined), else print an error message to logfile and return -1.
 */

static int set_group(void)
{
#if defined(RUNGROUP) && defined(HAVE_SETGRENT)
    struct group *gr;

    setgrent();
    while ((gr = getgrent()) != NULL) {
	if (strcmp(gr->gr_name, RUNGROUP) == 0)
	    break;
    }
    endgrent();
    if (gr) {
	setgid(gr->gr_gid);
	return 0;
    } else {
	logeo("Grupo desconocido `%s'\n", RUNGROUP);
	return -1;
    }
#else
    return 0;
#endif
}

/*************************************************************************/

/* Parse command-line options for the "-dir" option only.  Return 0 if all
 * went well or -1 for a syntax error.
 */

/* XXX this could fail if we have "-some-option-taking-an-argument -dir" */

static int parse_dir_options(int ac, char **av)
{
    int i;
    char *s;

    for (i = 1; i < ac; i++) {
	s = av[i];
	if (*s == '-') {
	    s++;
	    if (strcmp(s, "dir") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-dir requiere un parametro\n");
		    return -1;
		}
		services_dir = av[i];
	    }
	}
    }
    return 0;
}

/*************************************************************************/

/* Parse command-line options.  Return 0 if all went well, -1 for an error
 * with an option, or 1 for -help.
 */

static int parse_options(int ac, char **av)
{
    int i;
    char *s, *t;

    for (i = 1; i < ac; i++) {
	s = av[i];
	if (*s == '-') {
	    s++;
	    if (strcmp(s, "remote") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-remote requiere hostname[:puerto]\n");
		    return -1;
		}
		s = av[i];
		t = strchr(s, ':');
		if (t) {
		    *t++ = 0;
		    if (atoi(t) > 0)
			RemotePort = atoi(t);
		    else {
			fprintf(stderr, "-remote: el numero de puerto debe ser positivo. Usando el puerto de por defecto.");
			return -1;
		    }
		}
		RemoteServer = s;
	    } else if (strcmp(s, "local") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-local requiere hostname o [hostname]:[puerto]\n");
		    return -1;
		}
		s = av[i];
		t = strchr(s, ':');
		if (t) {
		    *t++ = 0;
		    if (atoi(t) >= 0)
			LocalPort = atoi(t);
		    else {
			fprintf(stderr, "-local: el numero de puerto debe ser positivo o 0. Usando el puerto de por defecto.");
			return -1;
		    }
		}
		LocalHost = s;
	    } else if (strcmp(s, "name") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-name requiere un parametro\n");
		    return -1;
		}
		ServerName = av[i];
	    } else if (strcmp(s, "desc") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-desc requiere un parametro\n");
		    return -1;
		}
		ServerDesc = av[i];
	    } else if (strcmp(s, "user") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-user requiere a parametro\n");
		    return -1;
		}
		ServiceUser = av[i];
	    } else if (strcmp(s, "host") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-host requiere a parametro\n");
		    return -1;
		}
		ServiceHost = av[i];
	    } else if (strcmp(s, "dir") == 0) {
		/* Handled by parse_dir_options() */
		i++;  /* Skip parameter */
	    } else if (strcmp(s, "log") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-log requiere un parametro\n");
		    return -1;
		}
		log_filename = av[i];
	    } else if (strcmp(s, "update") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-update requiere un parametro\n");
		    return -1;
		}
		s = av[i];
		if (atoi(s) <= 0) {
		    fprintf(stderr, "-update: el numero de segundos debe ser positivo");
		    return -1;
		} else
		    UpdateTimeout = atol(s);
	    } else if (strcmp(s, "expire") == 0) {
		if (++i >= ac) {
		    fprintf(stderr, "-expire requiere un parametro\n");
		    return -1;
		}
		s = av[i];
		if (atoi(s) <= 0) {
		    fprintf(stderr, "-expire: el numero de segundos debe ser positivo");
		    return -1;
		} else
		    ExpireTimeout = atol(s);
	    } else if (strcmp(s, "debug") == 0) {
		debug++;
	    } else if (strcmp(s, "readonly") == 0) {
		readonly = 1;
		skeleton = 0;
	    } else if (strcmp(s, "skeleton") == 0) {
		readonly = 0;
		skeleton = 1;
	    } else if (strcmp(s, "nofork") == 0) {
		nofork = 1;
	    } else if (strcmp(s, "forceload") == 0) {
		forceload = 1;
	    } else {
		fprintf(stderr, "Opcion desconocida -%s\n", s);
		return -1;
	    }
	} else {
	    fprintf(stderr, "Argumentos de opcion no permitidos\n");
	    return -1;
	}
    }
    return 0;
}

/*************************************************************************/

/* Remove our PID file.  Done at exit. */

static void remove_pidfile(void)
{
    remove(PIDFilename);
}

/*************************************************************************/

/* Create our PID file and write the PID to it. */

static void write_pidfile(void)
{
    FILE *pidfile;

    pidfile = fopen(PIDFilename, "w");
    if (pidfile) {
	fprintf(pidfile, "%d\n", (int)getpid());
	fclose(pidfile);
	atexit(remove_pidfile);
    } else {
	log_perror("ATENCION: No puedo abrir el fichero PID %s", PIDFilename);
    }
}

/*************************************************************************/

/* Overall initialization routine.  Returns 0 on success, -1 on failure. */

int init(int ac, char **av)
{
    int i;
    int openlog_failed = 0, openlog_errno = 0;
    int started_from_term = isatty(0) && isatty(1) && isatty(2);

    /* Imported from main.c */
    extern void sighandler(int signum);


    /* Set file creation mask and group ID. */
#if defined(DEFUMASK) && HAVE_UMASK
    umask(DEFUMASK);
#endif
    if (set_group() < 0)
	return -1;
    
    /* Parse command line for -dir option. */
    parse_dir_options(ac, av);

    /* Chdir to Services data directory. */
    if (chdir(services_dir) < 0) {
	fprintf(stderr, "chdir(%s): %s\n", services_dir, strerror(errno));
	return -1;
    }

    /* Open logfile, and complain if we didn't. */
    if (open_log() < 0) {
	openlog_errno = errno;
	if (started_from_term) {
	    fprintf(stderr, "ATENCION: No puedo abrir el archivo de log %s: %s\n",
			log_filename, strerror(errno));
	} else {
	    openlog_failed = 1;
	}
    }

    /* Read configuration file; exit if there are problems. */
    if (!read_config())
	return -1;
RootHostold=RootHost;
AdminHostold=AdminHost;
CoAdminHostold=CoAdminHost;
DevelHostold=DevelHost;
OperHostold=OperHost;
PatrocinaHostold=PatrocinaHost;
s_NickServold=s_NickServ;
s_ChanServold=s_ChanServ;
s_MemoServold=s_MemoServ;
s_HelpServold=s_HelpServ;
s_OperServold=s_OperServ;
s_CregServold=s_CregServ;
s_SpamServold=s_SpamServ;
s_StatServold=s_StatServ;
s_EuskalIRCServold=s_EuskalIRCServ;
s_GlobalNoticerold=s_GlobalNoticer;
s_NewsServold=s_NewsServ;
s_IrcIIHelpold=s_IrcIIHelp;
s_mIRCHelpold=s_mIRCHelp;
s_BddServold=s_BddServ;
s_ShadowServold=s_ShadowServ;
s_IpVirtualold=s_IpVirtual;
s_GeoIPold=s_GeoIP;
s_JokuServold=s_JokuServ;
DEntryMsgold=DEntryMsg;
CregApoyosold=CregApoyos;
SpamUsersold=SpamUsers;
#if defined(REG_NICK_MAIL)
#if defined(SENDMAIL)
SendMailPatchold=SendMailPatch;
#endif
#if defined(SMTP)
ServerSMTPold=ServerSMTP;
PortSMTPold=PortSMTP;
#endif
NicksMailold=NicksMail;
SendFromold=SendFrom;
WebNetworkold=WebNetwork;
#endif
NSListMaxold=NSListMax;
int NSExpireoldval,NSExpireval;
if (NSExpire >= 86400)
		NSExpireval = (NSExpire/86400);
	if (NSExpireold >= 86400)
		NSExpireoldval = (NSExpireold/86400);
NSExpireold=NSExpire;

int NSRegMailoldval,NSRegMailval;
if (NSRegMail >= 86400)
		NSRegMailval = (NSRegMail/86400);
	if (NSRegMailold >= 86400)
		NSRegMailoldval = (NSRegMailold/86400);
NSRegMailold=NSRegMail;

NSSecureAdminsold=NSSecureAdmins;

int CSExpireoldval,CSExpireval;
CSMaxRegold=CSMaxReg;
if (CSExpire >= 86400)
		CSExpireval = (CSExpire/86400);
	if (CSExpireold >= 86400)
		CSExpireoldval = (CSExpireold/86400);
CSExpireold=CSExpire;
CSAccessMaxold=CSAccessMax;
CSAutokickMaxold=CSAutokickMax;
CSAutokickReasonold=CSAutokickReason;
CSListMaxold=CSListMax;
CanalAdminsold=CanalAdmins;
CanalOpersold=CanalOpers;
CanalCybersold=CanalCybers;
CanalAyudaold=CanalAyuda;
CanalSpamersold=CanalSpamers;
RootNumberold=RootNumber;
LogMaxUsersold=LogMaxUsers;


    /* Parse all remaining command-line options. */
    parse_options(ac, av);

    /* Detach ourselves if requested. */
    if (!nofork) {
	if ((i = fork()) < 0) {
	    perror("fork()");
	    return -1;
	} else if (i != 0) {
	    exit(0);
	}
	if (started_from_term) {
	    close(0);
	    close(1);
	    close(2);
	}
	if (setpgid(0, 0) < 0) {
	    perror("setpgid()");
	    return -1;
	}
    }

    /* Write our PID to the PID file. */
    write_pidfile();

    /* Announce ourselves to the logfile. */
    if (debug || readonly || skeleton) {
	logeo("euskalirc-services-bdd %s (compilados para %s) iniciados (opciones:%s%s%s)",
		version_number, version_protocol,
		debug ? " debug" : "", readonly ? " readonly" : "",
		skeleton ? " skeleton" : "");
    } else {
	logeo("euskalirc-services-bdd %s compilados para %s iniciados.",
		version_number, version_protocol);
    }
    start_time = time(NULL);

    /* If in read-only mode, close the logfile again. */
    if (readonly)
	close_log();

    /* Set signal handlers.  Catch certain signals to let us do things or
     * panic as necessary, and ignore all others.
     */
#if defined(NSIG)
    for (i = 1; i <= NSIG; i++)
#else
    for (i = 1; i <= 32; i++)
#endif
	signal(i, SIG_IGN);

    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGBUS, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGHUP, sighandler);
    signal(SIGILL, sighandler);
    signal(SIGTRAP, sighandler);
#if defined(SIGIOT)
    signal(SIGIOT, sighandler);
#endif
    signal(SIGFPE, sighandler);

    signal(SIGUSR1, sighandler);  /* This is our "out-of-memory" panic switch */

    /* Initialize multi-language support */
    lang_init();
    if (debug)
	logeo("debug: Cargando lenguajes");

    /* Initialiize subservices */
    ns_init();
    cs_init();
#if defined(CREGSERV)
    cr_init();
#endif    
    ms_init();
    os_init();
    load_spam();
    load_ipv();   /*leyendo datos de frases*/
    /* Load up databases */
 
    if (!skeleton) {
	load_ns_dbase();
	if (debug)
	    logeo("debug: Cargando la DB de %s (1/7)", s_NickServ);
	load_cs_dbase();
	if (debug)
	    logeo("debug: Cargando la DB de %s (2/7)", s_ChanServ);
	    load_cr_dbase();
	if (debug)
	    logeo("debug: Cargando la DB de %s (3/7)", s_CregServ);
    }
	
    load_os_dbase();
load_jok_dbase();
    if (debug)
	logeo("debug: Cargando la DB de %s (4/7)", s_OperServ);
    load_akill();
     /*load_aregistra();*/
	
    if (debug)
	logeo("debug: Cargando la DB de GLINES (5/7)");
    load_news();
    if (debug)
	logeo("debug: Cargando la DB de NOTICIAS (6/7)");
   
    if (debug)
	logeo("debug: Cargando la DB de SPAM (7/7)");
    load_cr_dbase();
    load_ipv();
    load_achanakick();
    load_X_dbase();
    logeo("Cargadas las bases de datos");

    /* Connect to the remote server */
    servsock = conn(RemoteServer, RemotePort, LocalHost, LocalPort);
    if (servsock < 0)
	fatal_perror("No puedo conectar al servidor");
    send_cmd(NULL, "PASS :%s", RemotePassword);
#if defined(IRC_UNDERNET_P09)
    send_cmd(NULL, "SERVER %s 1 %lu %lu P09 :%s",
             ServerName, start_time, start_time, ServerDesc);
#else /* IRC_UNDERNET_P10 */
    send_cmd(NULL, "SERVER %s %d 0 %ld J10 %cD] :%s",
             ServerName, 2, start_time, convert2y[ServerNumerico], ServerDesc); 
#endif
    sgets2(inbuf, sizeof(inbuf), servsock);
    if (strnicmp(inbuf, "ERROR", 5) == 0) {
	/* Close server socket first to stop wallops, since the other
	 * server doesn't want to listen to us anyway */
	disconn(servsock);
	servsock = -1;
	fatal("El servidor ha devuelto: %s", inbuf);
    }


    /* Announce a logfile error if there was one */
    if (openlog_failed) {
	canalopers(NULL, "4ATENCION: No puedo abrir el fichero de log: 12%s",
		strerror(openlog_errno));
    }

    /* Bring in our pseudo-clients */
    introduce_user(NULL);


    send_cmd(ServerName, "SETTIME %lu", time(NULL));
/*#if HAVE_ALLWILD_NOTICE
    send_cmd(s_OperServ, "NOTICE $*.%s :Establecidos los servicios de la RED.", NETWORK_DOMAIN);
    
#else
# ifdef NETWORK_DOMAIN
    send_cmd(s_OperServ, "NOTICE $*.%s :Establecidos los servicios de la RED.", NETWORK_DOMAIN);
# else
    Go through all common top-level domains.  If you have others,
     add them here.
     
    send_cmd(s_OperServ, "NOTICE $*.es :Establecidos los servicios de la RED.");
    send_cmd(s_OperServ, "NOTICE $*.com :Establecidos los servicios de la RED.");
    send_cmd(s_OperServ, "NOTICE $*.net :Establecidos los servicios de la RED.");
    send_cmd(s_OperServ, "NOTICE $*.org :Establecidos los servicios de la RED.");    
    send_cmd(s_OperServ, "NOTICE $*.edu :Establecidos los servicios de la RED.");
    send_cmd(s_OperServ, "NOTICE $*.cat :Establecidos los servicios de la RED.");
    send_cmd(s_OperServ, "NOTICE $*.eus :Establecidos los servicios de la RED.");
    send_cmd(s_OperServ, "NOTICE $*.tk :Establecidos los servicios de la RED.");
 #endif
 #endif*/
        
    join_chanserv();
     join_jokuserv(); 

    /* Success! */
    return 0;
}

/*************************************************************************/
