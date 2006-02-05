# $NetBSD: Makefile,v 1.13 2006/02/05 23:10:29 joerg Exp $
#

DISTNAME=	pymetar-0.12
PKGNAME=	${PYPKGPREFIX}-pymetar-0.12
PKGREVISION=	1
CATEGORIES=	net python
MASTER_SITES=	http://www.schwarzvogel.de/pkgs/

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	http://www.schwarzvogel.de/software-pymetar.shtml
COMMENT=	Downloads weather reports

PYDISTUTILSPKG=	yes
PLIST_SUBST+=	PYVERSSUFFIX=${PYVERSSUFFIX:Q}
PYTHON_VERSIONS_ACCEPTED=	23 24

post-extract:
	${MV} ${WRKSRC}/bin/weather.py ${WRKSRC}/bin/weather-${PYVERSSUFFIX}.py

.include "../../lang/python/extension.mk"
.include "../../lang/python/application.mk"
.include "../../mk/bsd.pkg.mk"
