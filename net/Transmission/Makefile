# $NetBSD: Makefile,v 1.3 2007/05/27 09:16:37 tnn Exp $
#

DISTNAME=		Transmission-0.72
CATEGORIES=		net
MASTER_SITES=		http://download.m0k.org/transmission/files/

MAINTAINER=		tnn@NetBSD.org
HOMEPAGE=		http://transmission.m0k.org/
COMMENT=		Free, lightweight BitTorrent client

USE_TOOLS+=		gmake msgfmt pkg-config
USE_DIRS+=		xdg-1.1

#--disable-openssl --disable-gtk
GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--verbose

post-extract:
	${MV} "${WRKDIR}/Transmission .72" ${WRKSRC}

.include "../../x11/gtk2/buildlink3.mk"
.include "../../security/openssl/buildlink3.mk"
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
