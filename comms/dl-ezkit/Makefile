# $NetBSD: Makefile,v 1.1.1.2 2001/07/15 20:10:52 gmcgarry Exp $

PKGNAME=        dl-ezkit-0.8.1
DISTNAME=	dl-ezkit-0.8.1-bsd
CATEGORIES=	comms
MASTER_SITES=	http://www.ww.tu-freiberg.de/~pberg/download/Unix-sources/

MAINTAINER=	gmcgarry@netbsd
HOMEPAGE=	http://www.ww.tu-freiberg.de/~pberg/download/
COMMENT=	coff file downloader for the EZ-KIT Lite using the RS232

WRKSRC=		${WRKDIR}/dl-ezkit
MAKEFILE=	Makefile.bsd

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/dl ${PREFIX}/bin/

.include "../../mk/bsd.pkg.mk"

