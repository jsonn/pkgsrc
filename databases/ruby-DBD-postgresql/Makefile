# $NetBSD: Makefile,v 1.1.1.1 2001/07/08 17:28:53 mjl Exp $

PKGNAME=	${RUBY_PKGNAMEPREFIX}DBD-postgresql-${VERSION}
COMMENT=	DBD/postgres for ruby

do-configure:
	cd ${WRKSRC}; ${RUBY} setup.rb config --with=dbd_pg

DEPENDS+=	ruby-postgresql-*:../../databases/ruby-postgresql
DEPENDS+=	ruby-DBI-*:../../databases/ruby-DBI	

.include "../../databases/ruby-DBI/Makefile.common"
