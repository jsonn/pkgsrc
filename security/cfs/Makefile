# $NetBSD: Makefile,v 1.1.1.2 2001/06/05 22:19:11 jlam Exp $

DISTNAME=		cfs-1.4.1
CATEGORIES=		security
MASTER_SITES=		http://www.crypto.com/software/

MAINTAINER=		jlam@netbsd.org
COMMENT=		Cryptographic File System

ONLY_FOR_PLATFORMS=	NetBSD-*

ALL_TARGET=		cfs
INSTALL_TARGET=		install_cfs

# Required settings necessary to build CFS on NetBSD.  This is copied from
# ${WRKSRC}/Makefile.
#
CFLAGS+=		-traditional	# uses strange preprocessor behaviour
CFLAGS+=		-DPROTOTYPES=1
CFLAGS+=		-DBSD44 -DANYPORT -DCFS_PORT=2049 -DSHORTLINKS
MAKE_ENV+=		MAKE=${MAKE_PROGRAM}
MAKE_ENV+=		COMPAT="-lcompat"
MAKE_ENV+=		RPCOPTS="-b"

CFS_BUILD_SCRIPT=	${WRKSRC}/netbsd_make_with_bad_rpcgen
DOCDIR=			${PREFIX}/share/doc/cfs

# Avoid conflicts with coda-client's cpasswd by consistently renaming
# all references to c* to cfs_*.
#
SRC_SUBST=		cattach->cfs_attach	CATTACH->CFS_ATTACH
SRC_SUBST+=		cdetach->cfs_detach	CDETACH->CFS_DETACH
SRC_SUBST+=		cmkdir->cfs_mkdir	CMKDIR->CFS_MKDIR
SRC_SUBST+=		cpasswd->cfs_passwd	CPASSWD->CFS_PASSWD
SRC_SUBST+=		cname->cfs_name		CNAME->CFS_NAME
SRC_SUBST+=		ccat->cfs_cat		CCAT->CFS_CAT
SRC_SUBST+=		cmkkey->cfs_mkkey	CMKKEY->CFS_MKKEY

# Use getpass() instead of own getpassword() function, which doesn't seem to
# hide the password when the user is typing it.
#
SRC_SUBST+=		getpassword->getpass
CFLAGS+=		-DHAVE_GETPASS

SRC_SUBST_SED=		${SRC_SUBST:S/->/!/:S/$/!g/:S/^/ -e s!/}

post-extract:
	@${RM} -f ${WRKSRC}/esm.1
	@cd ${WRKSRC}; for file in *; do				\
		${MV} -f $${file} $${file}.orig;			\
		dest=`${ECHO} $${file} | ${SED} ${SRC_SUBST_SED}`;	\
        	${SED} ${SRC_SUBST_SED} $${file}.orig > $${dest};	\
		${RM} -f $${file}.orig;					\
	done

post-patch:
	${CHMOD} +x ${CFS_BUILD_SCRIPT}

do-build:
	cd ${WRKSRC}; ${SETENV} ${MAKE_ENV}				\
		${SH} ${CFS_BUILD_SCRIPT} ${ALL_TARGET}

pre-install:
	${SED}	-e "s|@PREFIX@|${PREFIX}|g"				\
		${FILESDIR}/cfsd.sh > ${WRKDIR}/cfsd.sh

post-install:
	${INSTALL_SCRIPT} ${WRKDIR}/cfsd.sh ${PREFIX}/etc/rc.d/cfsd
	cd ${WRKSRC}; for file in *.1; do				\
		${INSTALL_MAN} $${file} ${PREFIX}/man/man1/$${file};	\
	done
	cd ${WRKSRC}; for file in *.8; do				\
		${INSTALL_MAN} $${file} ${PREFIX}/man/man8/$${file};	\
	done
	${INSTALL_DATA_DIR} ${DOCDIR}
	cd ${WRKSRC}; for file in README.netbsd notes.ms; do		\
		${INSTALL_DATA} $${file} ${DOCDIR}/$${file};		\
	done

.include "../../mk/bsd.pkg.mk"
