# $NetBSD: Makefile,v 1.1.1.2 2009/02/07 17:22:29 abs Exp $
#

DISTNAME=	ee-1.4.6.src
PKGNAME=	${DISTNAME:S:.src::}
CATEGORIES=	editors
MASTER_SITES=	http://mahon.cwx.net/sources/
EXTRACT_SUFX=	.tgz

MAINTAINER=	abs@NetBSD.org
HOMEPAGE=	http://mahon.cwx.net/sources/
COMMENT=	Easy to use text editor with onscreen instructions

WRKSRC=	${WRKDIR}/easyedit-1.4.6

PKG_DESTDIR_SUPPORT= user-destdir
MAKE_JOBS_SAFE= no

do-install:
	${INSTALL_PROGRAM_DIR} ${DESTDIR}${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKSRC}/ee ${DESTDIR}${PREFIX}/bin
	${INSTALL_DATA_DIR} ${DESTDIR}${PREFIX}/${PKGMANDIR}/man1
	${INSTALL_MAN} ${WRKSRC}/ee.1 ${DESTDIR}${PREFIX}/${PKGMANDIR}/man1

.include "../../mk/curses.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
