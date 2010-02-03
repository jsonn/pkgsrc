# $NetBSD: Makefile,v 1.1.1.1 2010/02/03 23:38:50 wiz Exp $
#

DISTNAME=	wxMaxima-0.8.4
PKGREVISION=	1
CATEGORIES=	math
MASTER_SITES=	http://downloads.sourceforge.net/project/wxmaxima/wxMaxima/0.8.4/

MAINTAINER=	helgoman@users.sourceforge.net
HOMEPAGE=	http://wxmaxima.sourceforge.net/wiki/index.php/Main_Page
COMMENT=	Graphical UI for maxima
LICENSE=	gnu-gpl-v2

DEPENDS+=	maxima>=5.15.0:../../math/maxima
DEPENDS+=	wxGTK28>=2.8.0:../../x11/wxGTK28
DEPENDS+=	libxml2>=2.7.3:../../textproc/libxml2
DEPENDS+=	gnuplot>=4.2.3:../../graphics/gnuplot

PKG_DESTDIR_SUPPORT=	user-destdir

GNU_CONFIGURE=		yes
USE_PKGLOCALEDIR=	yes
USE_LANGUAGES=		c c++

.include "../../x11/wxGTK28/buildlink3.mk"
.include "../../textproc/libxml2/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
