# $NetBSD: Makefile,v 1.17 2009/12/16 21:22:02 drochner Exp $
#

DISTNAME=	pymetar-0.15
PKGNAME=	${PYPKGPREFIX}-pymetar-0.15
CATEGORIES=	net python
MASTER_SITES=	http://www.schwarzvogel.de/pkgs/

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	http://www.schwarzvogel.de/software-pymetar.shtml
COMMENT=	Downloads weather reports

PKG_DESTDIR_SUPPORT=	user-destdir

PYDISTUTILSPKG=	yes
PLIST_SUBST+=	PYVERSSUFFIX=${PYVERSSUFFIX:Q}

post-extract:
	${MV} ${WRKSRC}/bin/weather.py ${WRKSRC}/bin/weather-${PYVERSSUFFIX}.py

.include "../../lang/python/extension.mk"
.include "../../lang/python/application.mk"
.include "../../mk/bsd.pkg.mk"
