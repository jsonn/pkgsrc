# $NetBSD: Makefile,v 1.2 2001/07/30 16:08:36 taca Exp $

DISTNAME=	ruby-dbi-all-${RUBY_DBI_VERSION}
PKGNAME=	${RUBY_PKGNAMEPREFIX}DBI-${RUBY_DBI_VERSION}
CATEGORIES=	databases
MASTER_SITES=	${RUBY_DBI_MASTER_SITES}

MAINTAINER=	packages@netbsd.org
HOMEPAGE=	${RUBY_DBI_HOMEPAGE}
COMMENT=	DBI for ruby

CONFIGURE_ARGS=	--with=dbi
USE_RUBY_SETUP=	# defined

REPLACE_RUBY=	test/testdbi.rb
DOCS=		index.rd html/index.html html/rubyStyle.css
DOCS_DBI=	ChangeLog DBD_SPEC DBI_SPEC ToDo html/ChangeLog.html \
		html/DBD_SPEC.html html/DBI_SPEC.html html/ToDo.html
EXAMPLES=	examples/persistence.rb
TESTS=		README testdbi.rb example/config.sh example/example.cfg \
		example/setup.sh example/teardown.sh

post-install:
	${INSTALL_DATA_DIR} ${RUBY_DOCDIR}/dbi/dbi
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/doc/${FILE} ${RUBY_DOCDIR}/dbi
.endfor
.for FILE in ${DOCS_DBI}
	${INSTALL_DATA} ${WRKSRC}/lib/dbi/doc/${FILE} ${RUBY_DOCDIR}/dbi/dbi
.endfor
	${INSTALL_DATA_DIR} ${RUBY_EXAMPLESDIR}/dbi/test/example
.for FILE in ${EXAMPLES}
	${INSTALL_DATA} ${WRKSRC}/${FILE} ${RUBY_EXAMPLESDIR}/dbi
.endfor
.for FILE in ${TESTS}
	${INSTALL_DATA} ${WRKSRC}/test/${FILE} \
		${RUBY_EXAMPLESDIR}/dbi/test/${FILE}
.endfor
	${INSTALL_DATA_DIR} ${RUBY_SITELIBDIR}/DBD

.include "Makefile.common"
