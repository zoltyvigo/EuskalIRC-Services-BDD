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
 
static void gline_spam(const char *user, char *motivo);

 void antispamc(const char *source,const char *chan, char *buf)
 {
 	if (!strstr(buf, "5757")) {
		return;
	} else {
		privmsg(s_AntiSpam, chan, "Ey! %s dijo 5757!!", source);
	}
 }

void antispam(const char *source, char *buf)
{
	if (strstr(buf, "\1")) {
		gline_spam(source, "Ataques a canales");
	}
	return;
}

static void gline_spam(const char *user, char *motivo)
{
	User *u2;
	u2 = finduser(user);

	privmsg(s_AntiSpam, user, "Tu ereh mal0, ahora mismo te glineo a *@%s, y te digo que %s",u2->host, motivo);
}
