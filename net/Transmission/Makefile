# $NetBSD: Makefile,v 1.50 2009/08/26 19:58:46 sno Exp $
#
DISTNAME=		transmission-1.73
PKGNAME=		Transmission-1.73
PKGREVISION=		2
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

.include "../../mk/bsd.prefs.mk"
.if !empty(OPSYS:M*BSD)
CPPFLAGS+=		-DBSD
.endif

# Workaround for PR lib/40714, fixed since NetBSD-5.99.16
.if !empty(MACHINE_PLATFORM:MNetBSD-[1-5].*)
SUBST_CLASSES+=		tg
SUBST_STAGE.tg=		pre-configure
SUBST_MESSAGE.tg=	removing thousands grouping flag from %f (PR lib/40714)
SUBST_FILES.tg=		daemon/remote.c gtk/util.c libtransmission/utils.c \
			  qt/utils.cc */*.po
SUBST_SED.tg=		-e "s,%\\([.0-9]*\\)'\\([.0-9]*\\)f,%\\1\\2f,g"
.endif

PLIST_SRC+=		${PKGDIR}/PLIST.common

.include "options.mk"

# Transmission bundles it's own libevent (newer version than we have in pkgsrc.)
#BUILDLINK_API_DEPENDS.libevent+=	libevent>=1.4.12
#.include "../../devel/libevent/buildlink3.mk"

ALL_ENV+=		"OPENSSL_CFLAGS=-I${SSLBASE}/include"
ALL_ENV+=		"OPENSSL_LIBS=-L${SSLBASE}/lib -lcrypto -lssl"
.include "../../devel/libgetopt/buildlink3.mk"
.include "../../security/openssl/buildlink3.mk"
.include "../../www/curl/buildlink3.mk"
.include "../../mk/pthread.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
