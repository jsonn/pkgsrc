# $NetBSD: Makefile,v 1.2 2017/06/26 14:29:31 youri Exp $
#

DISTNAME=	vte-0.38.4
CATEGORIES=	x11
MASTER_SITES=	${MASTER_SITE_GNOME:=sources/vte/0.38/}
EXTRACT_SUFX=	.tar.xz

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	http://www.gnome.org/
COMMENT=	Terminal widget with improved accessibility and I18N support
LICENSE=	gnu-lgpl-v2

PKGCONFIG_OVERRIDE+=	vte.pc.in

USE_LIBTOOL=		yes
GNU_CONFIGURE=		yes
USE_TOOLS+=		gmake intltool msgfmt pkg-config bash

CONFIGURE_ARGS+=	--disable-python
CONFIGURE_ARGS+=	--enable-Bsymbolic=auto
CONFIGURE_ARGS+=	--disable-introspection
CONFIGURE_ARGS+=	--disable-vala

.include "../../x11/gtk3/buildlink3.mk"
.include "../../mk/termcap.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
