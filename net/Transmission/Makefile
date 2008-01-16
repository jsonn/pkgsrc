# $NetBSD: Makefile,v 1.21 2008/01/16 04:23:10 tnn Exp $
#
DISTNAME=		transmission-1.01
PKGNAME=		Transmission-1.01
CATEGORIES=		net
MASTER_SITES=		http://download.m0k.org/transmission/files/
EXTRACT_SUFX=		.tar.bz2

MAINTAINER=		tnn@NetBSD.org
HOMEPAGE=		http://www.transmissionbt.com/
COMMENT=		Free, lightweight BitTorrent client

PKG_DESTDIR_SUPPORT=	user-destdir
USE_TOOLS+=		gmake pkg-config intltool msgfmt
EXTRACT_USING=		gtar

GNU_CONFIGURE=		yes
LDFLAGS+=		${BUILDLINK_LDADD.getopt}

.include "../../mk/bsd.prefs.mk"
.if !empty(OPSYS:M*BSD)
CPPFLAGS+=		-DBSD
.endif

.include "options.mk"

# Transmission bundles it's own libevent (version 1.3.99-trunk)
#BUILDLINK_API_DEPENDS.libevent+=	libevent>=1.2
#.include "../../devel/libevent/buildlink3.mk"

CONFIGURE_ENV+=		"OPENSSL_CFLAGS=-I${SSLBASE}/include"
CONFIGURE_ENV+=		"OPENSSL_LIBS=-L${SSLBASE}/lib -lcrypto -lssl"
.include "../../security/openssl/buildlink3.mk"
.include "../../devel/libgetopt/buildlink3.mk"
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
