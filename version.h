/* Version information for Services.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define BUILD	"903"

const char version_number[] = "4.3.3";
const char version_build[] =
	"build #" BUILD ", compilado " __DATE__ " " __TIME__;
const char version_protocol[] =
#if defined(IRC_UNDERNET_P09)
        "ircu 2.10.01 - 2.10.07 Soporte P9"
#elif defined(IRC_UNDERNET_P10)
	"ircu 2.10.x Soporte P10"
#elif defined(IRC_UNDERNET)
        "ircu 2.9.3 o inferior"
#else
        "desconocido"
#endif
#ifdef IRC_HISPANO
	"con soporte para ESNET-Hispano"
#elif defined (IRC_TERRA)
	"con soporte para TerraIrcu"
#endif
	;
