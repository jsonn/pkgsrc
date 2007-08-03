# $NetBSD: options.mk,v 1.1 2007/08/03 22:34:23 tnn Exp $

.include "../../mk/bsd.prefs.mk"

PKG_OPTIONS_VAR=	PKG_OPTIONS.Transmission
PKG_SUPPORTED_OPTIONS=	gtk ssl
PKG_SUGGESTED_OPTIONS=	gtk ssl

.include "../../mk/bsd.options.mk"

.if !empty(PKG_OPTIONS:Mssl)
.	include "../../security/openssl/buildlink3.mk"
.else
CONFIGURE_ARGS+=	--disable-openssl
.endif

.if !empty(PKG_OPTIONS:Mgtk)
.	include "../../x11/gtk2/buildlink3.mk"
USE_TOOLS+=		msgfmt pkg-config
USE_DIRS+=		xdg-1.1
PLIST_SUBST+=		GTK=
.else
CONFIGURE_ARGS+=        --disable-gtk
PLIST_SUBST+=		GTK="@comment "
.endif
