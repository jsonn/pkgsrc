# $NetBSD: Makefile,v 1.6 2004/05/11 04:01:39 snj Exp $
#

DISTNAME=	pymetar-0.4
PKGNAME=	${PYPKGPREFIX}-pymetar-0.4
CATEGORIES=	net
MASTER_SITES=	http://unattached.i-no.de/pkgs/

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	http://unattached.i-no.de/software-pymetar.shtml
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
