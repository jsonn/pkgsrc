# $NetBSD: Makefile,v 1.16 2008/06/19 14:43:56 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBI-${RUBY_DBI_VERSION}
PKGREVISION=	1
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBI for ruby

USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbi

DOCS=		ChangeLog LICENSE
DOCS_DBI=	DBD_SPEC DBI_SPEC \
		html/ChangeLog.html html/DBD_SPEC.html html/DBI_SPEC.html \
		html/index.html html/rubyStyle.css
EXAMPLES=	examples/persistence.rb

OWN_DIRS+=	${RUBY_VENDORLIB}/DBD ${RUBY_VENDORARCHLIB}/DBD

INSTALLATION_DIRS=	${RUBY_DOC}/dbi/html ${RUBY_EG}/dbi

post-install:
.for f in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${f} ${DESTDIR}${PREFIX}/${RUBY_DOC}/dbi/${f}
.endfor
.for f in ${DOCS_DBI}
	${INSTALL_DATA} ${WRKSRC}/doc/${f} ${DESTDIR}${PREFIX}/${RUBY_DOC}/dbi/${f}
.endfor
.for f in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${f} ${DESTDIR}${PREFIX}/${RUBY_EG}/dbi
.endfor

.include "Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/bsd.pkg.mk"
