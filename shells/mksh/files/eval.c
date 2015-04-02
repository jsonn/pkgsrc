/*	$OpenBSD: eval.c,v 1.40 2013/09/14 20:09:30 millert Exp $	*/

/*-
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 *		 2011, 2012, 2013, 2014, 2015
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

__RCSID("$MirOS: src/bin/mksh/eval.c,v 1.158.2.4 2015/03/01 15:42:58 tg Exp $");

/*
 * string expansion
 *
 * first pass: quoting, IFS separation, ~, ${}, $() and $(()) substitution.
 * second pass: alternation ({,}), filename expansion (*?[]).
 */

/* expansion generator state */
typedef struct {
	/* not including an "int type;" member, see expand() */
	/* string */
	const char *str;
	/* source */
	union {
		/* string[] */
		const char **strv;
		/* file */
		struct shf *shf;
	} u;
	/* variable in ${var...} */
	struct tbl *var;
	/* split "$@" / call waitlast in $() */
	bool split;
} Expand;

#define	XBASE		0	/* scanning original */
#define	XSUB		1	/* expanding ${} string */
#define	XARGSEP		2	/* ifs0 between "$*" */
#define	XARG		3	/* expanding $*, $@ */
#define	XCOM		4	/* expanding $() */
#define XNULLSUB	5	/* "$@" when $# is 0 (don't generate word) */
#define XSUBMID		6	/* middle of expanding ${} */

/* States used for field splitting */
#define IFS_WORD	0	/* word has chars (or quotes except "$@") */
#define IFS_WS		1	/* have seen IFS white-space */
#define IFS_NWS		2	/* have seen IFS non-white-space */
#define IFS_IWS		3	/* begin of word, ignore IFS WS */
#define IFS_QUOTE	4	/* beg.w/quote, become IFS_WORD unless "$@" */

static int varsub(Expand *, const char *, const char *, int *, int *);
static int comsub(Expand *, const char *, int);
static char *valsub(struct op *, Area *);
static char *trimsub(char *, char *, int);
static void glob(char *, XPtrV *, bool);
static void globit(XString *, char **, char *, XPtrV *, int);
static const char *maybe_expand_tilde(const char *, XString *, char **, bool);
#ifndef MKSH_NOPWNAM
static char *homedir(char *);
#endif
static void alt_expand(XPtrV *, char *, char *, char *, int);
static int utflen(const char *) MKSH_A_PURE;
static void utfincptr(const char *, mksh_ari_t *);

/* UTFMODE functions */
static int
utflen(const char *s)
{
	size_t n;

	if (UTFMODE) {
		n = 0;
		while (*s) {
			s += utf_ptradj(s);
			++n;
		}
	} else
		n = strlen(s);

	if (n > 2147483647)
		n = 2147483647;
	return ((int)n);
}

static void
utfincptr(const char *s, mksh_ari_t *lp)
{
	const char *cp = s;

	while ((*lp)--)
		cp += utf_ptradj(cp);
	*lp = cp - s;
}

/* compile and expand word */
char *
substitute(const char *cp, int f)
{
	struct source *s, *sold;

	sold = source;
	s = pushs(SWSTR, ATEMP);
	s->start = s->str = cp;
	source = s;
	if (yylex(ONEWORD) != LWORD)
		internal_errorf("bad substitution");
	source = sold;
	afree(s, ATEMP);
	return (evalstr(yylval.cp, f));
}

/*
 * expand arg-list
 */
char **
eval(const char **ap, int f)
{
	XPtrV w;

	if (*ap == NULL) {
		union mksh_ccphack vap;

		vap.ro = ap;
		return (vap.rw);
	}
	XPinit(w, 32);
	/* space for shell name */
	XPput(w, NULL);
	while (*ap != NULL)
		expand(*ap++, &w, f);
	XPput(w, NULL);
	return ((char **)XPclose(w) + 1);
}

/*
 * expand string
 */
char *
evalstr(const char *cp, int f)
{
	XPtrV w;
	char *dp = null;

	XPinit(w, 1);
	expand(cp, &w, f);
	if (XPsize(w))
		dp = *XPptrv(w);
	XPfree(w);
	return (dp);
}

/*
 * expand string - return only one component
 * used from iosetup to expand redirection files
 */
char *
evalonestr(const char *cp, int f)
{
	XPtrV w;
	char *rv;

	XPinit(w, 1);
	expand(cp, &w, f);
	switch (XPsize(w)) {
	case 0:
		rv = null;
		break;
	case 1:
		rv = (char *) *XPptrv(w);
		break;
	default:
		rv = evalstr(cp, f&~DOGLOB);
		break;
	}
	XPfree(w);
	return (rv);
}

/* for nested substitution: ${var:=$var2} */
typedef struct SubType {
	struct tbl *var;	/* variable for ${var..} */
	struct SubType *prev;	/* old type */
	struct SubType *next;	/* poped type (to avoid re-allocating) */
	size_t	base;		/* begin position of expanded word */
	short	stype;		/* [=+-?%#] action after expanded word */
	short	f;		/* saved value of f (DOPAT, etc) */
	uint8_t	quotep;		/* saved value of quote (for ${..[%#]..}) */
	uint8_t	quotew;		/* saved value of quote (for ${..[+-=]..}) */
} SubType;

void
expand(
    /* input word */
    const char *ccp,
    /* output words */
    XPtrV *wp,
    /* DO* flags */
    int f)
{
	int c = 0;
	/* expansion type */
	int type;
	/* quoted */
	int quote = 0;
	/* destination string and live pointer */
	XString ds;
	char *dp;
	/* source */
	const char *sp;
	/* second pass flags */
	int fdo;
	/* have word */
	int word;
	/* field splitting of parameter/command substitution */
	int doblank;
	/* expansion variables */
	Expand x = {
		NULL, { NULL }, NULL, 0
	};
	SubType st_head, *st;
	/* record number of trailing newlines in COMSUB */
	int newlines = 0;
	bool saw_eq, make_magic;
	unsigned int tilde_ok;
	size_t len;
	char *cp;

	if (ccp == NULL)
		internal_errorf("expand(NULL)");
	/* for alias, readonly, set, typeset commands */
	if ((f & DOVACHECK) && is_wdvarassign(ccp)) {
		f &= ~(DOVACHECK | DOBLANK | DOGLOB | DOTILDE);
		f |= DOASNTILDE | DOSCALAR;
	}
	if (Flag(FNOGLOB))
		f &= ~DOGLOB;
	if (Flag(FMARKDIRS))
		f |= DOMARKDIRS;
	if (Flag(FBRACEEXPAND) && (f & DOGLOB))
		f |= DOBRACE;

	/* init destination string */
	Xinit(ds, dp, 128, ATEMP);
	type = XBASE;
	sp = ccp;
	fdo = 0;
	saw_eq = false;
	/* must be 1/0 */
	tilde_ok = (f & (DOTILDE | DOASNTILDE)) ? 1 : 0;
	doblank = 0;
	make_magic = false;
	word = (f&DOBLANK) ? IFS_WS : IFS_WORD;
	/* clang doesn't know OSUBST comes before CSUBST */
	memset(&st_head, 0, sizeof(st_head));
	st = &st_head;

	while (/* CONSTCOND */ 1) {
		Xcheck(ds, dp);

		switch (type) {
		case XBASE:
			/* original prefixed string */
			c = *sp++;
			switch (c) {
			case EOS:
				c = 0;
				break;
			case CHAR:
				c = *sp++;
				break;
			case QCHAR:
				/* temporary quote */
				quote |= 2;
				c = *sp++;
				break;
			case OQUOTE:
				switch (word) {
				case IFS_QUOTE:
					/* """something */
					word = IFS_WORD;
					break;
				case IFS_WORD:
					break;
				default:
					word = IFS_QUOTE;
					break;
				}
				tilde_ok = 0;
				quote = 1;
				continue;
			case CQUOTE:
				quote = st->quotew;
				continue;
			case COMSUB:
			case FUNSUB:
			case VALSUB:
				tilde_ok = 0;
				if (f & DONTRUNCOMMAND) {
					word = IFS_WORD;
					*dp++ = '$';
					*dp++ = c == COMSUB ? '(' : '{';
					if (c != COMSUB)
						*dp++ = c == FUNSUB ? ' ' : '|';
					while (*sp != '\0') {
						Xcheck(ds, dp);
						*dp++ = *sp++;
					}
					if (c != COMSUB) {
						*dp++ = ';';
						*dp++ = '}';
					} else
						*dp++ = ')';
				} else {
					type = comsub(&x, sp, c);
					if (type != XBASE && (f & DOBLANK))
						doblank++;
					sp = strnul(sp) + 1;
					newlines = 0;
				}
				continue;
			case EXPRSUB:
				tilde_ok = 0;
				if (f & DONTRUNCOMMAND) {
					word = IFS_WORD;
					*dp++ = '$'; *dp++ = '('; *dp++ = '(';
					while (*sp != '\0') {
						Xcheck(ds, dp);
						*dp++ = *sp++;
					}
					*dp++ = ')'; *dp++ = ')';
				} else {
					struct tbl v;

					v.flag = DEFINED|ISSET|INTEGER;
					/* not default */
					v.type = 10;
					v.name[0] = '\0';
					v_evaluate(&v, substitute(sp, 0),
					    KSH_UNWIND_ERROR, true);
					sp = strnul(sp) + 1;
					x.str = str_val(&v);
					type = XSUB;
					if (f & DOBLANK)
						doblank++;
				}
				continue;
			case OSUBST: {
				/* ${{#}var{:}[=+-?#%]word} */
			/*-
			 * format is:
			 *	OSUBST [{x] plain-variable-part \0
			 *	    compiled-word-part CSUBST [}x]
			 * This is where all syntax checking gets done...
			 */
				/* skip the { or x (}) */
				const char *varname = ++sp;
				int stype;
				int slen = 0;

				/* skip variable */
				sp = cstrchr(sp, '\0') + 1;
				type = varsub(&x, varname, sp, &stype, &slen);
				if (type < 0) {
					char *beg, *end, *str;
 unwind_substsyn:
					/* restore sp */
					sp = varname - 2;
					end = (beg = wdcopy(sp, ATEMP)) +
					    (wdscan(sp, CSUBST) - sp);
					/* ({) the } or x is already skipped */
					if (end < wdscan(beg, EOS))
						*end = EOS;
					str = snptreef(NULL, 64, "%S", beg);
					afree(beg, ATEMP);
					errorf("%s: %s", str, "bad substitution");
				}
				if (f & DOBLANK)
					doblank++;
				tilde_ok = 0;
				if (word == IFS_QUOTE && type != XNULLSUB)
					word = IFS_WORD;
				if (type == XBASE) {
					/* expand? */
					if (!st->next) {
						SubType *newst;

						newst = alloc(sizeof(SubType), ATEMP);
						newst->next = NULL;
						newst->prev = st;
						st->next = newst;
					}
					st = st->next;
					st->stype = stype;
					st->base = Xsavepos(ds, dp);
					st->f = f;
					if (x.var == &vtemp) {
						st->var = tempvar();
						st->var->flag &= ~INTEGER;
						/* can't fail here */
						setstr(st->var,
						    str_val(x.var),
						    KSH_RETURN_ERROR | 0x4);
					} else
						st->var = x.var;

					st->quotew = st->quotep = quote;
					/* skip qualifier(s) */
					if (stype)
						sp += slen;
					switch (stype & 0x17F) {
					case 0x100 | '#':
						x.str = shf_smprintf("%08X",
						    (unsigned int)hash(str_val(st->var)));
						break;
					case 0x100 | 'Q': {
						struct shf shf;

						shf_sopen(NULL, 0, SHF_WR|SHF_DYNAMIC, &shf);
						print_value_quoted(&shf, str_val(st->var));
						x.str = shf_sclose(&shf);
						break;
					    }
					case '0': {
						char *beg, *mid, *end, *stg;
						mksh_ari_t from = 0, num = -1, flen, finc = 0;

						beg = wdcopy(sp, ATEMP);
						mid = beg + (wdscan(sp, ADELIM) - sp);
						stg = beg + (wdscan(sp, CSUBST) - sp);
						if (mid >= stg)
							goto unwind_substsyn;
						mid[-2] = EOS;
						if (mid[-1] == /*{*/'}') {
							sp += mid - beg - 1;
							end = NULL;
						} else {
							end = mid +
							    (wdscan(mid, ADELIM) - mid);
							if (end >= stg ||
							    /* more than max delimiters */
							    end[-1] != /*{*/ '}')
								goto unwind_substsyn;
							end[-2] = EOS;
							sp += end - beg - 1;
						}
						evaluate(substitute(stg = wdstrip(beg, 0), 0),
						    &from, KSH_UNWIND_ERROR, true);
						afree(stg, ATEMP);
						if (end) {
							evaluate(substitute(stg = wdstrip(mid, 0), 0),
							    &num, KSH_UNWIND_ERROR, true);
							afree(stg, ATEMP);
						}
						afree(beg, ATEMP);
						beg = str_val(st->var);
						flen = utflen(beg);
						if (from < 0) {
							if (-from < flen)
								finc = flen + from;
						} else
							finc = from < flen ? from : flen;
						if (UTFMODE)
							utfincptr(beg, &finc);
						beg += finc;
						flen = utflen(beg);
						if (num < 0 || num > flen)
							num = flen;
						if (UTFMODE)
							utfincptr(beg, &num);
						strndupx(x.str, beg, num, ATEMP);
						goto do_CSUBST;
					    }
					case '/': {
						char *s, *p, *d, *sbeg, *end;
						char *pat, *rrep;
						char *tpat0, *tpat1, *tpat2;

						s = wdcopy(sp, ATEMP);
						p = s + (wdscan(sp, ADELIM) - sp);
						d = s + (wdscan(sp, CSUBST) - sp);
						if (p >= d)
							goto unwind_substsyn;
						p[-2] = EOS;
						if (p[-1] == /*{*/'}')
							d = NULL;
						else
							d[-2] = EOS;
						sp += (d ? d : p) - s - 1;
						tpat0 = wdstrip(s,
						    WDS_KEEPQ | WDS_MAGIC);
						pat = substitute(tpat0, 0);
						if (d) {
							d = wdstrip(p, WDS_KEEPQ);
							rrep = substitute(d, 0);
							afree(d, ATEMP);
						} else
							rrep = null;
						afree(s, ATEMP);
						s = d = pat;
						while (*s)
							if (*s != '\\' ||
							    s[1] == '%' ||
							    s[1] == '#' ||
							    s[1] == '\0' ||
				/* XXX really? */	    s[1] == '\\' ||
							    s[1] == '/')
								*d++ = *s++;
							else
								s++;
						*d = '\0';
						afree(tpat0, ATEMP);

						/* check for special cases */
						d = str_val(st->var);
						switch (*pat) {
						case '#':
							/* anchor at begin */
							tpat0 = pat + 1;
							tpat1 = rrep;
							tpat2 = d;
							break;
						case '%':
							/* anchor at end */
							tpat0 = pat + 1;
							tpat1 = d;
							tpat2 = rrep;
							break;
						case '\0':
							/* empty pattern */
							goto no_repl;
						default:
							tpat0 = pat;
							/* silence gcc */
							tpat1 = tpat2 = NULL;
						}
						if (gmatchx(null, tpat0, false)) {
							/*
							 * pattern matches
							 * the empty string
							 */
							if (tpat0 == pat)
								goto no_repl;
							/* but is anchored */
							s = shf_smprintf("%s%s",
							    tpat1, tpat2);
							goto do_repl;
						}

						/* prepare string on which to work */
						strdupx(s, d, ATEMP);
						sbeg = s;

						/* first see if we have any match at all */
						tpat0 = pat;
						if (*pat == '#') {
							/* anchor at the beginning */
							tpat1 = shf_smprintf("%s%c*", ++tpat0, MAGIC);
							tpat2 = tpat1;
						} else if (*pat == '%') {
							/* anchor at the end */
							tpat1 = shf_smprintf("%c*%s", MAGIC, ++tpat0);
							tpat2 = tpat0;
						} else {
							/* float */
							tpat1 = shf_smprintf("%c*%s%c*", MAGIC, pat, MAGIC);
							tpat2 = tpat1 + 2;
						}
 again_repl:
						/*
						 * this would not be necessary if gmatchx would return
						 * the start and end values of a match found, like re*
						 */
						if (!gmatchx(sbeg, tpat1, false))
							goto end_repl;
						end = strnul(s);
						/* now anchor the beginning of the match */
						if (*pat != '#')
							while (sbeg <= end) {
								if (gmatchx(sbeg, tpat2, false))
									break;
								else
									sbeg++;
							}
						/* now anchor the end of the match */
						p = end;
						if (*pat != '%')
							while (p >= sbeg) {
								bool gotmatch;

								c = *p;
								*p = '\0';
								gotmatch = tobool(gmatchx(sbeg, tpat0, false));
								*p = c;
								if (gotmatch)
									break;
								p--;
							}
						strndupx(end, s, sbeg - s, ATEMP);
						d = shf_smprintf("%s%s%s", end, rrep, p);
						afree(end, ATEMP);
						sbeg = d + (sbeg - s) + strlen(rrep);
						afree(s, ATEMP);
						s = d;
						if (stype & 0x80)
							goto again_repl;
 end_repl:
						afree(tpat1, ATEMP);
 do_repl:
						x.str = s;
 no_repl:
						afree(pat, ATEMP);
						if (rrep != null)
							afree(rrep, ATEMP);
						goto do_CSUBST;
					    }
					case '#':
					case '%':
						/* ! DOBLANK,DOBRACE,DOTILDE */
						f = (f & DONTRUNCOMMAND) |
						    DOPAT | DOTEMP | DOSCALAR;
						st->quotew = quote = 0;
						/*
						 * Prepend open pattern (so |
						 * in a trim will work as
						 * expected)
						 */
						if (!Flag(FSH)) {
							*dp++ = MAGIC;
							*dp++ = 0x80 | '@';
						}
						break;
					case '=':
						/*
						 * Enabling tilde expansion
						 * after :s here is
						 * non-standard ksh, but is
						 * consistent with rules for
						 * other assignments. Not
						 * sure what POSIX thinks of
						 * this.
						 * Not doing tilde expansion
						 * for integer variables is a
						 * non-POSIX thing - makes
						 * sense though, since ~ is
						 * a arithmetic operator.
						 */
						if (!(x.var->flag & INTEGER))
							f |= DOASNTILDE | DOTILDE;
						f |= DOTEMP;
						/*
						 * These will be done after the
						 * value has been assigned.
						 */
						f &= ~(DOBLANK|DOGLOB|DOBRACE);
						tilde_ok = 1;
						break;
					case '?':
						f &= ~DOBLANK;
						f |= DOTEMP;
						/* FALLTHROUGH */
					default:
						/* '-' '+' '?' */
						if (quote)
							word = IFS_WORD;
						else if (dp == Xstring(ds, dp))
							word = IFS_IWS;
						/* Enable tilde expansion */
						tilde_ok = 1;
						f |= DOTILDE;
					}
				} else
					/* skip word */
					sp += wdscan(sp, CSUBST) - sp;
				continue;
			    }
			case CSUBST:
				/* only get here if expanding word */
 do_CSUBST:
				/* ({) skip the } or x */
				sp++;
				/* in case of ${unset:-} */
				tilde_ok = 0;
				*dp = '\0';
				quote = st->quotep;
				f = st->f;
				if (f & DOBLANK)
					doblank--;
				switch (st->stype & 0x17F) {
				case '#':
				case '%':
					if (!Flag(FSH)) {
						/* Append end-pattern */
						*dp++ = MAGIC;
						*dp++ = ')';
					}
					*dp = '\0';
					dp = Xrestpos(ds, dp, st->base);
					/*
					 * Must use st->var since calling
					 * global would break things
					 * like x[i+=1].
					 */
					x.str = trimsub(str_val(st->var),
						dp, st->stype);
					if (x.str[0] != '\0') {
						word = IFS_IWS;
						type = XSUB;
					} else if (quote) {
						word = IFS_WORD;
						type = XSUB;
					} else {
						if (dp == Xstring(ds, dp))
							word = IFS_IWS;
						type = XNULLSUB;
					}
					if (f & DOBLANK)
						doblank++;
					st = st->prev;
					continue;
				case '=':
					/*
					 * Restore our position and substitute
					 * the value of st->var (may not be
					 * the assigned value in the presence
					 * of integer/right-adj/etc attributes).
					 */
					dp = Xrestpos(ds, dp, st->base);
					/*
					 * Must use st->var since calling
					 * global would cause with things
					 * like x[i+=1] to be evaluated twice.
					 */
					/*
					 * Note: not exported by FEXPORT
					 * in AT&T ksh.
					 */
					/*
					 * XXX POSIX says readonly is only
					 * fatal for special builtins (setstr
					 * does readonly check).
					 */
					len = strlen(dp) + 1;
					setstr(st->var,
					    debunk(alloc(len, ATEMP),
					    dp, len), KSH_UNWIND_ERROR);
					x.str = str_val(st->var);
					type = XSUB;
					if (f & DOBLANK)
						doblank++;
					st = st->prev;
					word = quote || (!*x.str && (f & DOSCALAR)) ? IFS_WORD : IFS_IWS;
					continue;
				case '?': {
					char *s = Xrestpos(ds, dp, st->base);

					errorf("%s: %s", st->var->name,
					    dp == s ?
					    "parameter null or not set" :
					    (debunk(s, s, strlen(s) + 1), s));
				    }
				case '0':
				case '/':
				case 0x100 | '#':
				case 0x100 | 'Q':
					dp = Xrestpos(ds, dp, st->base);
					type = XSUB;
					word = quote || (!*x.str && (f & DOSCALAR)) ? IFS_WORD : IFS_IWS;
					if (f & DOBLANK)
						doblank++;
					st = st->prev;
					continue;
				/* default: '-' '+' */
				}
				st = st->prev;
				type = XBASE;
				continue;

			case OPAT:
				/* open pattern: *(foo|bar) */
				/* Next char is the type of pattern */
				make_magic = true;
				c = *sp++ | 0x80;
				break;

			case SPAT:
				/* pattern separator (|) */
				make_magic = true;
				c = '|';
				break;

			case CPAT:
				/* close pattern */
				make_magic = true;
				c = /*(*/ ')';
				break;
			}
			break;

		case XNULLSUB:
			/*
			 * Special case for "$@" (and "${foo[@]}") - no
			 * word is generated if $# is 0 (unless there is
			 * other stuff inside the quotes).
			 */
			type = XBASE;
			if (f & DOBLANK) {
				doblank--;
				if (dp == Xstring(ds, dp) && word != IFS_WORD)
					word = IFS_IWS;
			}
			continue;

		case XSUB:
		case XSUBMID:
			if ((c = *x.str++) == 0) {
				type = XBASE;
				if (f & DOBLANK)
					doblank--;
				continue;
			}
			break;

		case XARGSEP:
			type = XARG;
			quote = 1;
			/* FALLTHROUGH */
		case XARG:
			if ((c = *x.str++) == '\0') {
				/*
				 * force null words to be created so
				 * set -- "" 2 ""; echo "$@" will do
				 * the right thing
				 */
				if (quote && x.split)
					word = IFS_WORD;
				if ((x.str = *x.u.strv++) == NULL) {
					type = XBASE;
					if (f & DOBLANK)
						doblank--;
					continue;
				}
				c = ifs0;
				if ((f & DOHEREDOC)) {
					/* pseudo-field-split reliably */
					if (c == 0)
						c = ' ';
					break;
				}
				if ((f & DOSCALAR)) {
					/* do not field-split */
					if (x.split) {
						c = ' ';
						break;
					}
					if (c == 0)
						continue;
				}
				if (c == 0) {
					if (quote && !x.split)
						continue;
					if (!quote && word == IFS_WS)
						continue;
					/* this is so we don't terminate */
					c = ' ';
					/* now force-emit a word */
					goto emit_word;
				}
				if (quote && x.split) {
					/* terminate word for "$@" */
					type = XARGSEP;
					quote = 0;
				}
			}
			break;

		case XCOM:
			if (x.u.shf == NULL) {
				/* $(<...) failed */
				subst_exstat = 1;
				/* fake EOF */
				c = -1;
			} else if (newlines) {
				/* spit out saved NLs */
				c = '\n';
				--newlines;
			} else {
				while ((c = shf_getc(x.u.shf)) == 0 || c == '\n')
					if (c == '\n')
						/* save newlines */
						newlines++;
				if (newlines && c != -1) {
					shf_ungetc(c, x.u.shf);
					c = '\n';
					--newlines;
				}
			}
			if (c == -1) {
				newlines = 0;
				if (x.u.shf)
					shf_close(x.u.shf);
				if (x.split)
					subst_exstat = waitlast();
				type = XBASE;
				if (f & DOBLANK)
					doblank--;
				continue;
			}
			break;
		}

		/* check for end of word or IFS separation */
		if (c == 0 || (!quote && (f & DOBLANK) && doblank &&
		    !make_magic && ctype(c, C_IFS))) {
			/*-
			 * How words are broken up:
			 *			|	value of c
			 *	word		|	ws	nws	0
			 *	-----------------------------------
			 *	IFS_WORD		w/WS	w/NWS	w
			 *	IFS_WS			-/WS	-/NWS	-
			 *	IFS_NWS			-/NWS	w/NWS	-
			 *	IFS_IWS			-/WS	w/NWS	-
			 * (w means generate a word)
			 */
			if ((word == IFS_WORD) || (word == IFS_QUOTE) || (c &&
			    (word == IFS_IWS || word == IFS_NWS) &&
			    !ctype(c, C_IFSWS))) {
 emit_word:
				*dp++ = '\0';
				cp = Xclose(ds, dp);
				if (fdo & DOBRACE)
					/* also does globbing */
					alt_expand(wp, cp, cp,
					    cp + Xlength(ds, (dp - 1)),
					    fdo | (f & DOMARKDIRS));
				else if (fdo & DOGLOB)
					glob(cp, wp, tobool(f & DOMARKDIRS));
				else if ((f & DOPAT) || !(fdo & DOMAGIC))
					XPput(*wp, cp);
				else
					XPput(*wp, debunk(cp, cp,
					    strlen(cp) + 1));
				fdo = 0;
				saw_eq = false;
				/* must be 1/0 */
				tilde_ok = (f & (DOTILDE | DOASNTILDE)) ? 1 : 0;
				if (c == 0)
					return;
				Xinit(ds, dp, 128, ATEMP);
			} else if (c == 0) {
				return;
			} else if (type == XSUB && ctype(c, C_IFS) &&
			    !ctype(c, C_IFSWS) && Xlength(ds, dp) == 0) {
				*(cp = alloc(1, ATEMP)) = '\0';
				XPput(*wp, cp);
				type = XSUBMID;
			}
			if (word != IFS_NWS)
				word = ctype(c, C_IFSWS) ? IFS_WS : IFS_NWS;
		} else {
			if (type == XSUB) {
				if (word == IFS_NWS &&
				    Xlength(ds, dp) == 0) {
					*(cp = alloc(1, ATEMP)) = '\0';
					XPput(*wp, cp);
				}
				type = XSUBMID;
			}

			/* age tilde_ok info - ~ code tests second bit */
			tilde_ok <<= 1;
			/* mark any special second pass chars */
			if (!quote)
				switch (c) {
				case '[':
				case '!':
				case '-':
				case ']':
					/*
					 * For character classes - doesn't hurt
					 * to have magic !,-,]s outside of
					 * [...] expressions.
					 */
					if (f & (DOPAT | DOGLOB)) {
						fdo |= DOMAGIC;
						if (c == '[')
							fdo |= f & DOGLOB;
						*dp++ = MAGIC;
					}
					break;
				case '*':
				case '?':
					if (f & (DOPAT | DOGLOB)) {
						fdo |= DOMAGIC | (f & DOGLOB);
						*dp++ = MAGIC;
					}
					break;
				case '{':
				case '}':
				case ',':
					if ((f & DOBRACE) && (c == '{' /*}*/ ||
					    (fdo & DOBRACE))) {
						fdo |= DOBRACE|DOMAGIC;
						*dp++ = MAGIC;
					}
					break;
				case '=':
					/* Note first unquoted = for ~ */
					if (!(f & DOTEMP) && !saw_eq &&
					    (Flag(FBRACEEXPAND) ||
					    (f & DOASNTILDE))) {
						saw_eq = true;
						tilde_ok = 1;
					}
					break;
				case ':':
					/* : */
					/* Note unquoted : for ~ */
					if (!(f & DOTEMP) && (f & DOASNTILDE))
						tilde_ok = 1;
					break;
				case '~':
					/*
					 * tilde_ok is reset whenever
					 * any of ' " $( $(( ${ } are seen.
					 * Note that tilde_ok must be preserved
					 * through the sequence ${A=a=}~
					 */
					if (type == XBASE &&
					    (f & (DOTILDE | DOASNTILDE)) &&
					    (tilde_ok & 2)) {
						const char *tcp;
						char *tdp = dp;

						tcp = maybe_expand_tilde(sp,
						    &ds, &tdp,
						    tobool(f & DOASNTILDE));
						if (tcp) {
							if (dp != tdp)
								word = IFS_WORD;
							dp = tdp;
							sp = tcp;
							continue;
						}
					}
					break;
				}
			else
				/* undo temporary */
				quote &= ~2;

			if (make_magic) {
				make_magic = false;
				fdo |= DOMAGIC | (f & DOGLOB);
				*dp++ = MAGIC;
			} else if (ISMAGIC(c)) {
				fdo |= DOMAGIC;
				*dp++ = MAGIC;
			}
			/* save output char */
			*dp++ = c;
			word = IFS_WORD;
		}
	}
}

/*
 * Prepare to generate the string returned by ${} substitution.
 */
static int
varsub(Expand *xp, const char *sp, const char *word,
    int *stypep,	/* becomes qualifier type */
    int *slenp)		/* " " len (=, :=, etc.) valid iff *stypep != 0 */
{
	int c;
	int state;	/* next state: XBASE, XARG, XSUB, XNULLSUB */
	int stype;	/* substitution type */
	int slen;
	const char *p;
	struct tbl *vp;
	bool zero_ok = false;

	if ((stype = sp[0]) == '\0')
		/* Bad variable name */
		return (-1);

	xp->var = NULL;

	/*-
	 * ${#var}, string length (-U: characters, +U: octets) or array size
	 * ${%var}, string width (-U: screen columns, +U: octets)
	 */
	c = sp[1];
	if (stype == '%' && c == '\0')
		return (-1);
	if ((stype == '#' || stype == '%') && c != '\0') {
		/* Can't have any modifiers for ${#...} or ${%...} */
		if (*word != CSUBST)
			return (-1);
		sp++;
		/* Check for size of array */
		if ((p = cstrchr(sp, '[')) && (p[1] == '*' || p[1] == '@') &&
		    p[2] == ']') {
			int n = 0;

			if (stype != '#')
				return (-1);
			vp = global(arrayname(sp));
			if (vp->flag & (ISSET|ARRAY))
				zero_ok = true;
			for (; vp; vp = vp->u.array)
				if (vp->flag & ISSET)
					n++;
			c = n;
		} else if (c == '*' || c == '@') {
			if (stype != '#')
				return (-1);
			c = e->loc->argc;
		} else {
			p = str_val(global(sp));
			zero_ok = p != null;
			if (stype == '#')
				c = utflen(p);
			else {
				/* partial utf_mbswidth reimplementation */
				const char *s = p;
				unsigned int wc;
				size_t len;
				int cw;

				c = 0;
				while (*s) {
					if (!UTFMODE || (len = utf_mbtowc(&wc,
					    s)) == (size_t)-1)
						/* not UTFMODE or not UTF-8 */
						wc = (unsigned char)(*s++);
					else
						/* UTFMODE and UTF-8 */
						s += len;
					/* wc == char or wchar at s++ */
					if ((cw = utf_wcwidth(wc)) == -1) {
						/* 646, 8859-1, 10646 C0/C1 */
						c = -1;
						break;
					}
					c += cw;
				}
			}
		}
		if (Flag(FNOUNSET) && c == 0 && !zero_ok)
			errorf("%s: %s", sp, "parameter not set");
		/* unqualified variable/string substitution */
		*stypep = 0;
		xp->str = shf_smprintf("%d", c);
		return (XSUB);
	}

	/* Check for qualifiers in word part */
	stype = 0;
	c = word[slen = 0] == CHAR ? word[1] : 0;
	if (c == ':') {
		slen += 2;
		stype = 0x80;
		c = word[slen + 0] == CHAR ? word[slen + 1] : 0;
	}
	if (!stype && c == '/') {
		slen += 2;
		stype = c;
		if (word[slen] == ADELIM) {
			slen += 2;
			stype |= 0x80;
		}
	} else if (stype == 0x80 && (c == ' ' || c == '0')) {
		stype |= '0';
	} else if (ctype(c, C_SUBOP1)) {
		slen += 2;
		stype |= c;
	} else if (ctype(c, C_SUBOP2)) {
		/* Note: ksh88 allows :%, :%%, etc */
		slen += 2;
		stype = c;
		if (word[slen + 0] == CHAR && c == word[slen + 1]) {
			stype |= 0x80;
			slen += 2;
		}
	} else if (c == '@') {
		/* @x where x is command char */
		slen += 2;
		stype |= 0x100;
		if (word[slen] == CHAR) {
			stype |= word[slen + 1];
			slen += 2;
		}
	} else if (stype)
		/* : is not ok */
		return (-1);
	if (!stype && *word != CSUBST)
		return (-1);
	*stypep = stype;
	*slenp = slen;

	c = sp[0];
	if (c == '*' || c == '@') {
		switch (stype & 0x17F) {
		/* can't assign to a vector */
		case '=':
		/* can't trim a vector (yet) */
		case '%':
		case '#':
		case '0':
		case '/':
		case 0x100 | '#':
		case 0x100 | 'Q':
			return (-1);
		}
		if (e->loc->argc == 0) {
			xp->str = null;
			xp->var = global(sp);
			state = c == '@' ? XNULLSUB : XSUB;
		} else {
			xp->u.strv = (const char **)e->loc->argv + 1;
			xp->str = *xp->u.strv++;
			/* $@ */
			xp->split = tobool(c == '@');
			state = XARG;
		}
		/* POSIX 2009? */
		zero_ok = true;
	} else {
		if ((p = cstrchr(sp, '[')) && (p[1] == '*' || p[1] == '@') &&
		    p[2] == ']') {
			XPtrV wv;

			switch (stype & 0x17F) {
			/* can't assign to a vector */
			case '=':
			/* can't trim a vector (yet) */
			case '%':
			case '#':
			case '?':
			case '0':
			case '/':
			case 0x100 | '#':
			case 0x100 | 'Q':
				return (-1);
			}
			XPinit(wv, 32);
			if ((c = sp[0]) == '!')
				++sp;
			vp = global(arrayname(sp));
			for (; vp; vp = vp->u.array) {
				if (!(vp->flag&ISSET))
					continue;
				XPput(wv, c == '!' ? shf_smprintf("%lu",
				    arrayindex(vp)) :
				    str_val(vp));
			}
			if (XPsize(wv) == 0) {
				xp->str = null;
				state = p[1] == '@' ? XNULLSUB : XSUB;
				XPfree(wv);
			} else {
				XPput(wv, 0);
				xp->u.strv = (const char **)XPptrv(wv);
				xp->str = *xp->u.strv++;
				/* ${foo[@]} */
				xp->split = tobool(p[1] == '@');
				state = XARG;
			}
		} else {
			/* Can't assign things like $! or $1 */
			if ((stype & 0x17F) == '=' &&
			    ctype(*sp, C_VAR1 | C_DIGIT))
				return (-1);
			if (*sp == '!' && sp[1]) {
				++sp;
				xp->var = global(sp);
				if (vstrchr(sp, '['))
					xp->str = shf_smprintf("%s[%lu]",
					    xp->var->name,
					    arrayindex(xp->var));
				else
					xp->str = xp->var->name;
			} else {
				xp->var = global(sp);
				xp->str = str_val(xp->var);
			}
			state = XSUB;
		}
	}

	c = stype & 0x7F;
	/* test the compiler's code generator */
	if (((stype < 0x100) && (ctype(c, C_SUBOP2) || c == '/' ||
	    (((stype&0x80) ? *xp->str=='\0' : xp->str==null) ? /* undef? */
	    c == '=' || c == '-' || c == '?' : c == '+'))) ||
	    stype == (0x80 | '0') || stype == (0x100 | '#') ||
	    stype == (0x100 | 'Q'))
		/* expand word instead of variable value */
		state = XBASE;
	if (Flag(FNOUNSET) && xp->str == null && !zero_ok &&
	    (ctype(c, C_SUBOP2) || (state != XBASE && c != '+')))
		errorf("%s: %s", sp, "parameter not set");
	return (state);
}

/*
 * Run the command in $(...) and read its output.
 */
static int
comsub(Expand *xp, const char *cp, int fn MKSH_A_UNUSED)
{
	Source *s, *sold;
	struct op *t;
	struct shf *shf;
	uint8_t old_utfmode = UTFMODE;

	s = pushs(SSTRING, ATEMP);
	s->start = s->str = cp;
	sold = source;
	t = compile(s, true);
	afree(s, ATEMP);
	source = sold;

	UTFMODE = old_utfmode;

	if (t == NULL)
		return (XBASE);

	/* no waitlast() unless specifically enabled later */
	xp->split = false;

	if (t->type == TCOM &&
	    *t->args == NULL && *t->vars == NULL && t->ioact != NULL) {
		/* $(<file) */
		struct ioword *io = *t->ioact;
		char *name;

		if ((io->flag & IOTYPE) != IOREAD)
			errorf("%s: %s", "funny $() command",
			    snptreef(NULL, 32, "%R", io));
		shf = shf_open(name = evalstr(io->name, DOTILDE), O_RDONLY, 0,
			SHF_MAPHI|SHF_CLEXEC);
		if (shf == NULL)
			warningf(!Flag(FTALKING), "%s: %s %s: %s", name,
			    "can't open", "$(<...) input", cstrerror(errno));
	} else if (fn == FUNSUB) {
		int ofd1;
		struct temp *tf = NULL;

		/*
		 * create a temporary file, open for reading and writing,
		 * with an shf open for reading (buffered) but yet unused
		 */
		maketemp(ATEMP, TT_FUNSUB, &tf);
		if (!tf->shf) {
			errorf("can't %s temporary file %s: %s",
			    "create", tf->tffn, cstrerror(errno));
		}
		/* extract shf from temporary file, unlink and free it */
		shf = tf->shf;
		unlink(tf->tffn);
		afree(tf, ATEMP);
		/* save stdout and let it point to the tempfile */
		ofd1 = savefd(1);
		ksh_dup2(shf_fileno(shf), 1, false);
		/*
		 * run tree, with output thrown into the tempfile,
		 * in a new function block
		 */
		valsub(t, NULL);
		subst_exstat = exstat & 0xFF;
		/* rewind the tempfile and restore regular stdout */
		lseek(shf_fileno(shf), (off_t)0, SEEK_SET);
		restfd(1, ofd1);
	} else if (fn == VALSUB) {
		xp->str = valsub(t, ATEMP);
		subst_exstat = exstat & 0xFF;
		return (XSUB);
	} else {
		int ofd1, pv[2];

		openpipe(pv);
		shf = shf_fdopen(pv[0], SHF_RD, NULL);
		ofd1 = savefd(1);
		if (pv[1] != 1) {
			ksh_dup2(pv[1], 1, false);
			close(pv[1]);
		}
		execute(t, XXCOM | XPIPEO | XFORK, NULL);
		restfd(1, ofd1);
		startlast();
		/* waitlast() */
		xp->split = true;
	}

	xp->u.shf = shf;
	return (XCOM);
}

/*
 * perform #pattern and %pattern substitution in ${}
 */
static char *
trimsub(char *str, char *pat, int how)
{
	char *end = strnul(str);
	char *p, c;

	switch (how & 0xFF) {
	case '#':
		/* shortest match at beginning */
		for (p = str; p <= end; p += utf_ptradj(p)) {
			c = *p; *p = '\0';
			if (gmatchx(str, pat, false)) {
				*p = c;
				return (p);
			}
			*p = c;
		}
		break;
	case '#'|0x80:
		/* longest match at beginning */
		for (p = end; p >= str; p--) {
			c = *p; *p = '\0';
			if (gmatchx(str, pat, false)) {
				*p = c;
				return (p);
			}
			*p = c;
		}
		break;
	case '%':
		/* shortest match at end */
		p = end;
		while (p >= str) {
			if (gmatchx(p, pat, false))
				goto trimsub_match;
			if (UTFMODE) {
				char *op = p;
				while ((p-- > str) && ((*p & 0xC0) == 0x80))
					;
				if ((p < str) || (p + utf_ptradj(p) != op))
					p = op - 1;
			} else
				--p;
		}
		break;
	case '%'|0x80:
		/* longest match at end */
		for (p = str; p <= end; p++)
			if (gmatchx(p, pat, false)) {
 trimsub_match:
				strndupx(end, str, p - str, ATEMP);
				return (end);
			}
		break;
	}

	/* no match, return string */
	return (str);
}

/*
 * glob
 * Name derived from V6's /etc/glob, the program that expanded filenames.
 */

/* XXX cp not const 'cause slashes are temporarily replaced with NULs... */
static void
glob(char *cp, XPtrV *wp, bool markdirs)
{
	int oldsize = XPsize(*wp);

	if (glob_str(cp, wp, markdirs) == 0)
		XPput(*wp, debunk(cp, cp, strlen(cp) + 1));
	else
		qsort(XPptrv(*wp) + oldsize, XPsize(*wp) - oldsize,
		    sizeof(void *), xstrcmp);
}

#define GF_NONE		0
#define GF_EXCHECK	BIT(0)		/* do existence check on file */
#define GF_GLOBBED	BIT(1)		/* some globbing has been done */
#define GF_MARKDIR	BIT(2)		/* add trailing / to directories */

/*
 * Apply file globbing to cp and store the matching files in wp. Returns
 * the number of matches found.
 */
int
glob_str(char *cp, XPtrV *wp, bool markdirs)
{
	int oldsize = XPsize(*wp);
	XString xs;
	char *xp;

	Xinit(xs, xp, 256, ATEMP);
	globit(&xs, &xp, cp, wp, markdirs ? GF_MARKDIR : GF_NONE);
	Xfree(xs, xp);

	return (XPsize(*wp) - oldsize);
}

static void
globit(XString *xs,	/* dest string */
    char **xpp,		/* ptr to dest end */
    char *sp,		/* source path */
    XPtrV *wp,		/* output list */
    int check)		/* GF_* flags */
{
	char *np;		/* next source component */
	char *xp = *xpp;
	char *se;
	char odirsep;

	/* This to allow long expansions to be interrupted */
	intrcheck();

	if (sp == NULL) {
		/* end of source path */
		/*
		 * We only need to check if the file exists if a pattern
		 * is followed by a non-pattern (eg, foo*x/bar; no check
		 * is needed for foo* since the match must exist) or if
		 * any patterns were expanded and the markdirs option is set.
		 * Symlinks make things a bit tricky...
		 */
		if ((check & GF_EXCHECK) ||
		    ((check & GF_MARKDIR) && (check & GF_GLOBBED))) {
#define stat_check()	(stat_done ? stat_done : (stat_done = \
			    stat(Xstring(*xs, xp), &statb) < 0 ? -1 : 1))
			struct stat lstatb, statb;
			/* -1: failed, 1 ok, 0 not yet done */
			int stat_done = 0;

			if (mksh_lstat(Xstring(*xs, xp), &lstatb) < 0)
				return;
			/*
			 * special case for systems which strip trailing
			 * slashes from regular files (eg, /etc/passwd/).
			 * SunOS 4.1.3 does this...
			 */
			if ((check & GF_EXCHECK) && xp > Xstring(*xs, xp) &&
			    xp[-1] == '/' && !S_ISDIR(lstatb.st_mode) &&
			    (!S_ISLNK(lstatb.st_mode) ||
			    stat_check() < 0 || !S_ISDIR(statb.st_mode)))
				return;
			/*
			 * Possibly tack on a trailing / if there isn't already
			 * one and if the file is a directory or a symlink to a
			 * directory
			 */
			if (((check & GF_MARKDIR) && (check & GF_GLOBBED)) &&
			    xp > Xstring(*xs, xp) && xp[-1] != '/' &&
			    (S_ISDIR(lstatb.st_mode) ||
			    (S_ISLNK(lstatb.st_mode) && stat_check() > 0 &&
			    S_ISDIR(statb.st_mode)))) {
				*xp++ = '/';
				*xp = '\0';
			}
		}
		strndupx(np, Xstring(*xs, xp), Xlength(*xs, xp), ATEMP);
		XPput(*wp, np);
		return;
	}

	if (xp > Xstring(*xs, xp))
		*xp++ = '/';
	while (*sp == '/') {
		Xcheck(*xs, xp);
		*xp++ = *sp++;
	}
	np = strchr(sp, '/');
	if (np != NULL) {
		se = np;
		/* don't assume '/', can be multiple kinds */
		odirsep = *np;
		*np++ = '\0';
	} else {
		odirsep = '\0'; /* keep gcc quiet */
		se = sp + strlen(sp);
	}


	/*
	 * Check if sp needs globbing - done to avoid pattern checks for strings
	 * containing MAGIC characters, open [s without the matching close ],
	 * etc. (otherwise opendir() will be called which may fail because the
	 * directory isn't readable - if no globbing is needed, only execute
	 * permission should be required (as per POSIX)).
	 */
	if (!has_globbing(sp, se)) {
		XcheckN(*xs, xp, se - sp + 1);
		debunk(xp, sp, Xnleft(*xs, xp));
		xp += strlen(xp);
		*xpp = xp;
		globit(xs, xpp, np, wp, check);
	} else {
		DIR *dirp;
		struct dirent *d;
		char *name;
		size_t len, prefix_len;

		/* xp = *xpp;	copy_non_glob() may have re-alloc'd xs */
		*xp = '\0';
		prefix_len = Xlength(*xs, xp);
		dirp = opendir(prefix_len ? Xstring(*xs, xp) : ".");
		if (dirp == NULL)
			goto Nodir;
		while ((d = readdir(dirp)) != NULL) {
			name = d->d_name;
			if (name[0] == '.' &&
			    (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
				/* always ignore . and .. */
				continue;
			if ((*name == '.' && *sp != '.') ||
			    !gmatchx(name, sp, true))
				continue;

			len = strlen(d->d_name) + 1;
			XcheckN(*xs, xp, len);
			memcpy(xp, name, len);
			*xpp = xp + len - 1;
			globit(xs, xpp, np, wp,
				(check & GF_MARKDIR) | GF_GLOBBED
				| (np ? GF_EXCHECK : GF_NONE));
			xp = Xstring(*xs, xp) + prefix_len;
		}
		closedir(dirp);
 Nodir:
		;
	}

	if (np != NULL)
		*--np = odirsep;
}

/* remove MAGIC from string */
char *
debunk(char *dp, const char *sp, size_t dlen)
{
	char *d;
	const char *s;

	if ((s = cstrchr(sp, MAGIC))) {
		if (s - sp >= (ssize_t)dlen)
			return (dp);
		memmove(dp, sp, s - sp);
		for (d = dp + (s - sp); *s && (d - dp < (ssize_t)dlen); s++)
			if (!ISMAGIC(*s) || !(*++s & 0x80) ||
			    !vstrchr("*+?@! ", *s & 0x7f))
				*d++ = *s;
			else {
				/* extended pattern operators: *+?@! */
				if ((*s & 0x7f) != ' ')
					*d++ = *s & 0x7f;
				if (d - dp < (ssize_t)dlen)
					*d++ = '(';
			}
		*d = '\0';
	} else if (dp != sp)
		strlcpy(dp, sp, dlen);
	return (dp);
}

/*
 * Check if p is an unquoted name, possibly followed by a / or :. If so
 * puts the expanded version in *dcp,dp and returns a pointer in p just
 * past the name, otherwise returns 0.
 */
static const char *
maybe_expand_tilde(const char *p, XString *dsp, char **dpp, bool isassign)
{
	XString ts;
	char *dp = *dpp;
	char *tp;
	const char *r;

	Xinit(ts, tp, 16, ATEMP);
	/* : only for DOASNTILDE form */
	while (p[0] == CHAR && p[1] != '/' && (!isassign || p[1] != ':'))
	{
		Xcheck(ts, tp);
		*tp++ = p[1];
		p += 2;
	}
	*tp = '\0';
	r = (p[0] == EOS || p[0] == CHAR || p[0] == CSUBST) ?
	    do_tilde(Xstring(ts, tp)) : NULL;
	Xfree(ts, tp);
	if (r) {
		while (*r) {
			Xcheck(*dsp, dp);
			if (ISMAGIC(*r))
				*dp++ = MAGIC;
			*dp++ = *r++;
		}
		*dpp = dp;
		r = p;
	}
	return (r);
}

/*
 * tilde expansion
 *
 * based on a version by Arnold Robbins
 */

char *
do_tilde(char *cp)
{
	char *dp = null;

	if (cp[0] == '\0')
		dp = str_val(global("HOME"));
	else if (cp[0] == '+' && cp[1] == '\0')
		dp = str_val(global("PWD"));
	else if (cp[0] == '-' && cp[1] == '\0')
		dp = str_val(global("OLDPWD"));
#ifndef MKSH_NOPWNAM
	else
		dp = homedir(cp);
#endif
	/* If HOME, PWD or OLDPWD are not set, don't expand ~ */
	return (dp == null ? NULL : dp);
}

#ifndef MKSH_NOPWNAM
/*
 * map userid to user's home directory.
 * note that 4.3's getpw adds more than 6K to the shell,
 * and the YP version probably adds much more.
 * we might consider our own version of getpwnam() to keep the size down.
 */
static char *
homedir(char *name)
{
	struct tbl *ap;

	ap = ktenter(&homedirs, name, hash(name));
	if (!(ap->flag & ISSET)) {
		struct passwd *pw;

		pw = getpwnam(name);
		if (pw == NULL)
			return (NULL);
		strdupx(ap->val.s, pw->pw_dir, APERM);
		ap->flag |= DEFINED|ISSET|ALLOC;
	}
	return (ap->val.s);
}
#endif

static void
alt_expand(XPtrV *wp, char *start, char *exp_start, char *end, int fdo)
{
	unsigned int count = 0;
	char *brace_start, *brace_end, *comma = NULL;
	char *field_start;
	char *p = exp_start;

	/* search for open brace */
	while ((p = strchr(p, MAGIC)) && p[1] != '{' /*}*/)
		p += 2;
	brace_start = p;

	/* find matching close brace, if any */
	if (p) {
		comma = NULL;
		count = 1;
		p += 2;
		while (*p && count) {
			if (ISMAGIC(*p++)) {
				if (*p == '{' /*}*/)
					++count;
				else if (*p == /*{*/ '}')
					--count;
				else if (*p == ',' && count == 1)
					comma = p;
				++p;
			}
		}
	}
	/* no valid expansions... */
	if (!p || count != 0) {
		/*
		 * Note that given a{{b,c} we do not expand anything (this is
		 * what AT&T ksh does. This may be changed to do the {b,c}
		 * expansion. }
		 */
		if (fdo & DOGLOB)
			glob(start, wp, tobool(fdo & DOMARKDIRS));
		else
			XPput(*wp, debunk(start, start, end - start));
		return;
	}
	brace_end = p;
	if (!comma) {
		alt_expand(wp, start, brace_end, end, fdo);
		return;
	}

	/* expand expression */
	field_start = brace_start + 2;
	count = 1;
	for (p = brace_start + 2; p != brace_end; p++) {
		if (ISMAGIC(*p)) {
			if (*++p == '{' /*}*/)
				++count;
			else if ((*p == /*{*/ '}' && --count == 0) ||
			    (*p == ',' && count == 1)) {
				char *news;
				int l1, l2, l3;

				/*
				 * addition safe since these operate on
				 * one string (separate substrings)
				 */
				l1 = brace_start - start;
				l2 = (p - 1) - field_start;
				l3 = end - brace_end;
				news = alloc(l1 + l2 + l3 + 1, ATEMP);
				memcpy(news, start, l1);
				memcpy(news + l1, field_start, l2);
				memcpy(news + l1 + l2, brace_end, l3);
				news[l1 + l2 + l3] = '\0';
				alt_expand(wp, news, news + l1,
				    news + l1 + l2 + l3, fdo);
				field_start = p + 1;
			}
		}
	}
	return;
}

/* helper function due to setjmp/longjmp woes */
static char *
valsub(struct op *t, Area *ap)
{
	char * volatile cp = NULL;
	struct tbl * volatile vp = NULL;

	newenv(E_FUNC);
	newblock();
	if (ap)
		vp = local("REPLY", false);
	if (!kshsetjmp(e->jbuf))
		execute(t, XXCOM | XERROK, NULL);
	if (vp)
		strdupx(cp, str_val(vp), ap);
	quitenv(NULL);

	return (cp);
}
