# $NetBSD: buildlink2.mk,v 1.1.1.1 2002/12/03 19:34:59 drochner Exp $

.include "../../lang/python/pyversion.mk"

BUILDLINK_PACKAGES+=		pymetar
BUILDLINK_DEPENDS.pymetar?=	${PYPKGPREFIX}-pymetar-*
BUILDLINK_PKGSRCDIR.pymetar?=	../../net/py-METAR
