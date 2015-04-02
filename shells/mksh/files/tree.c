/*	$OpenBSD: tree.c,v 1.20 2012/06/27 07:17:19 otto Exp $	*/

/*-
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 *		 2011, 2012, 2013
 *	Thorsten Glaser <tg@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 */

#include "sh.h"

__RCSID("$MirOS: src/bin/mksh/tree.c,v 1.72 2013/09/24 20:19:45 tg Exp $");

#define INDENT	8

static void ptree(struct op *, int, struct shf *);
static void pioact(struct shf *, struct ioword *);
static const char *wdvarput(struct shf *, const char *, int, int);
static void vfptreef(struct shf *, int, const char *, va_list);
static struct ioword **iocopy(struct ioword **, Area *);
static void iofree(struct ioword **, Area *);

/* "foo& ; bar" and "foo |& ; bar" are invalid */
static bool prevent_semicolon;

static const char Telif_pT[] = "elif %T";

/*
 * print a command tree
 */
static void
ptree(struct op *t, int indent, struct shf *shf)
{
	const char **w;
	struct ioword **ioact;
	struct op *t1;
	int i;

 Chain:
	if (t == NULL)
		return;
	switch (t->type) {
	case TCOM:
		prevent_semicolon = false;
		/*
		 * special-case 'var=<<EOF' (rough; see
		 * exec.c:execute() for full code)
		 */
		if (
		    /* we have zero arguments, i.e. no programme to run */
		    t->args[0] == NULL &&
		    /* we have exactly one variable assignment */
		    t->vars[0] != NULL && t->vars[1] == NULL &&
		    /* we have exactly one I/O redirection */
		    t->ioact != NULL && t->ioact[0] != NULL &&
		    t->ioact[1] == NULL &&
		    /* of type "here document" (or "here string") */
		    (t->ioact[0]->flag & IOTYPE) == IOHERE) {
			fptreef(shf, indent, "%S", t->vars[0]);
			break;
		}

		if (t->vars) {
			w = (const char **)t->vars;
			while (*w)
				fptreef(shf, indent, "%S ", *w++);
		} else
			shf_puts("#no-vars# ", shf);
		if (t->args) {
			w = t->args;
			while (*w)
				fptreef(shf, indent, "%S ", *w++);
		} else
			shf_puts("#no-args# ", shf);
		break;
	case TEXEC:
		t = t->left;
		goto Chain;
	case TPAREN:
		fptreef(shf, indent + 2, "( %T) ", t->left);
		break;
	case TPIPE:
		fptreef(shf, indent, "%T| ", t->left);
		t = t->right;
		goto Chain;
	case TLIST:
		fptreef(shf, indent, "%T%;", t->left);
		t = t->right;
		goto Chain;
	case TOR:
	case TAND:
		fptreef(shf, indent, "%T%s %T",
		    t->left, (t->type == TOR) ? "||" : "&&", t->right);
		break;
	case TBANG:
		shf_puts("! ", shf);
		prevent_semicolon = false;
		t = t->right;
		goto Chain;
	case TDBRACKET:
		w = t->args;
		shf_puts("[[", shf);
		while (*w)
			fptreef(shf, indent, " %S", *w++);
		shf_puts(" ]] ", shf);
		break;
	case TSELECT:
	case TFOR:
		fptreef(shf, indent, "%s %s ",
		    (t->type == TFOR) ? "for" : Tselect, t->str);
		if (t->vars != NULL) {
			shf_puts("in ", shf);
			w = (const char **)t->vars;
			while (*w)
				fptreef(shf, indent, "%S ", *w++);
			fptreef(shf, indent, "%;");
		}
		fptreef(shf, indent + INDENT, "do%N%T", t->left);
		fptreef(shf, indent, "%;done ");
		break;
	case TCASE:
		fptreef(shf, indent, "case %S in", t->str);
		for (t1 = t->left; t1 != NULL; t1 = t1->right) {
			fptreef(shf, indent, "%N(");
			w = (const char **)t1->vars;
			while (*w) {
				fptreef(shf, indent, "%S%c", *w,
				    (w[1] != NULL) ? '|' : ')');
				++w;
			}
			fptreef(shf, indent + INDENT, "%N%T%N;%c", t1->left,
			    t1->u.charflag);
		}
		fptreef(shf, indent, "%Nesac ");
		break;
#ifdef DEBUG
	case TELIF:
		internal_errorf("TELIF in tree.c:ptree() unexpected");
		/* FALLTHROUGH */
#endif
	case TIF:
		i = 2;
		t1 = t;
		goto process_TIF;
		do {
			t1 = t1->right;
			i = 0;
			fptreef(shf, indent, "%;");
 process_TIF:
			/* 5 == strlen("elif ") */
			fptreef(shf, indent + 5 - i, Telif_pT + i, t1->left);
			t1 = t1->right;
			if (t1->left != NULL) {
				fptreef(shf, indent, "%;");
				fptreef(shf, indent + INDENT, "%s%N%T",
				    "then", t1->left);
			}
		} while (t1->right && t1->right->type == TELIF);
		if (t1->right != NULL) {
			fptreef(shf, indent, "%;");
			fptreef(shf, indent + INDENT, "%s%N%T",
			    "else", t1->right);
		}
		fptreef(shf, indent, "%;fi ");
		break;
	case TWHILE:
	case TUNTIL:
		/* 6 == strlen("while "/"until ") */
		fptreef(shf, indent + 6, "%s %T",
		    (t->type == TWHILE) ? "while" : "until",
		    t->left);
		fptreef(shf, indent, "%;");
		fptreef(shf, indent + INDENT, "do%N%T", t->right);
		fptreef(shf, indent, "%;done ");
		break;
	case TBRACE:
		fptreef(shf, indent + INDENT, "{%N%T", t->left);
		fptreef(shf, indent, "%;} ");
		break;
	case TCOPROC:
		fptreef(shf, indent, "%T|& ", t->left);
		prevent_semicolon = true;
		break;
	case TASYNC:
		fptreef(shf, indent, "%T& ", t->left);
		prevent_semicolon = true;
		break;
	case TFUNCT:
		fpFUNCTf(shf, indent, tobool(t->u.ksh_func), t->str, t->left);
		break;
	case TTIME:
		fptreef(shf, indent, "%s %T", "time", t->left);
		break;
	default:
		shf_puts("<botch>", shf);
		prevent_semicolon = false;
		break;
	}
	if ((ioact = t->ioact) != NULL) {
		bool need_nl = false;

		while (*ioact != NULL)
			pioact(shf, *ioact++);
		/* Print here documents after everything else... */
		ioact = t->ioact;
		while (*ioact != NULL) {
			struct ioword *iop = *ioact++;

			/* heredoc is NULL when tracing (set -x) */
			if ((iop->flag & (IOTYPE | IOHERESTR)) == IOHERE &&
			    iop->heredoc) {
				shf_putc('\n', shf);
				shf_puts(iop->heredoc, shf);
				fptreef(shf, indent, "%s",
				    iop->flag & IONDELIM ? "<<" :
				    evalstr(iop->delim, 0));
				need_nl = true;
			}
		}
		/*
		 * Last delimiter must be followed by a newline (this
		 * often leads to an extra blank line, but it's not
		 * worth worrying about)
		 */
		if (need_nl) {
			shf_putc('\n', shf);
			prevent_semicolon = true;
		}
	}
}

static void
pioact(struct shf *shf, struct ioword *iop)
{
	int flag = iop->flag;
	int type = flag & IOTYPE;
	int expected;

	expected = (type == IOREAD || type == IORDWR || type == IOHERE) ? 0 :
	    (type == IOCAT || type == IOWRITE) ? 1 :
	    (type == IODUP && (iop->unit == !(flag & IORDUP))) ? iop->unit :
	    iop->unit + 1;
	if (iop->unit != expected)
		shf_fprintf(shf, "%d", iop->unit);

	switch (type) {
	case IOREAD:
		shf_putc('<', shf);
		break;
	case IOHERE:
		shf_puts("<<", shf);
		if (flag & IOSKIP)
			shf_putc('-', shf);
		break;
	case IOCAT:
		shf_puts(">>", shf);
		break;
	case IOWRITE:
		shf_putc('>', shf);
		if (flag & IOCLOB)
			shf_putc('|', shf);
		break;
	case IORDWR:
		shf_puts("<>", shf);
		break;
	case IODUP:
		shf_puts(flag & IORDUP ? "<&" : ">&", shf);
		break;
	}
	/* name/delim are NULL when printing syntax errors */
	if (type == IOHERE) {
		if (iop->delim)
			wdvarput(shf, iop->delim, 0, WDS_TPUTS);
		if (iop->flag & IOHERESTR)
			shf_putc(' ', shf);
	} else if (iop->name) {
		if (iop->flag & IONAMEXP)
			print_value_quoted(shf, iop->name);
		else
			wdvarput(shf, iop->name, 0, WDS_TPUTS);
		shf_putc(' ', shf);
	}
	prevent_semicolon = false;
}

/* variant of fputs for ptreef and wdstrip */
static const char *
wdvarput(struct shf *shf, const char *wp, int quotelevel, int opmode)
{
	int c;
	const char *cs;

	/*-
	 * problems:
	 *	`...` -> $(...)
	 *	'foo' -> "foo"
	 *	x${foo:-"hi"} -> x${foo:-hi} unless WDS_TPUTS
	 *	x${foo:-'hi'} -> x${foo:-hi} unless WDS_KEEPQ
	 * could change encoding to:
	 *	OQUOTE ["'] ... CQUOTE ["']
	 *	COMSUB [(`] ...\0	(handle $ ` \ and maybe " in `...` case)
	 */
	while (/* CONSTCOND */ 1)
		switch (*wp++) {
		case EOS:
			return (--wp);
		case ADELIM:
		case CHAR:
			c = *wp++;
			if ((opmode & WDS_MAGIC) &&
			    (ISMAGIC(c) || c == '[' || c == '!' ||
			    c == '-' || c == ']' || c == '*' || c == '?'))
				shf_putc(MAGIC, shf);
			shf_putc(c, shf);
			break;
		case QCHAR: {
			bool doq;

			c = *wp++;
			doq = (c == '"' || c == '`' || c == '$' || c == '\\');
			if (opmode & WDS_TPUTS) {
				if (quotelevel == 0)
					doq = true;
			} else {
				if (!(opmode & WDS_KEEPQ))
					doq = false;
			}
			if (doq)
				shf_putc('\\', shf);
			shf_putc(c, shf);
			break;
		}
		case COMSUB:
			shf_puts("$(", shf);
			cs = ")";
 pSUB:
			while ((c = *wp++) != 0)
				shf_putc(c, shf);
			shf_puts(cs, shf);
			break;
		case FUNSUB:
			c = ' ';
			if (0)
				/* FALLTHROUGH */
		case VALSUB:
			  c = '|';
			shf_putc('$', shf);
			shf_putc('{', shf);
			shf_putc(c, shf);
			cs = ";}";
			goto pSUB;
		case EXPRSUB:
			shf_puts("$((", shf);
			cs = "))";
			goto pSUB;
		case OQUOTE:
			if (opmode & WDS_TPUTS) {
				quotelevel++;
				shf_putc('"', shf);
			}
			break;
		case CQUOTE:
			if (opmode & WDS_TPUTS) {
				if (quotelevel)
					quotelevel--;
				shf_putc('"', shf);
			}
			break;
		case OSUBST:
			shf_putc('$', shf);
			if (*wp++ == '{')
				shf_putc('{', shf);
			while ((c = *wp++) != 0)
				shf_putc(c, shf);
			wp = wdvarput(shf, wp, 0, opmode);
			break;
		case CSUBST:
			if (*wp++ == '}')
				shf_putc('}', shf);
			return (wp);
		case OPAT:
			if (opmode & WDS_MAGIC) {
				shf_putc(MAGIC, shf);
				shf_putchar(*wp++ | 0x80, shf);
			} else {
				shf_putchar(*wp++, shf);
				shf_putc('(', shf);
			}
			break;
		case SPAT:
			c = '|';
			if (0)
		case CPAT:
				c = /*(*/ ')';
			if (opmode & WDS_MAGIC)
				shf_putc(MAGIC, shf);
			shf_putc(c, shf);
			break;
		}
}

/*
 * this is the _only_ way to reliably handle
 * variable args with an ANSI compiler
 */
/* VARARGS */
void
fptreef(struct shf *shf, int indent, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfptreef(shf, indent, fmt, va);
	va_end(va);
}

/* VARARGS */
char *
snptreef(char *s, ssize_t n, const char *fmt, ...)
{
	va_list va;
	struct shf shf;

	shf_sopen(s, n, SHF_WR | (s ? 0 : SHF_DYNAMIC), &shf);

	va_start(va, fmt);
	vfptreef(&shf, 0, fmt, va);
	va_end(va);

	/* shf_sclose NUL terminates */
	return (shf_sclose(&shf));
}

static void
vfptreef(struct shf *shf, int indent, const char *fmt, va_list va)
{
	int c;

	while ((c = *fmt++)) {
		if (c == '%') {
			switch ((c = *fmt++)) {
			case 'c':
				/* character (octet, probably) */
				shf_putchar(va_arg(va, int), shf);
				break;
			case 's':
				/* string */
				shf_puts(va_arg(va, char *), shf);
				break;
			case 'S':
				/* word */
				wdvarput(shf, va_arg(va, char *), 0, WDS_TPUTS);
				break;
			case 'd':
				/* signed decimal */
				shf_fprintf(shf, "%d", va_arg(va, int));
				break;
			case 'u':
				/* unsigned decimal */
				shf_fprintf(shf, "%u", va_arg(va, unsigned int));
				break;
			case 'T':
				/* format tree */
				ptree(va_arg(va, struct op *), indent, shf);
				goto dont_trash_prevent_semicolon;
			case ';':
				/* newline or ; */
			case 'N':
				/* newline or space */
				if (shf->flags & SHF_STRING) {
					if (c == ';' && !prevent_semicolon)
						shf_putc(';', shf);
					shf_putc(' ', shf);
				} else {
					int i;

					shf_putc('\n', shf);
					i = indent;
					while (i >= 8) {
						shf_putc('\t', shf);
						i -= 8;
					}
					while (i--)
						shf_putc(' ', shf);
				}
				break;
			case 'R':
				/* I/O redirection */
				pioact(shf, va_arg(va, struct ioword *));
				break;
			default:
				shf_putc(c, shf);
				break;
			}
		} else
			shf_putc(c, shf);
		prevent_semicolon = false;
 dont_trash_prevent_semicolon:
		;
	}
}

/*
 * copy tree (for function definition)
 */
struct op *
tcopy(struct op *t, Area *ap)
{
	struct op *r;
	const char **tw;
	char **rw;

	if (t == NULL)
		return (NULL);

	r = alloc(sizeof(struct op), ap);

	r->type = t->type;
	r->u.evalflags = t->u.evalflags;

	if (t->type == TCASE)
		r->str = wdcopy(t->str, ap);
	else
		strdupx(r->str, t->str, ap);

	if (t->vars == NULL)
		r->vars = NULL;
	else {
		tw = (const char **)t->vars;
		while (*tw)
			++tw;
		rw = r->vars = alloc2(tw - (const char **)t->vars + 1,
		    sizeof(*tw), ap);
		tw = (const char **)t->vars;
		while (*tw)
			*rw++ = wdcopy(*tw++, ap);
		*rw = NULL;
	}

	if (t->args == NULL)
		r->args = NULL;
	else {
		tw = t->args;
		while (*tw)
			++tw;
		r->args = (const char **)(rw = alloc2(tw - t->args + 1,
		    sizeof(*tw), ap));
		tw = t->args;
		while (*tw)
			*rw++ = wdcopy(*tw++, ap);
		*rw = NULL;
	}

	r->ioact = (t->ioact == NULL) ? NULL : iocopy(t->ioact, ap);

	r->left = tcopy(t->left, ap);
	r->right = tcopy(t->right, ap);
	r->lineno = t->lineno;

	return (r);
}

char *
wdcopy(const char *wp, Area *ap)
{
	size_t len;

	len = wdscan(wp, EOS) - wp;
	return (memcpy(alloc(len, ap), wp, len));
}

/* return the position of prefix c in wp plus 1 */
const char *
wdscan(const char *wp, int c)
{
	int nest = 0;

	while (/* CONSTCOND */ 1)
		switch (*wp++) {
		case EOS:
			return (wp);
		case ADELIM:
			if (c == ADELIM)
				return (wp + 1);
			/* FALLTHROUGH */
		case CHAR:
		case QCHAR:
			wp++;
			break;
		case COMSUB:
		case FUNSUB:
		case VALSUB:
		case EXPRSUB:
			while (*wp++ != 0)
				;
			break;
		case OQUOTE:
		case CQUOTE:
			break;
		case OSUBST:
			nest++;
			while (*wp++ != '\0')
				;
			break;
		case CSUBST:
			wp++;
			if (c == CSUBST && nest == 0)
				return (wp);
			nest--;
			break;
		case OPAT:
			nest++;
			wp++;
			break;
		case SPAT:
		case CPAT:
			if (c == wp[-1] && nest == 0)
				return (wp);
			if (wp[-1] == CPAT)
				nest--;
			break;
		default:
			internal_warningf(
			    "wdscan: unknown char 0x%x (carrying on)",
			    wp[-1]);
		}
}

/*
 * return a copy of wp without any of the mark up characters and with
 * quote characters (" ' \) stripped. (string is allocated from ATEMP)
 */
char *
wdstrip(const char *wp, int opmode)
{
	struct shf shf;

	shf_sopen(NULL, 32, SHF_WR | SHF_DYNAMIC, &shf);
	wdvarput(&shf, wp, 0, opmode);
	/* shf_sclose NUL terminates */
	return (shf_sclose(&shf));
}

static struct ioword **
iocopy(struct ioword **iow, Area *ap)
{
	struct ioword **ior;
	int i;

	ior = iow;
	while (*ior)
		++ior;
	ior = alloc2(ior - iow + 1, sizeof(struct ioword *), ap);

	for (i = 0; iow[i] != NULL; i++) {
		struct ioword *p, *q;

		p = iow[i];
		q = alloc(sizeof(struct ioword), ap);
		ior[i] = q;
		*q = *p;
		if (p->name != NULL)
			q->name = wdcopy(p->name, ap);
		if (p->delim != NULL)
			q->delim = wdcopy(p->delim, ap);
		if (p->heredoc != NULL)
			strdupx(q->heredoc, p->heredoc, ap);
	}
	ior[i] = NULL;

	return (ior);
}

/*
 * free tree (for function definition)
 */
void
tfree(struct op *t, Area *ap)
{
	char **w;

	if (t == NULL)
		return;

	if (t->str != NULL)
		afree(t->str, ap);

	if (t->vars != NULL) {
		for (w = t->vars; *w != NULL; w++)
			afree(*w, ap);
		afree(t->vars, ap);
	}

	if (t->args != NULL) {
		/*XXX we assume the caller is right */
		union mksh_ccphack cw;

		cw.ro = t->args;
		for (w = cw.rw; *w != NULL; w++)
			afree(*w, ap);
		afree(t->args, ap);
	}

	if (t->ioact != NULL)
		iofree(t->ioact, ap);

	tfree(t->left, ap);
	tfree(t->right, ap);

	afree(t, ap);
}

static void
iofree(struct ioword **iow, Area *ap)
{
	struct ioword **iop;
	struct ioword *p;

	iop = iow;
	while ((p = *iop++) != NULL) {
		if (p->name != NULL)
			afree(p->name, ap);
		if (p->delim != NULL)
			afree(p->delim, ap);
		if (p->heredoc != NULL)
			afree(p->heredoc, ap);
		afree(p, ap);
	}
	afree(iow, ap);
}

void
fpFUNCTf(struct shf *shf, int i, bool isksh, const char *k, struct op *v)
{
	if (isksh)
		fptreef(shf, i, "%s %s %T", Tfunction, k, v);
	else
		fptreef(shf, i, "%s() %T", k, v);
}


/* for jobs.c */
void
vistree(char *dst, size_t sz, struct op *t)
{
	unsigned int c;
	char *cp, *buf;
	size_t n;

	buf = alloc(sz + 16, ATEMP);
	snptreef(buf, sz + 16, "%T", t);
	cp = buf;
 vist_loop:
	if (UTFMODE && (n = utf_mbtowc(&c, cp)) != (size_t)-1) {
		if (c == 0 || n >= sz)
			/* NUL or not enough free space */
			goto vist_out;
		/* copy multibyte char */
		sz -= n;
		while (n--)
			*dst++ = *cp++;
		goto vist_loop;
	}
	if (--sz == 0 || (c = (unsigned char)(*cp++)) == 0)
		/* NUL or not enough free space */
		goto vist_out;
	if (ISCTRL(c & 0x7F)) {
		/* C0 or C1 control character or DEL */
		if (--sz == 0)
			/* not enough free space for two chars */
			goto vist_out;
		*dst++ = (c & 0x80) ? '$' : '^';
		c = UNCTRL(c & 0x7F);
	} else if (UTFMODE && c > 0x7F) {
		/* better not try to display broken multibyte chars */
		/* also go easy on the Unicode: no U+FFFD here */
		c = '?';
	}
	*dst++ = c;
	goto vist_loop;

 vist_out:
	*dst = '\0';
	afree(buf, ATEMP);
}

#ifdef DEBUG
void
dumpchar(struct shf *shf, int c)
{
	if (ISCTRL(c & 0x7F)) {
		/* C0 or C1 control character or DEL */
		shf_putc((c & 0x80) ? '$' : '^', shf);
		c = UNCTRL(c & 0x7F);
	}
	shf_putc(c, shf);
}

/* see: wdvarput */
static const char *
dumpwdvar_i(struct shf *shf, const char *wp, int quotelevel)
{
	int c;

	while (/* CONSTCOND */ 1) {
		switch(*wp++) {
		case EOS:
			shf_puts("EOS", shf);
			return (--wp);
		case ADELIM:
			shf_puts("ADELIM=", shf);
			if (0)
		case CHAR:
				shf_puts("CHAR=", shf);
			dumpchar(shf, *wp++);
			break;
		case QCHAR:
			shf_puts("QCHAR<", shf);
			c = *wp++;
			if (quotelevel == 0 ||
			    (c == '"' || c == '`' || c == '$' || c == '\\'))
				shf_putc('\\', shf);
			dumpchar(shf, c);
			goto closeandout;
		case COMSUB:
			shf_puts("COMSUB<", shf);
 dumpsub:
			while ((c = *wp++) != 0)
				dumpchar(shf, c);
 closeandout:
			shf_putc('>', shf);
			break;
		case FUNSUB:
			shf_puts("FUNSUB<", shf);
			goto dumpsub;
		case VALSUB:
			shf_puts("VALSUB<", shf);
			goto dumpsub;
		case EXPRSUB:
			shf_puts("EXPRSUB<", shf);
			goto dumpsub;
		case OQUOTE:
			shf_fprintf(shf, "OQUOTE{%d", ++quotelevel);
			break;
		case CQUOTE:
			shf_fprintf(shf, "%d}CQUOTE", quotelevel);
			if (quotelevel)
				quotelevel--;
			else
				shf_puts("(err)", shf);
			break;
		case OSUBST:
			shf_puts("OSUBST(", shf);
			dumpchar(shf, *wp++);
			shf_puts(")[", shf);
			while ((c = *wp++) != 0)
				dumpchar(shf, c);
			shf_putc('|', shf);
			wp = dumpwdvar_i(shf, wp, 0);
			break;
		case CSUBST:
			shf_puts("]CSUBST(", shf);
			dumpchar(shf, *wp++);
			shf_putc(')', shf);
			return (wp);
		case OPAT:
			shf_puts("OPAT=", shf);
			dumpchar(shf, *wp++);
			break;
		case SPAT:
			shf_puts("SPAT", shf);
			break;
		case CPAT:
			shf_puts("CPAT", shf);
			break;
		default:
			shf_fprintf(shf, "INVAL<%u>", (uint8_t)wp[-1]);
			break;
		}
		shf_putc(' ', shf);
	}
}
void
dumpwdvar(struct shf *shf, const char *wp)
{
	dumpwdvar_i(shf, wp, 0);
}

void
dumpioact(struct shf *shf, struct op *t)
{
	struct ioword **ioact, *iop;

	if ((ioact = t->ioact) == NULL)
		return;

	shf_puts("{IOACT", shf);
	while ((iop = *ioact++) != NULL) {
		int type = iop->flag & IOTYPE;
#define DT(x) case x: shf_puts(#x, shf); break;
#define DB(x) if (iop->flag & x) shf_puts("|" #x, shf);

		shf_putc(';', shf);
		switch (type) {
		DT(IOREAD)
		DT(IOWRITE)
		DT(IORDWR)
		DT(IOHERE)
		DT(IOCAT)
		DT(IODUP)
		default:
			shf_fprintf(shf, "unk%d", type);
		}
		DB(IOEVAL)
		DB(IOSKIP)
		DB(IOCLOB)
		DB(IORDUP)
		DB(IONAMEXP)
		DB(IOBASH)
		DB(IOHERESTR)
		DB(IONDELIM)
		shf_fprintf(shf, ",unit=%d", iop->unit);
		if (iop->delim) {
			shf_puts(",delim<", shf);
			dumpwdvar(shf, iop->delim);
			shf_putc('>', shf);
		}
		if (iop->name) {
			if (iop->flag & IONAMEXP) {
				shf_puts(",name=", shf);
				print_value_quoted(shf, iop->name);
			} else {
				shf_puts(",name<", shf);
				dumpwdvar(shf, iop->name);
				shf_putc('>', shf);
			}
		}
		if (iop->heredoc) {
			shf_puts(",heredoc=", shf);
			print_value_quoted(shf, iop->heredoc);
		}
#undef DT
#undef DB
	}
	shf_putc('}', shf);
}

void
dumptree(struct shf *shf, struct op *t)
{
	int i, j;
	const char **w, *name;
	struct op *t1;
	static int nesting;

	for (i = 0; i < nesting; ++i)
		shf_putc('\t', shf);
	++nesting;
	shf_puts("{tree:" /*}*/, shf);
	if (t == NULL) {
		name = "(null)";
		goto out;
	}
	dumpioact(shf, t);
	switch (t->type) {
#define OPEN(x) case x: name = #x; shf_puts(" {" #x ":", shf); /*}*/

	OPEN(TCOM)
		if (t->vars) {
			i = 0;
			w = (const char **)t->vars;
			while (*w) {
				shf_putc('\n', shf);
				for (j = 0; j < nesting; ++j)
					shf_putc('\t', shf);
				shf_fprintf(shf, " var%d<", i++);
				dumpwdvar(shf, *w++);
				shf_putc('>', shf);
			}
		} else
			shf_puts(" #no-vars#", shf);
		if (t->args) {
			i = 0;
			w = t->args;
			while (*w) {
				shf_putc('\n', shf);
				for (j = 0; j < nesting; ++j)
					shf_putc('\t', shf);
				shf_fprintf(shf, " arg%d<", i++);
				dumpwdvar(shf, *w++);
				shf_putc('>', shf);
			}
		} else
			shf_puts(" #no-args#", shf);
		break;
	OPEN(TEXEC)
 dumpleftandout:
		t = t->left;
 dumpandout:
		shf_putc('\n', shf);
		dumptree(shf, t);
		break;
	OPEN(TPAREN)
		goto dumpleftandout;
	OPEN(TPIPE)
 dumpleftmidrightandout:
		shf_putc('\n', shf);
		dumptree(shf, t->left);
/* middumprightandout: (unused) */
		shf_fprintf(shf, "/%s:", name);
 dumprightandout:
		t = t->right;
		goto dumpandout;
	OPEN(TLIST)
		goto dumpleftmidrightandout;
	OPEN(TOR)
		goto dumpleftmidrightandout;
	OPEN(TAND)
		goto dumpleftmidrightandout;
	OPEN(TBANG)
		goto dumprightandout;
	OPEN(TDBRACKET)
		i = 0;
		w = t->args;
		while (*w) {
			shf_putc('\n', shf);
			for (j = 0; j < nesting; ++j)
				shf_putc('\t', shf);
			shf_fprintf(shf, " arg%d<", i++);
			dumpwdvar(shf, *w++);
			shf_putc('>', shf);
		}
		break;
	OPEN(TFOR)
 dumpfor:
		shf_fprintf(shf, " str<%s>", t->str);
		if (t->vars != NULL) {
			i = 0;
			w = (const char **)t->vars;
			while (*w) {
				shf_putc('\n', shf);
				for (j = 0; j < nesting; ++j)
					shf_putc('\t', shf);
				shf_fprintf(shf, " var%d<", i++);
				dumpwdvar(shf, *w++);
				shf_putc('>', shf);
			}
		}
		goto dumpleftandout;
	OPEN(TSELECT)
		goto dumpfor;
	OPEN(TCASE)
		shf_fprintf(shf, " str<%s>", t->str);
		i = 0;
		for (t1 = t->left; t1 != NULL; t1 = t1->right) {
			shf_putc('\n', shf);
			for (j = 0; j < nesting; ++j)
				shf_putc('\t', shf);
			shf_fprintf(shf, " sub%d[(", i);
			w = (const char **)t1->vars;
			while (*w) {
				dumpwdvar(shf, *w);
				if (w[1] != NULL)
					shf_putc('|', shf);
				++w;
			}
			shf_putc(')', shf);
			dumpioact(shf, t);
			shf_putc('\n', shf);
			dumptree(shf, t1->left);
			shf_fprintf(shf, " ;%c/%d]", t1->u.charflag, i++);
		}
		break;
	OPEN(TWHILE)
		goto dumpleftmidrightandout;
	OPEN(TUNTIL)
		goto dumpleftmidrightandout;
	OPEN(TBRACE)
		goto dumpleftandout;
	OPEN(TCOPROC)
		goto dumpleftandout;
	OPEN(TASYNC)
		goto dumpleftandout;
	OPEN(TFUNCT)
		shf_fprintf(shf, " str<%s> ksh<%s>", t->str,
		    t->u.ksh_func ? "yes" : "no");
		goto dumpleftandout;
	OPEN(TTIME)
		goto dumpleftandout;
	OPEN(TIF)
 dumpif:
		shf_putc('\n', shf);
		dumptree(shf, t->left);
		t = t->right;
		dumpioact(shf, t);
		if (t->left != NULL) {
			shf_puts(" /TTHEN:\n", shf);
			dumptree(shf, t->left);
		}
		if (t->right && t->right->type == TELIF) {
			shf_puts(" /TELIF:", shf);
			t = t->right;
			dumpioact(shf, t);
			goto dumpif;
		}
		if (t->right != NULL) {
			shf_puts(" /TELSE:\n", shf);
			dumptree(shf, t->right);
		}
		break;
	OPEN(TEOF)
 dumpunexpected:
		shf_puts("unexpected", shf);
		break;
	OPEN(TELIF)
		goto dumpunexpected;
	OPEN(TPAT)
		goto dumpunexpected;
	default:
		name = "TINVALID";
		shf_fprintf(shf, "{T<%d>:" /*}*/, t->type);
		goto dumpunexpected;

#undef OPEN
	}
 out:
	shf_fprintf(shf, /*{*/ " /%s}\n", name);
	--nesting;
}
#endif
