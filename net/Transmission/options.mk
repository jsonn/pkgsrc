# $NetBSD: options.mk,v 1.6 2008/02/18 13:19:15 tnn Exp $

.include "../../mk/bsd.prefs.mk"

PKG_OPTIONS_VAR=	PKG_OPTIONS.Transmission
PKG_SUPPORTED_OPTIONS=	gtk # wxwidgets # wx currently doesn't build.
PKG_SUGGESTED_OPTIONS=	gtk

.include "../../mk/bsd.options.mk"

.if !empty(PKG_OPTIONS:Mgtk)
. include "../../x11/gtk2/buildlink3.mk"
. include "../../graphics/hicolor-icon-theme/buildlink3.mk"
CONFIGURE_ARGS+=	--with-gtk
USE_DIRS+=		xdg-1.1
PLIST_SRC+=		${PKGDIR}/PLIST.gtk
.else
CONFIGURE_ARGS+=	--without-gtk
pre-configure:
	touch ${WRKSRC}/po/Makefile
.endif

.if !empty(PKG_OPTIONS:Mwxwidgets)
. include "../../x11/wxGTK/buildlink3.mk"
CONFIGURE_ARGS+=	--with-wx
USE_LANGUAGES+=		c c++
.  if empty(PKG_OPTIONS:Mgtk)
PKG_FAIL_REASON+=	"The wxwidgets option needs the gtk option."
.  endif
.else
CONFIGURE_ARGS+=	--without-wx
.endif
