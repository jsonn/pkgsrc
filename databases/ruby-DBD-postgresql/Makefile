# $NetBSD: Makefile,v 1.25 2008/10/16 21:43:54 erh Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-postgresql-${RUBY_DBI_VERSION}
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/postgres for ruby

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI
DEPENDS+= ${RUBY_PKGPREFIX}-pg>=0.7.9:../../databases/ruby-pg

USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_pg
INSTALLATION_DIRS=	${RUBY_EG}/dbi/dbd-postgresql

BUILD_DEFS+=    RUBY_DBI_VERSION

DOCS=		README
EXAMPLES=	examples/sqlsh.rb.new

post-install:
.for f in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${f} \
		${DESTDIR}${PREFIX}/${RUBY_EG}/dbi/dbd-postgresql
.endfor

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/pgsql.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
