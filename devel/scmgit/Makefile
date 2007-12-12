# $NetBSD: Makefile,v 1.1.1.2 2007/12/12 04:05:27 bjs Exp $
#

.include "Makefile.common"

DISTNAME=		# empty
PKGNAME=		scmgit-${GIT_VERSION}
CATEGORIES+=		meta-pkgs
COMMENT=		GIT Tree History Storage Tool (meta-package)

DISTFILES=		# empty

DEPENDS+=		scmgit-base-[0-9]*:../../devel/scmgit-base
DEPENDS+=		scmgit-docs-[0-9]*:../../devel/scmgit-docs

AUTO_MKDIRS=	no
NO_CHECKSUM=	yes
NO_CONFIGURE=	yes
NO_BUILD=	yes
PLIST_SRC=	# empty (meta-pkg)

do-patch:

do-install:

.include "../../mk/bsd.pkg.mk"
