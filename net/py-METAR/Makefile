# $NetBSD: Makefile,v 1.2 2003/06/02 01:17:46 jschauma Exp $
#

DISTNAME=	pymetar-0.4
PKGNAME=	${PYPKGPREFIX}-pymetar-0.4
CATEGORIES=	net
MASTER_SITES=	http://unattached.i-no.de/pkgs/

MAINTAINER=	tech-pkg@netbsd.org
HOMEPAGE=	http://unattached.i-no.de/software-pymetar.shtml
COMMENT=	downloads weather reports

PYDISTUTILSPKG=	yes
PYTHON_PATCH_SCRIPTS=	bin/pymet${PYVERSSUFFIX}
PLIST_SUBST+=	PYVERSSUFFIX=${PYVERSSUFFIX}

post-extract:
	${MV} ${WRKSRC}/bin/pymet ${WRKSRC}/bin/pymet${PYVERSSUFFIX}

.include "../../lang/python/extension.mk"
.include "../../lang/python/application.mk"
.include "../../mk/bsd.pkg.mk"
