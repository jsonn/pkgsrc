# $NetBSD: buildlink2.mk,v 1.2 2004/03/29 05:05:42 jlam Exp $

.include "../../lang/python/pyversion.mk"

BUILDLINK_PACKAGES+=		pymetar
BUILDLINK_PKGBASE.pymetar?=	${PYPKGPREFIX}-pymetar
BUILDLINK_DEPENDS.pymetar?=	${PYPKGPREFIX}-pymetar-[0-9]*
BUILDLINK_PKGSRCDIR.pymetar?=	../../net/py-METAR
