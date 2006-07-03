# $NetBSD: Makefile,v 1.1.1.1 2006/07/03 08:40:13 dogcow Exp $
#

DISTNAME=		Transmission-0.6.1
CATEGORIES=		net
MASTER_SITES=		http://download.m0k.org/transmission/files/

MAINTAINER=		dogcow@NetBSD.org
HOMEPAGE=		http://transmission.m0k.org/
COMMENT=		Free, lightweight BitTorrent client

USE_TOOLS+=		gmake

#--disable-openssl --disable-gtk
GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--verbose

.include "../../x11/gtk2/buildlink3.mk"
.include "../../security/openssl/buildlink3.mk"
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
