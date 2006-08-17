# $NetBSD: Makefile,v 1.15 2006/08/17 13:05:08 taca Exp $

DISTNAME=	${RUBY_DBI_DISTNAME}
PKGNAME=	${RUBY_PKGPREFIX}-DBI-${RUBY_DBI_VERSION}
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

OWN_DIRS+=	${RUBY_SITELIBDIR}/DBD ${RUBY_SITEARCHLIBDIR}/DBD

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/html
.for file in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${file} ${RUBY_DOCDIR}/dbi/${file}
.endfor
.for file in ${DOCS_DBI}
	${INSTALL_DATA} ${WRKSRC}/doc/${file} ${RUBY_DOCDIR}/dbi/${file}
.endfor
	${INSTALL_DATA_DIR} ${RUBY_EXAMPLESDIR}/dbi
.for file in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${file} ${RUBY_EXAMPLESDIR}/dbi
.endfor

.include "Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/bsd.pkg.mk"
