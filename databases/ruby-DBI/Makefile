# $NetBSD: Makefile,v 1.1.1.1 2001/07/08 17:26:57 mjl Exp $

PKGNAME=	${RUBY_PKGNAMEPREFIX}DBI-${VERSION}
COMMENT=	DBI for ruby

do-configure:
	cd ${WRKSRC}; ${RUBY} setup.rb config --with=dbi

.include "../../databases/ruby-DBI/Makefile.common"
