/* Version information for Services.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define BUILD	"17"

const char version_branchstatus[] = "BETA-RELEASE";
const char version_number[] = "4.3.3";
const char version_upworld[] = "1.1-release1";
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

/* Look folks, please leave this INFO reply intact and unchanged. If you do
 * have the urge to metion yourself, please simply add your name to the list.
 * The other people listed below have just as much right, if not more, to be
 * mentioned. Leave everything else untouched. Thanks.
 */
/* Lo dicho antes, NO TOCAR NI CAMBIAR LOS CREDITOS, si quieres, puedes
 * añadir mas lineas
 */

const char *info_text[] =
    {
        "IRC Services developed by and copyright (c) 1996-2001",
        "Andrew Church <achurch@achurch.org>.",
        "Parts copyright (c) 1999-2000 Andrew Kempe and others.",
        "IRC Services may be freely redistributed under the GNU",
        "General Public License.",
        "-",
        "For the more information and a list of distribution sites,",
        "please visit: http://www.ircservices.za.net/",
        "-",
        "Estos bots han sido reprogramados y adaptados originalmente",
        "para la red Upworld.Org, usando la base de iRCServices 4.3.3",
        "por Toni García - zoltan <zolty@zolty.net>",
        "-",
        "Por petición popular, estos bots han sido liberados bajo la",
        "licencia GNU-GPL. Puedes bajar la última version de estos bots",
        "o más informacion en la siguiente web:",
        "http://irc.zolty.net/upworld.html",
	"-",
	"---------------------------------------------------------------",
	"-",
	"Esta es una modificación de los Upworld de Toni García creada",
	"para interactuar con la Base de Datos Distribuida del IRCd de",
	"la red iRC-Hispano. Modificación esctrita por David Martín Díaz",
	"[x] - <equis@fuckmicrosoft.com>",
        0,
    };
