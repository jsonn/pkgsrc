# $NetBSD: Makefile,v 1.4 2017/07/07 09:48:07 fhajny Exp $

DISTNAME=	lager-3.5.1
CATEGORIES=	sysutils
MASTER_SITES=	${MASTER_SITE_GITHUB:=erlang-lager/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	http://github.com/basho/lager/
COMMENT=	Logging framework for Erlang/OTP
LICENSE=	apache-2.0

DEPENDS+=	erlang-goldrush>=0.1.9:../../sysutils/erlang-goldrush

GITHUB_PROJECT=	lager
GITHUB_TAG=	${PKGVERSION_NOREV}

.include "../../lang/erlang/module.mk"
.include "../../mk/bsd.pkg.mk"
