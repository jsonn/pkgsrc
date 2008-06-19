# $NetBSD: Makefile,v 1.23 2008/06/19 14:44:06 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-postgresql-${RUBY_DBI_VERSION}
PKGREVISION=	2
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/postgres for ruby

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

DISTINFO_FILE?=	${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR?=	${.CURDIR}/../ruby-DBI/patches
USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_pg
INSTALLATION_DIRS=	${RUBY_DOC}/dbi/dbd-postgresql \
			${RUBY_EG}/dbi/dbd-postgresql

DOCS=		README
EXAMPLES=	examples/sqlsh.rb.new

post-install:
.for f in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/doc/dbd/postgresql/${f} \
		${DESTDIR}${PREFIX}/${RUBY_DOC}/dbi/dbd-postgresql
.endfor
.for f in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${f} \
		${DESTDIR}${PREFIX}/${RUBY_EG}/dbi/dbd-postgresql
.endfor

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/pgsql.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
