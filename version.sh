#!/bin/sh
#
# Increment Services build number

VERSION=4.3.3

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

const char version_number[] = "$VERSION";
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
EOF
