# $NetBSD: Makefile,v 1.11 2006/02/17 17:54:38 wiz Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGPREFIX}-DBI-${RUBY_DBI_VERSION}
PKGREVISION=	2
CATEGORIES=	databases ruby
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	tech-pkg@NetBSD.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBI for ruby

USE_RUBY_SETUP=		yes
CONFIGURE_ARGS+=	--with=dbi

REPLACE_RUBY=	test/testdbi.rb
DOCS=		ChangeLog LICENSE
DOCS_DBI=	DBD_SPEC DBI_SPEC ToDo index.rd sf_logo.html \
		html/ChangeLog.html html/DBD_SPEC.html html/DBI_SPEC.html \
		html/ToDo.html html/index.html html/rubyStyle.css
EXAMPLES=	examples/persistence.rb
TESTS=		README testdbi.rb example/config.sh example/example.cfg \
		example/setup.sh example/teardown.sh

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/html
.for file in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${file} ${RUBY_DOCDIR}/dbi/${file}
.endfor
.for file in ${DOCS_DBI}
	${INSTALL_DATA} ${WRKSRC}/doc/${file} ${RUBY_DOCDIR}/dbi/${file}
.endfor
	${INSTALL_DATA_DIR} ${RUBY_EXAMPLESDIR}/dbi/test/example
.for file in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${file} ${RUBY_EXAMPLESDIR}/dbi
.endfor
.for file in ${TESTS}
	${INSTALL_DATA} ${WRKSRC}/test/${file} \
		${RUBY_EXAMPLESDIR}/dbi/test/${file}
.endfor
	${INSTALL_DATA_DIR} ${RUBY_SITELIBDIR}/DBD
	${INSTALL_DATA_DIR} ${RUBY_SITEARCHLIBDIR}/DBD

.include "Makefile.common"
.include "../../lang/ruby/modules.mk"
.include "../../mk/bsd.pkg.mk"
