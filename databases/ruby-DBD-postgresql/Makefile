# $NetBSD: Makefile,v 1.7 2003/09/10 16:21:00 taca Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGNAMEPREFIX}DBD-postgresql-${RUBY_DBI_VERSION}
CATEGORIES=	databases
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/postgres for ruby

DISTINFO_FILE=  ${.CURDIR}/../ruby-DBI/distinfo
CONFIGURE_ARGS=	--with=dbd_pg
USE_RUBY_SETUP=	# defined

DEPENDS+= ${RUBY_PKGNAMEPREFIX}DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

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

.include "../postgresql-lib/buildlink2.mk"
.include "../ruby-DBI/Makefile.common"
