# $NetBSD: Makefile,v 1.16 2008/06/19 14:44:06 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBD-mysql-${RUBY_DBI_VERSION}
PKGREVISION=	1
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/mysql for ruby

DEPENDS+= ${RUBY_PKGPREFIX}-DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

DISTINFO_FILE=	${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR=	${.CURDIR}/../ruby-DBI/patches
USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbd_mysql
INSTALLATION_DIRS=	${RUBY_DOC}/dbi/dbd-mysql

DOCS=		HISTORY README

post-install:
.for f in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/doc/dbd/mysql/${f} \
		${DESTDIR}${PREFIX}/${RUBY_DOC}/dbi/dbd-mysql
.endfor

.include "../../databases/ruby-DBI/Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/mysql.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
