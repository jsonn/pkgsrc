# $NetBSD: Makefile,v 1.1.1.2 2002/01/28 01:05:04 jlam Exp $

DISTNAME=	perl-ldap-0.25
PKGNAME=	p5-Net-LDAP-0.25
CATEGORIES=	net perl5
MASTER_SITES=	${MASTER_SITE_SOURCEFORGE:=perl-ldap/} \
		${MASTER_SITE_PERL_CPAN:=Net/}

MAINTAINER=	packages@netbsd.org
HOMEPAGE=	http://perl-ldap.sourceforge.net/
COMMENT=	perl5 modules providing OO access to LDAP servers

DEPENDS+=	p5-Convert-ASN1-[0-9]*:../../textproc/p5-Convert-ASN1
DEPENDS+=	p5-Digest-MD5-[0-9]*:../../security/p5-Digest-MD5
DEPENDS+=	p5-IO-Socket-SSL-[0-9]*:../../security/p5-IO-Socket-SSL
DEPENDS+=	p5-URI>=1.08:../../www/p5-URI
DEPENDS+=	p5-XML-Parser-[0-9]*:../../textproc/p5-XML-Parser

USE_BUILDLINK_ONLY=	YES
PERL5_PACKLIST=		${PERL5_SITEARCH}/auto/perl-ldap/.packlist

do-configure:
	@cd ${WRKSRC}; ${SETENV} ${MAKE_ENV} ${PERL5} Makefile.PL

.include "../../lang/perl5/buildlink.mk"
.include "../../mk/bsd.pkg.mk"
