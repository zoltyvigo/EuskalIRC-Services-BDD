/* Version information for Services.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define BUILD	"833"

const char version_number[] = "4.3.3";
const char version_build[] =
	"build #" BUILD ", compilado " __DATE__ " " __TIME__;
const char version_protocol[] =
#if defined(IRC_BAHAMUT)
	"ircd.dal Bahamut"
#elif defined(IRC_DAL4_4_15)
        "ircd.dal 4.4.15+"
#elif defined(IRC_DALNET)
	"ircd.dal 4.4.13-"
#elif defined(DB_NETWORKS)
	"ircu 2.10.x P10 con soporte BDD"
#elif defined(IRC_UNDERNET_P10)
	"ircu 2.10.x P10"
#elif defined(IRC_UNDERNET_P09)
	"ircu 2.10.01 a 2.10.07 P09"
#elif defined(IRC_UNDERNET)
	"ircu 2.9.32-"
#elif defined(IRC_TS8)
	"RFC1459 + TS8"
#elif defined(IRC_CLASSIC)
	"RFC1459"
#else
	"desconocido"
#endif
	;
