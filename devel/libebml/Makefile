# $NetBSD: Makefile,v 1.1.1.2 2004/04/07 10:40:01 salo Exp $
#

DISTNAME=	libebml-0.6.5
CATEGORIES=	devel
MASTER_SITES=	http://dl.matroska.org/downloads/libebml/

MAINTAINER=	shattered@users.sourceforge.net
HOMEPAGE=	http://www.sourceforge.net/projects/ebml/
COMMENT=	Extensible Binary Meta Language support library

USE_BUILDLINK3=	yes
USE_GNU_TOOLS+=	make

NO_CONFIGURE=	yes
MAKEFLAGS+=	prefix="${PREFIX}"
BUILD_DIRS=	${WRKDIR}/${DISTNAME}/make/linux

.include "../../mk/bsd.pkg.mk"
