# $NetBSD: Makefile,v 1.2 2005/01/28 14:08:01 taca Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-sqlite-${RUBY_DBI_VERSION}
PKGREVISION=	1
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/SQLite for ruby

DISTINFO_FILE=  	${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR=		${.CURDIR}/../ruby-DBI/patches
RUBY_HAS_ARCHLIB=	yes
USE_BUILDLINK3=		yes
USE_RUBY_SETUP=		yes
CONFIGURE_ARGS=		--with=dbd_sqlite

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

.include "../ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../databases/sqlite/buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
