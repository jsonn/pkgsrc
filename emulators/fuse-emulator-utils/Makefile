# $NetBSD: Makefile,v 1.3 2017/07/04 09:40:12 adam Exp $

DISTNAME=	fuse-utils-1.3.2
PKGNAME=	${DISTNAME:S/fuse/fuse-emulator/}
CATEGORIES=	emulators
MASTER_SITES=	${MASTER_SITE_SOURCEFORGE:=fuse-emulator/}

MAINTAINER=	adam@NetBSD.org
HOMEPAGE=	http://fuse-emulator.sourceforge.net/
COMMENT=	ZX Spectrum Emulator utilities
LICENSE=	gnu-gpl-v2

USE_LANGUAGES=		c c++
USE_TOOLS+=		pkg-config
GNU_CONFIGURE=		yes

.include "../../emulators/libspectrum/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
