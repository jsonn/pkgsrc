# $NetBSD: Makefile,v 1.14 2005/03/06 17:19:05 taca Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-postgresql-${RUBY_DBI_VERSION}
PKGREVISION=	1
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/postgres for ruby

DISTINFO_FILE?=	${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR?=	${.CURDIR}/../ruby-DBI/patches
USE_BUILDLINK3=	yes
USE_RUBY_SETUP=	yes
CONFIGURE_ARGS=	--with=dbd_pg

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

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
.include "../../lang/ruby/modules.mk"
.include "../../mk/pgsql.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
