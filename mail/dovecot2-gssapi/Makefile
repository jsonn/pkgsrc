# $NetBSD: Makefile,v 1.3 2016/12/12 14:22:03 wiz Exp $
#

.include "../../mail/dovecot2/Makefile.plugin"

PKGNAME=		${DISTNAME:S/dovecot/dovecot-gssapi/}
COMMENT+=		(GSSAPI plugin)

CONFIGURE_ARGS+=	--with-gssapi=plugin

INSTALLATION_DIRS+=	lib/dovecot/auth

do-install:
	cd ${WRKSRC} && ${LIBTOOL} --mode=install ${INSTALL_LIB} 	\
		src/auth/libmech_gssapi.la 				\
		${DESTDIR}${PREFIX}/lib/dovecot/auth

.include "../../mk/krb5.buildlink3.mk"
.include "../../mk/bsd.pkg.mk"
