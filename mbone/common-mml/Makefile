# $NetBSD: Makefile,v 1.1.1.2 2006/04/11 17:57:08 minskim Exp $

DISTNAME=		common-${VERSION}
PKGNAME=		common-mml-${VERSION}
CATEGORIES=	 	mbone
MASTER_SITES=		http://www-mice.cs.ucl.ac.uk/multimedia/software/common/

MAINTAINER=		psham@cs.nmsu.edu
HOMEPAGE=		http://www-mice.cs.ucl.ac.uk/multimedia/software/common/
COMMENT=		Common multimedia library required by mbone tools

VERSION=		1.2.16

WRKSRC=			${WRKDIR}/common
GNU_CONFIGURE=		yes
USE_LIBTOOL=		yes

INCDIR=			${PREFIX}/include/common-mml
DOCDIR=			${PREFIX}/share/doc/common-mml

do-install:
	${INSTALL_DATA_DIR} ${INCDIR} ${DOCDIR}
	${INSTALL_DATA} ${WRKSRC}/src/*.h ${INCDIR}
	${INSTALL_DATA} ${WRKSRC}/src/*.a ${PREFIX}/lib
	${INSTALL_DATA} ${WRKSRC}/doc/html/*.html ${DOCDIR}

.include "../../mk/bsd.pkg.mk"
