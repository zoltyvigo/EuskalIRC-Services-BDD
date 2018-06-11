#!/bin/sh
#
# Increment Services build number

VERSION=0.3
BRANCHSTATUS=BETA-RELEASE

# Cambiar este siempre
# Las versiones seran 1.a.b




if [ -f version.h ] ; then
	BUILD=`fgrep '#define BUILD' version.h | sed 's/^#define BUILD.*"\([0-9]*\)".*$/\1/'`
	BUILD=`expr $BUILD + 1 2>/dev/null`
else
	BUILD=1
fi
if [ ! "$BUILD" ] ; then
	BUILD=1
fi
cat >version.h <<EOF
/* Version information for Services.
 *
 * Services is copyright (c) 1996-1999 Andy Church.
 *     E-mail: <achurch@dragonfire.net>
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define BUILD	"$BUILD"

const char version_branchstatus[] = "$BRANCHSTATUS";
const char version_number[] = "$VERSION";
const char version_upworld[] = "$VERSION_UPWORLD";
const char version_build[] =
	"build #" BUILD ", compilado " __DATE__ " " __TIME__;
const char version_protocol[] =
#if defined(SERVICES_BDD)
	"u2.10.H.10.x Soporte P9"
#elif (IRC_UNDERNET_P09)
        "ircu 2.10.01 - 2.10.07 Soporte P9"
#elif defined(SERVICES_BDD)
	"ircu 2.10.x Soporte P10"
#elif defined(IRC_UNDERNET_P10)
	"ircu 2.10.x Soporte P10"
#elif defined(IRC_UNDERNET)
        "ircu 2.9.3 o inferior"
#else
        "desconocido"
#endif
#ifdef SERVICES_BDD
	"con soporte para ircuh m�s modernos"
#elif IRC_HISPANO
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
 * a�adir mas lineas
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
        "por Toni Garc�a - zoltan <zolty@zolty.net>",
        "-",
        "Por petici�n popular, estos bots han sido liberados bajo la",
        "licencia GNU-GPL. Puedes bajar la �ltima version de estos bots",
        "o m�s informacion en la siguiente web:",
        "http://irc.zolty.net/upworld.html",
	"-",
	"---------------------------------------------------------------",
	"-",
	"Esta es una modificaci�n de los Upworld de Toni Garc�a creada",
	"para interactuar con la Base de Datos Distribuida del IRCd de",
	"la red iRC-Hispano. Modificaci�n esctrita por David Mart�n D�az",
	"[x] - <equis@fuckmicrosoft.com>",
	"---------------------------------------------------------------",
	"Es una variaci�n de los 4botshispanobdd-1.2.1 conocido como Proyecto",
	"4euskalirc-services-bdd 12,15http://euskalirc.wordpress.com",
	"por 2donostiarra - admin.euskalirc@gmail.com 12,15http://www.euskalirc.tk",
        0,
    };
EOF
