# $NetBSD: Makefile,v 1.19 2006/08/17 13:07:45 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-postgresql-${RUBY_DBI_VERSION}
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/postgres for ruby

DISTINFO_FILE?=	${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR?=	${.CURDIR}/../ruby-DBI/patches
USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_pg

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

DOCS=		README
EXAMPLES=	examples/sqlsh.rb.new

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/dbd-postgresql
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/doc/dbd/postgresql/${FILE} \
		${RUBY_DOCDIR}/dbi/dbd-postgresql
.endfor
	${INSTALL_DATA_DIR} ${RUBY_EXAMPLESDIR}/dbi/dbd-postgresql
.for FILE in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${FILE} \
		${RUBY_EXAMPLESDIR}/dbi/dbd-postgresql
.endfor

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/pgsql.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
