# $NetBSD: Makefile,v 1.124 2017/02/12 06:25:18 ryoon Exp $

DISTNAME=		vice-2.4
PKGREVISION=		19
CATEGORIES=		emulators
MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=vice-emu/}
MASTER_SITES+=		ftp://ftp.zimmers.net/pub/cbm/crossplatform/emulators/VICE/
MASTER_SITES+=		http://www.zimmers.net/anonftp/pub/cbm/crossplatform/emulators/VICE/

MAINTAINER=		rhialto@falu.nl
HOMEPAGE=		http://vice-emu.sourceforge.net/
COMMENT=		Emulator for C64, C128, CBM-II, PET, VIC20, Plus4 and C16
LICENSE=		gnu-gpl-v2

BUILD_DEPENDS+=		tex-pdftex-[0-9]*:../../print/tex-pdftex

RESTRICTED=		ROM image copyright is questionable.
NO_BIN_ON_CDROM=	${RESTRICTED}
NO_SRC_ON_CDROM=	${RESTRICTED}
NO_BIN_ON_FTP=		${RESTRICTED}
NO_SRC_ON_FTP=		${RESTRICTED}

EXTRACT_ONLY=		${DISTNAME}${EXTRACT_SUFX}

USE_PKGLOCALEDIR=	YES
USE_TOOLS+=		bdftopcf gmake msgfmt
USE_TOOLS+=		pkg-config
USE_LANGUAGES=		c c++
GNU_CONFIGURE=		YES
CONFIGURE_ENV+=		toolchain_check=no

INFO_FILES=		YES
FONTS_DIRS.x11=		${PREFIX}/share/vice/fonts
REQD_DIRS+=		${PREFIX}/share/vice/fonts
REQD_DIRS+=		${PREFIX}/share/vice

PLIST_SRC+=		PLIST

.include "options.mk"

CONFIGURE_ARGS.NetBSD+=	--without-oss
CPPFLAGS.NetBSD+=	-DHAVE_STDINT_H
CPPFLAGS.DragonFly+=	-DHAVE_STDINT_H

.include "../../mk/readline.buildlink3.mk"
.include "../../devel/gettext-lib/buildlink3.mk"
.include "../../devel/zlib/buildlink3.mk"
.include "../../graphics/png/buildlink3.mk"
.include "../../graphics/giflib/buildlink3.mk"
.include "../../mk/jpeg.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
