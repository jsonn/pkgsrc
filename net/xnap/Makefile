# $NetBSD: Makefile,v 1.1.1.2 2002/06/19 08:58:13 agc Exp $

DISTNAME=	xnap-2.0-pre3
PKGNAME=	xnap-2.0rc3
CATEGORIES=	net
MASTER_SITES=	${MASTER_SITE_SOURCEFORGE:=xnap/}
EXTRACT_SUFX=	.tgz

MAINTAINER=	dolemite@wuli.nu
HOMEPAGE=	http://xnap.sourceforge.net/
COMMENT=	XNap is a powerful Java file sharing client

USE_JAVA=	YES

PLIST_SUBST+=	BINNAME=${BINNAME}
WRKSRC=		${WRKDIR}

BINNAME=	xnap
DOCDIR=		${PREFIX}/share/doc/${BINNAME}
LIBDIR=		${PREFIX}/lib

.include "../../mk/bsd.prefs.mk"

# "jdk" and "kaffe" packages don't work.
.if ${MACHINE_ARCH} == i386
PKG_JVM=	sun-jdk
.elif ${MACHINE_ARCH} == powerpc
PKG_JVM=	blackdown-jdk13
.endif

.if ${PKG_JVM} == blackdown-jdk13
JVM_OPTS+=	-green -jar
.else
JVM_OPTS=	-jar
.endif

post-extract:
	${CHMOD} -R g-w,o-w ${WRKSRC}/*

do-build:
	${ECHO} "#!/bin/sh" >${WRKSRC}/${BINNAME}
	${ECHO} "PATH=${JAVA_HOME}/bin:\$${PATH}; export PATH" >>${WRKSRC}/${BINNAME}
.if defined(CLASSPATH)
	${ECHO} "CLASSPATH=${CLASSPATH}:\$${CLASSPATH}; export CLASSPATH" >>${WRKSRC}/${BINNAME}
.endif
	${ECHO} "exec java ${JVM_OPTS} ${LIBDIR}/${BINNAME}.jar" >>${WRKSRC}/${BINNAME}

do-install:
	${INSTALL_SCRIPT} ${WRKSRC}/${BINNAME} ${PREFIX}/bin
	${INSTALL_DATA} ${WRKSRC}/${DISTNAME}/${BINNAME}.jar ${PREFIX}/lib
	${INSTALL_DATA_DIR} ${DOCDIR} ${LIBDIR}

.include "../../mk/bsd.pkg.mk"
