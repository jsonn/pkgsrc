# $NetBSD: Makefile,v 1.8 2004/09/16 21:32:32 he Exp $
#

DISTNAME=	pymetar-0.4
PKGNAME=	${PYPKGPREFIX}-pymetar-0.4
CATEGORIES=	net python
MASTER_SITES=	http://www.schwarzvogel.de/pkgs/

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	http://www.schwarzvogel.de/software-pymetar.shtml
COMMENT=	Downloads weather reports

PYDISTUTILSPKG=	yes
PYTHON_PATCH_SCRIPTS=	bin/pymet${PYVERSSUFFIX}
PLIST_SUBST+=	PYVERSSUFFIX=${PYVERSSUFFIX}
USE_BUILDLINK3=	yes

post-extract:
	${MV} ${WRKSRC}/bin/pymet ${WRKSRC}/bin/pymet${PYVERSSUFFIX}

.include "../../lang/python/extension.mk"
.include "../../lang/python/application.mk"
.include "../../mk/bsd.pkg.mk"
