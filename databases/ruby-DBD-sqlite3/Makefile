# $NetBSD: Makefile,v 1.1.1.1 2008/06/22 15:47:15 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-sqlite3-${RUBY_DBI_VERSION}
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/SQLite for ruby

USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_sqlite3

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../databases/sqlite3/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
