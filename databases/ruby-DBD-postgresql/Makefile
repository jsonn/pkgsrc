# $NetBSD: Makefile,v 1.3 2002/01/19 11:30:29 taca Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGNAMEPREFIX}DBD-postgresql-${RUBY_DBI_VERSION}
CATEGORIES=	databases
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	packages@netbsd.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/postgres for ruby

CONFIGURE_ARGS=	--with=dbd_pg
USE_RUBY_SETUP=	# defined

DEPENDS+=	${RUBY_PKGNAMEPREFIX}postgresql>=0.6.4:../../databases/ruby-postgresql
DEPENDS+=	${RUBY_PKGNAMEPREFIX}DBI>=0.0.6:../../databases/ruby-DBI

DOCS=		README
EXAMPLES=	examples/sqlsh.rb.new

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/dbd-postgresql
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/lib/dbd_pg/doc/${FILE} \
		${RUBY_DOCDIR}/dbi/dbd-postgresql
.endfor
	${INSTALL_DATA_DIR} ${RUBY_EXAMPLESDIR}/dbi/dbd-postgresql
.for FILE in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${FILE} \
		${RUBY_EXAMPLESDIR}/dbi/dbd-postgresql
.endfor

.include "../ruby-DBI/Makefile.common"
