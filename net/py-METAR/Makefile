# $NetBSD: Makefile,v 1.12 2005/12/05 20:50:48 rillig Exp $
#

DISTNAME=	pymetar-0.12
PKGNAME=	${PYPKGPREFIX}-pymetar-0.12
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
