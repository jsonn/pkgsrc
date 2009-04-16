# $NetBSD: Makefile,v 1.44 2009/04/16 17:28:52 tnn Exp $
#
DISTNAME=		transmission-1.52
PKGNAME=		Transmission-1.52
CATEGORIES=		net
MASTER_SITES=		http://download.m0k.org/transmission/files/
EXTRACT_SUFX=		.tar.bz2

MAINTAINER=		tnn@NetBSD.org
HOMEPAGE=		http://www.transmissionbt.com/
COMMENT=		Free, lightweight BitTorrent client

PKG_DESTDIR_SUPPORT=	user-destdir
USE_LIBTOOL=		yes
USE_TOOLS+=		gmake pkg-config intltool msgfmt
EXTRACT_USING=		gtar

GNU_CONFIGURE=		yes
LDFLAGS+=		${BUILDLINK_LDADD.getopt} ${BUILDLINK_LDADD.gettext}
# Tell libtool to link all internal libraries statically
LDFLAGS+=		-static

CONFIGURE_ARGS+=	--disable-wx

SUBST_CLASSES+=			fix-paths
SUBST_STAGE.fix-paths=		pre-configure
SUBST_MESSAGE.fix-paths=	Fixing absolute paths.
SUBST_FILES.fix-paths=		libtransmission/platform.c
SUBST_SED.fix-paths=		-e 's,"/usr/local/share:/usr/share,"${PREFIX}/share/,g'

.include "../../mk/bsd.prefs.mk"
.if !empty(OPSYS:M*BSD)
CPPFLAGS+=		-DBSD
.endif

.if ${OPSYS} == "NetBSD"
SUBST_CLASSES+=		tg
SUBST_STAGE.tg=		pre-configure
SUBST_MESSAGE.tg=	removing thousands grouping flag from %f (PR lib/40714)
SUBST_FILES.tg=		daemon/remote.c gtk/util.c */*.po
SUBST_SED.tg=		-e "s,%\([.0-9]*\)'\([.0-9]*\)f,%\1\2f,g"
.endif

PLIST_SRC+=		${PKGDIR}/PLIST.common

.include "options.mk"

# Transmission bundles it's own libevent (version 1.3.99-trunk)
#BUILDLINK_API_DEPENDS.libevent+=	libevent>=1.2
#.include "../../devel/libevent/buildlink3.mk"

CONFIGURE_ENV+=		"OPENSSL_CFLAGS=-I${SSLBASE}/include"
CONFIGURE_ENV+=		"OPENSSL_LIBS=-L${SSLBASE}/lib -lcrypto -lssl"
.include "../../devel/libgetopt/buildlink3.mk"
.include "../../security/openssl/buildlink3.mk"
.include "../../www/curl/buildlink3.mk"
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
