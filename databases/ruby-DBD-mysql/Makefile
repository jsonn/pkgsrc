# $NetBSD: Makefile,v 1.14 2006/08/17 13:07:45 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-mysql-${RUBY_DBI_VERSION}
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/mysql for ruby

DISTINFO_FILE=  ${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR=	${.CURDIR}/../ruby-DBI/patches
USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_mysql

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

DOCS=		HISTORY README

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/dbd-mysql
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/doc/dbd/mysql/${FILE} \
		${RUBY_DOCDIR}/dbi/dbd-mysql
.endfor

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/mysql.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
