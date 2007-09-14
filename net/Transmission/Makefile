# $NetBSD: Makefile,v 1.12 2007/09/14 13:21:14 tnn Exp $
#

DISTNAME=		transmission-0.82
PKGNAME=		Transmission-0.82
CATEGORIES=		net
MASTER_SITES=		http://download.m0k.org/transmission/files/

MAINTAINER=		tnn@NetBSD.org
HOMEPAGE=		http://transmission.m0k.org/
COMMENT=		Free, lightweight BitTorrent client

PKG_DESTDIR_SUPPORT=	user-destdir
USE_TOOLS+=		gmake

GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--verbose
LDFLAGS+=		${BUILDLINK_LDADD.getopt}

.include "options.mk"


# It's amusing how they manage to package this in a different way with
# every release.
EXTRACT_DIR=		${WRKSRC}

#post-extract:
#	${MV} "${WRKDIR}/transmission-0.80" ${WRKSRC}

BUILDLINK_API_DEPENDS.libevent+=	libevent>=1.2

.include "../../devel/libevent/buildlink3.mk"
.include "../../devel/libgetopt/buildlink3.mk"
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
