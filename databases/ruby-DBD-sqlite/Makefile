# $NetBSD: Makefile,v 1.10 2006/08/17 13:07:45 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-sqlite-${RUBY_DBI_VERSION}
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/SQLite for ruby

DISTINFO_FILE=  	${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR=		${.CURDIR}/../ruby-DBI/patches
RUBY_HAS_ARCHLIB=	yes
USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_sqlite

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../databases/sqlite/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
