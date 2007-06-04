# $NetBSD: Makefile,v 1.1.1.2 2007/06/04 08:49:18 rillig Exp $

DISTNAME=	Class-Accessor-Chained-0.01
PKGNAME=	p5-${DISTNAME}
CATEGORIES=	devel perl5
MASTER_SITES=	${MASTER_SITE_PERL_CPAN:=Class/}

MAINTAINER=	kuli0020@umn.edu
HOMEPAGE=	http://search.cpan.org/dist/Class-Accessor-Chained/
COMMENT=	Make chained accessors

DEPENDS+=	p5-Class-Accessor>=0:../../devel/p5-Class-Accessor
BUILD_DEPENDS+=	{perl>=5.7.3,p5-Test-Simple>=0}:../../devel/p5-Test-Simple

PERL5_MODULE_TYPE=	Module::Build

PERL5_PACKLIST= auto/Class/Accessor/Chained/.packlist

.include "../../lang/perl5/module.mk"
.include "../../mk/bsd.pkg.mk"
