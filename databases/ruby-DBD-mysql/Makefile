# $NetBSD: Makefile,v 1.2 2003/09/19 10:11:56 wiz Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGNAMEPREFIX}DBD-mysql-${RUBY_DBI_VERSION}
CATEGORIES=	databases
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBD/mysql for ruby

DISTINFO_FILE=  ${.CURDIR}/../ruby-DBI/distinfo
PATCHDIR=	${.CURDIR}/../ruby-DBI/patches
CONFIGURE_ARGS=	--with=dbd_mysql
USE_RUBY_SETUP=	# defined

DEPENDS+=	${RUBY_PKGNAMEPREFIX}DBI>=${RUBY_DBI_VERSION}:../../databases/ruby-DBI

DOCS=		HISTORY README

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/dbd-mysql
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/lib/dbd_mysql/doc/${FILE} \
		${RUBY_DOCDIR}/dbi/dbd-mysql
.endfor

.include "../../databases/mysql-client/buildlink2.mk"
.include "../ruby-DBI/Makefile.common"
