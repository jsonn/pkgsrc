/*	$OpenBSD: history.c,v 1.40 2014/11/20 15:22:39 tedu Exp $	*/
/*	$OpenBSD: trap.c,v 1.23 2010/05/19 17:36:08 jasper Exp $	*/

/*-
 * Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 *		 2011, 2012, 2014, 2015
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
#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

__RCSID("$MirOS: src/bin/mksh/histrap.c,v 1.134.2.3 2015/03/01 15:43:00 tg Exp $");

Trap sigtraps[NSIG + 1];
static struct sigaction Sigact_ign;

#if HAVE_PERSISTENT_HISTORY
static int histload(Source *, unsigned char *, size_t);
static int writehistline(int, int, const char *);
static void writehistfile(int, const char *);
#endif

static int hist_execute(char *);
static char **hist_get(const char *, bool, bool);
static char **hist_get_oldest(void);

static bool hstarted;		/* set after hist_init() called */
static Source *hist_source;

#if HAVE_PERSISTENT_HISTORY
/*XXX imake style */
#if defined(__linux)
#define caddr_cast(x)	((void *)(x))
#else
#define caddr_cast(x)	((caddr_t)(x))
#endif

/* several OEs do not have these constants */
#ifndef MAP_FAILED
#define MAP_FAILED	caddr_cast(-1)
#endif

/* some OEs need the default mapping type specified */
#ifndef MAP_FILE
#define MAP_FILE	0
#endif

/* current history file: name, fd, size */
static char *hname;
static int histfd = -1;
static off_t histfsize;
#endif

static const char Tnot_in_history[] = "not in history";
#define Thistory (Tnot_in_history + 7)

static const char TFCEDIT_dollaru[] = "${FCEDIT:-/bin/ed} $_";
#define Tspdollaru (TFCEDIT_dollaru + 18)

/* HISTSIZE default: size of saved history, persistent or standard */
#ifdef MKSH_SMALL
#define MKSH_DEFHISTSIZE	255
#else
#define MKSH_DEFHISTSIZE	2047
#endif
/* maximum considered size of persistent history file */
#define MKSH_MAXHISTFSIZE	((off_t)1048576 * 96)

int
c_fc(const char **wp)
{
	struct shf *shf;
	struct temp *tf;
	bool gflag = false, lflag = false, nflag = false, rflag = false,
	    sflag = false;
	int optc;
	const char *p, *first = NULL, *last = NULL;
	char **hfirst, **hlast, **hp, *editor = NULL;

	if (!Flag(FTALKING_I)) {
		bi_errorf("history %ss not available", Tfunction);
		return (1);
	}

	while ((optc = ksh_getopt(wp, &builtin_opt,
	    "e:glnrs0,1,2,3,4,5,6,7,8,9,")) != -1)
		switch (optc) {

		case 'e':
			p = builtin_opt.optarg;
			if (ksh_isdash(p))
				sflag = true;
			else {
				size_t len = strlen(p);

				/* almost certainly not overflowing */
				editor = alloc(len + 4, ATEMP);
				memcpy(editor, p, len);
				memcpy(editor + len, Tspdollaru, 4);
			}
			break;

		/* non-AT&T ksh */
		case 'g':
			gflag = true;
			break;

		case 'l':
			lflag = true;
			break;

		case 'n':
			nflag = true;
			break;

		case 'r':
			rflag = true;
			break;

		/* POSIX version of -e - */
		case 's':
			sflag = true;
			break;

		/* kludge city - accept -num as -- -num (kind of) */
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			p = shf_smprintf("-%c%s",
					optc, builtin_opt.optarg);
			if (!first)
				first = p;
			else if (!last)
				last = p;
			else {
				bi_errorf("too many arguments");
				return (1);
			}
			break;

		case '?':
			return (1);
		}
	wp += builtin_opt.optind;

	/* Substitute and execute command */
	if (sflag) {
		char *pat = NULL, *rep = NULL, *line;

		if (editor || lflag || nflag || rflag) {
			bi_errorf("can't use -e, -l, -n, -r with -s (-e -)");
			return (1);
		}

		/* Check for pattern replacement argument */
		if (*wp && **wp && (p = cstrchr(*wp + 1, '='))) {
			strdupx(pat, *wp, ATEMP);
			rep = pat + (p - *wp);
			*rep++ = '\0';
			wp++;
		}
		/* Check for search prefix */
		if (!first && (first = *wp))
			wp++;
		if (last || *wp) {
			bi_errorf("too many arguments");
			return (1);
		}

		hp = first ? hist_get(first, false, false) :
		    hist_get_newest(false);
		if (!hp)
			return (1);
		/* hist_replace */
		if (!pat)
			strdupx(line, *hp, ATEMP);
		else {
			char *s, *s1;
			size_t len, pat_len, rep_len;
			XString xs;
			char *xp;
			bool any_subst = false;

			pat_len = strlen(pat);
			rep_len = strlen(rep);
			Xinit(xs, xp, 128, ATEMP);
			for (s = *hp; (s1 = strstr(s, pat)) &&
			    (!any_subst || gflag); s = s1 + pat_len) {
				any_subst = true;
				len = s1 - s;
				XcheckN(xs, xp, len + rep_len);
				/*; first part */
				memcpy(xp, s, len);
				xp += len;
				/* replacement */
				memcpy(xp, rep, rep_len);
				xp += rep_len;
			}
			if (!any_subst) {
				bi_errorf("bad substitution");
				return (1);
			}
			len = strlen(s) + 1;
			XcheckN(xs, xp, len);
			memcpy(xp, s, len);
			xp += len;
			line = Xclose(xs, xp);
		}
		return (hist_execute(line));
	}

	if (editor && (lflag || nflag)) {
		bi_errorf("can't use -l, -n with -e");
		return (1);
	}

	if (!first && (first = *wp))
		wp++;
	if (!last && (last = *wp))
		wp++;
	if (*wp) {
		bi_errorf("too many arguments");
		return (1);
	}
	if (!first) {
		hfirst = lflag ? hist_get("-16", true, true) :
		    hist_get_newest(false);
		if (!hfirst)
			return (1);
		/* can't fail if hfirst didn't fail */
		hlast = hist_get_newest(false);
	} else {
		/*
		 * POSIX says not an error if first/last out of bounds
		 * when range is specified; AT&T ksh and pdksh allow out
		 * of bounds for -l as well.
		 */
		hfirst = hist_get(first, tobool(lflag || last), lflag);
		if (!hfirst)
			return (1);
		hlast = last ? hist_get(last, true, lflag) :
		    (lflag ? hist_get_newest(false) : hfirst);
		if (!hlast)
			return (1);
	}
	if (hfirst > hlast) {
		char **temp;

		temp = hfirst; hfirst = hlast; hlast = temp;
		/* POSIX */
		rflag = !rflag;
	}

	/* List history */
	if (lflag) {
		char *s, *t;

		for (hp = rflag ? hlast : hfirst;
		    hp >= hfirst && hp <= hlast; hp += rflag ? -1 : 1) {
			if (!nflag)
				shf_fprintf(shl_stdout, "%d",
				    hist_source->line - (int)(histptr - hp));
			shf_putc('\t', shl_stdout);
			/* print multi-line commands correctly */
			s = *hp;
			while ((t = strchr(s, '\n'))) {
				*t = '\0';
				shf_fprintf(shl_stdout, "%s\n\t", s);
				*t++ = '\n';
				s = t;
			}
			shf_fprintf(shl_stdout, "%s\n", s);
		}
		shf_flush(shl_stdout);
		return (0);
	}

	/* Run editor on selected lines, then run resulting commands */

	tf = maketemp(ATEMP, TT_HIST_EDIT, &e->temps);
	if (!(shf = tf->shf)) {
		bi_errorf("can't %s temporary file %s: %s",
		    "create", tf->tffn, cstrerror(errno));
		return (1);
	}
	for (hp = rflag ? hlast : hfirst;
	    hp >= hfirst && hp <= hlast; hp += rflag ? -1 : 1)
		shf_fprintf(shf, "%s\n", *hp);
	if (shf_close(shf) == -1) {
		bi_errorf("can't %s temporary file %s: %s",
		    "write", tf->tffn, cstrerror(errno));
		return (1);
	}

	/* Ignore setstr errors here (arbitrary) */
	setstr(local("_", false), tf->tffn, KSH_RETURN_ERROR);

	/* XXX: source should not get trashed by this.. */
	{
		Source *sold = source;
		int ret;

		ret = command(editor ? editor : TFCEDIT_dollaru, 0);
		source = sold;
		if (ret)
			return (ret);
	}

	{
		struct stat statb;
		XString xs;
		char *xp;
		ssize_t n;

		if (!(shf = shf_open(tf->tffn, O_RDONLY, 0, 0))) {
			bi_errorf("can't %s temporary file %s: %s",
			    "open", tf->tffn, cstrerror(errno));
			return (1);
		}

		if (stat(tf->tffn, &statb) < 0)
			n = 128;
		else if ((off_t)statb.st_size > MKSH_MAXHISTFSIZE) {
			bi_errorf("%s %s too large: %lu", Thistory,
			    "file", (unsigned long)statb.st_size);
			goto errout;
		} else
			n = (size_t)statb.st_size + 1;
		Xinit(xs, xp, n, hist_source->areap);
		while ((n = shf_read(xp, Xnleft(xs, xp), shf)) > 0) {
			xp += n;
			if (Xnleft(xs, xp) <= 0)
				XcheckN(xs, xp, Xlength(xs, xp));
		}
		if (n < 0) {
			bi_errorf("can't %s temporary file %s: %s",
			    "read", tf->tffn, cstrerror(shf_errno(shf)));
 errout:
			shf_close(shf);
			return (1);
		}
		shf_close(shf);
		*xp = '\0';
		strip_nuls(Xstring(xs, xp), Xlength(xs, xp));
		return (hist_execute(Xstring(xs, xp)));
	}
}

/* Save cmd in history, execute cmd (cmd gets trashed) */
static int
hist_execute(char *cmd)
{
	static int last_line = -1;
	Source *sold;
	int ret;
	char *p, *q;

	/* Back up over last histsave */
	if (histptr >= history && last_line != hist_source->line) {
		hist_source->line--;
		afree(*histptr, APERM);
		histptr--;
		last_line = hist_source->line;
	}

	for (p = cmd; p; p = q) {
		if ((q = strchr(p, '\n'))) {
			/* kill the newline */
			*q++ = '\0';
			if (!*q)
				/* ignore trailing newline */
				q = NULL;
		}
		histsave(&hist_source->line, p, true, true);

		/* POSIX doesn't say this is done... */
		shellf("%s\n", p);
		if (q)
			/* restore \n (trailing \n not restored) */
			q[-1] = '\n';
	}

	/*-
	 * Commands are executed here instead of pushing them onto the
	 * input 'cause POSIX says the redirection and variable assignments
	 * in
	 *	X=y fc -e - 42 2> /dev/null
	 * are to effect the repeated commands environment.
	 */
	/* XXX: source should not get trashed by this.. */
	sold = source;
	ret = command(cmd, 0);
	source = sold;
	return (ret);
}

/*
 * get pointer to history given pattern
 * pattern is a number or string
 */
static char **
hist_get(const char *str, bool approx, bool allow_cur)
{
	char **hp = NULL;
	int n;

	if (getn(str, &n)) {
		hp = histptr + (n < 0 ? n : (n - hist_source->line));
		if ((size_t)hp < (size_t)history) {
			if (approx)
				hp = hist_get_oldest();
			else {
				bi_errorf("%s: %s", str, Tnot_in_history);
				hp = NULL;
			}
		} else if ((size_t)hp > (size_t)histptr) {
			if (approx)
				hp = hist_get_newest(allow_cur);
			else {
				bi_errorf("%s: %s", str, Tnot_in_history);
				hp = NULL;
			}
		} else if (!allow_cur && hp == histptr) {
			bi_errorf("%s: %s", str, "invalid range");
			hp = NULL;
		}
	} else {
		bool anchored = *str == '?' ? (++str, false) : true;

		/* the -1 is to avoid the current fc command */
		if ((n = findhist(histptr - history - 1, 0, str, anchored)) < 0)
			bi_errorf("%s: %s", str, Tnot_in_history);
		else
			hp = &history[n];
	}
	return (hp);
}

/* Return a pointer to the newest command in the history */
char **
hist_get_newest(bool allow_cur)
{
	if (histptr < history || (!allow_cur && histptr == history)) {
		bi_errorf("no history (yet)");
		return (NULL);
	}
	return (allow_cur ? histptr : histptr - 1);
}

/* Return a pointer to the oldest command in the history */
static char **
hist_get_oldest(void)
{
	if (histptr <= history) {
		bi_errorf("no history (yet)");
		return (NULL);
	}
	return (history);
}

#if !defined(MKSH_NO_CMDLINE_EDITING) && !MKSH_S_NOVI
/* current position in history[] */
static char **current;

/*
 * Return the current position.
 */
char **
histpos(void)
{
	return (current);
}

int
histnum(int n)
{
	int last = histptr - history;

	if (n < 0 || n >= last) {
		current = histptr;
		return (last);
	} else {
		current = &history[n];
		return (n);
	}
}
#endif

/*
 * This will become unnecessary if hist_get is modified to allow
 * searching from positions other than the end, and in either
 * direction.
 */
int
findhist(int start, int fwd, const char *str, bool anchored)
{
	char **hp;
	int maxhist = histptr - history;
	int incr = fwd ? 1 : -1;
	size_t len = strlen(str);

	if (start < 0 || start >= maxhist)
		start = maxhist;

	hp = &history[start];
	for (; hp >= history && hp <= histptr; hp += incr)
		if ((anchored && strncmp(*hp, str, len) == 0) ||
		    (!anchored && strstr(*hp, str)))
			return (hp - history);

	return (-1);
}

/*
 * set history; this means reallocating the dataspace
 */
void
sethistsize(mksh_ari_t n)
{
	if (n > 0 && n != histsize) {
		int cursize = histptr - history;

		/* save most recent history */
		if (n < cursize) {
			memmove(history, histptr - n + 1, n * sizeof(char *));
			cursize = n - 1;
		}

		history = aresize2(history, n, sizeof(char *), APERM);

		histsize = n;
		histptr = history + cursize;
	}
}

#if HAVE_PERSISTENT_HISTORY
/*
 * set history file; this can mean reloading/resetting/starting
 * history file maintenance
 */
void
sethistfile(const char *name)
{
	/* if not started then nothing to do */
	if (hstarted == false)
		return;

	/* if the name is the same as the name we have */
	if (hname && strcmp(hname, name) == 0)
		return;

	/*
	 * it's a new name - possibly
	 */
	if (histfd != -1) {
		/* yes the file is open */
		(void)close(histfd);
		histfd = -1;
		histfsize = 0;
		afree(hname, APERM);
		hname = NULL;
		/* let's reset the history */
		histptr = history - 1;
		hist_source->line = 0;
	}

	hist_init(hist_source);
}
#endif

/*
 * initialise the history vector
 */
void
init_histvec(void)
{
	if (history == (char **)NULL) {
		histsize = MKSH_DEFHISTSIZE;
		history = alloc2(histsize, sizeof(char *), APERM);
		histptr = history - 1;
	}
}


/*
 * It turns out that there is a lot of ghastly hackery here
 */

#if !defined(MKSH_SMALL) && HAVE_PERSISTENT_HISTORY
/* do not save command in history but possibly sync */
bool
histsync(void)
{
	bool changed = false;

	if (histfd != -1) {
		int lno = hist_source->line;

		hist_source->line++;
		writehistfile(0, NULL);
		hist_source->line--;

		if (lno != hist_source->line)
			changed = true;
	}

	return (changed);
}
#endif

/*
 * save command in history
 */
void
histsave(int *lnp, const char *cmd, bool dowrite MKSH_A_UNUSED, bool ignoredups)
{
	char **hp;
	char *c, *cp;

	strdupx(c, cmd, APERM);
	if ((cp = strchr(c, '\n')) != NULL)
		*cp = '\0';

	if (ignoredups && !strcmp(c, *histptr)
#if !defined(MKSH_SMALL) && HAVE_PERSISTENT_HISTORY
	    && !histsync()
#endif
	    ) {
		afree(c, APERM);
		return;
	}
	++*lnp;

#if HAVE_PERSISTENT_HISTORY
	if (dowrite && histfd != -1)
		writehistfile(*lnp, c);
#endif

	hp = histptr;

	if (++hp >= history + histsize) {
		/* remove oldest command */
		afree(*history, APERM);
		for (hp = history; hp < history + histsize - 1; hp++)
			hp[0] = hp[1];
	}
	*hp = c;
	histptr = hp;
}

/*
 * Write history data to a file nominated by HISTFILE;
 * if HISTFILE is unset then history still happens, but
 * the data is not written to a file. All copies of ksh
 * looking at the file will maintain the same history.
 * This is ksh behaviour.
 *
 * This stuff uses mmap()
 *
 * This stuff is so totally broken it must eventually be
 * redesigned, without mmap, better checks, support for
 * larger files, etc. and handle partially corrupted files
 */

/*-
 * Open a history file
 * Format is:
 * Bytes 1, 2:
 *	HMAGIC - just to check that we are dealing with the correct object
 * Then follows a number of stored commands
 * Each command is
 *	<command byte><command number(4 octets, big endian)><bytes><NUL>
 */
#define HMAGIC1		0xAB
#define HMAGIC2		0xCD
#define COMMAND		0xFF

#if HAVE_PERSISTENT_HISTORY
static const unsigned char sprinkle[2] = { HMAGIC1, HMAGIC2 };
#endif

void
hist_init(Source *s)
{
#if HAVE_PERSISTENT_HISTORY
	unsigned char *base;
	int lines, fd;
	enum { hist_init_first, hist_init_retry, hist_init_restore } hs;
#endif

	if (Flag(FTALKING) == 0)
		return;

	hstarted = true;
	hist_source = s;

#if HAVE_PERSISTENT_HISTORY
	if ((hname = str_val(global("HISTFILE"))) == NULL)
		return;
	strdupx(hname, hname, APERM);
	hs = hist_init_first;

 retry:
	/* we have a file and are interactive */
	if ((fd = open(hname, O_RDWR | O_CREAT | O_APPEND | O_BINARY,
	    0600)) < 0)
		return;

	histfd = savefd(fd);
	if (histfd != fd)
		close(fd);

	mksh_lockfd(histfd);

	histfsize = lseek(histfd, (off_t)0, SEEK_END);
	if (histfsize > MKSH_MAXHISTFSIZE || hs == hist_init_restore) {
		/* we ignore too large files but still append to them */
		/* we also don't need to re-read after truncation */
		goto hist_init_tail;
	} else if (histfsize > 2) {
		/* we have some data, check its validity */
		base = (void *)mmap(NULL, (size_t)histfsize, PROT_READ,
		    MAP_FILE | MAP_PRIVATE, histfd, (off_t)0);
		if (base == (unsigned char *)MAP_FAILED)
			goto hist_init_fail;
		if (base[0] != HMAGIC1 || base[1] != HMAGIC2) {
			munmap(caddr_cast(base), (size_t)histfsize);
			goto hist_init_fail;
		}
		/* load _all_ data */
		lines = histload(hist_source, base + 2, (size_t)histfsize - 2);
		munmap(caddr_cast(base), (size_t)histfsize);
		/* check if the file needs to be truncated */
		if (lines > histsize && histptr >= history) {
			/* you're fucked up with the current code, trust me */
			char *nhname, **hp;
			struct stat sb;

			/* create temporary file */
			nhname = shf_smprintf("%s.%d", hname, (int)procpid);
			if ((fd = open(nhname, O_RDWR | O_CREAT | O_TRUNC |
			    O_EXCL | O_BINARY, 0600)) < 0) {
				/* just don't truncate then, meh. */
				goto hist_trunc_dont;
			}
			if (fstat(histfd, &sb) >= 0 &&
			    chown(nhname, sb.st_uid, sb.st_gid)) {
				/* abort the truncation then, meh. */
				goto hist_trunc_abort;
			}
			/* we definitively want some magic in that file */
			if (write(fd, sprinkle, 2) != 2)
				goto hist_trunc_abort;
			/* and of course the entries */
			hp = history;
			while (hp < histptr) {
				if (!writehistline(fd,
				    s->line - (histptr - hp), *hp))
					goto hist_trunc_abort;
				++hp;
			}
			/* now unlock, close both, rename, rinse, repeat */
			close(fd);
			fd = -1;
			hist_finish();
			if (rename(nhname, hname) < 0) {
 hist_trunc_abort:
				if (fd != -1)
					close(fd);
				unlink(nhname);
				if (fd != -1)
					goto hist_trunc_dont;
				/* darn! restore histfd and pray */
			}
			hs = hist_init_restore;
 hist_trunc_dont:
			afree(nhname, ATEMP);
			if (hs == hist_init_restore)
				goto retry;
		}
	} else if (histfsize != 0) {
		/* negative or too small... */
 hist_init_fail:
		/* ... or mmap failed or illegal */
		hist_finish();
		/* nuke the bogus file then retry, at most once */
		if (!unlink(hname) && hs != hist_init_retry) {
			hs = hist_init_retry;
			goto retry;
		}
		if (hs != hist_init_retry)
			bi_errorf("can't %s %s: %s",
			    "unlink HISTFILE", hname, cstrerror(errno));
		histfsize = 0;
		return;
	} else {
		/* size 0, add magic to the history file */
		if (write(histfd, sprinkle, 2) != 2) {
			hist_finish();
			return;
		}
	}
	histfsize = lseek(histfd, (off_t)0, SEEK_END);
 hist_init_tail:
	mksh_unlkfd(histfd);
#endif
}

#if HAVE_PERSISTENT_HISTORY
/*
 * load the history structure from the stored data
 */
static int
histload(Source *s, unsigned char *base, size_t bytes)
{
	int lno = 0, lines = 0;
	unsigned char *cp;

 histload_loop:
	/* !bytes check as some systems (older FreeBSDs) have buggy memchr */
	if (!bytes || (cp = memchr(base, COMMAND, bytes)) == NULL)
		return (lines);
	/* advance base pointer past COMMAND byte */
	bytes -= ++cp - base;
	base = cp;
	/* if there is no full string left, don't bother with the rest */
	if (bytes < 5 || (cp = memchr(base + 4, '\0', bytes - 4)) == NULL)
		return (lines);
	/* load the stored line number */
	lno = ((base[0] & 0xFF) << 24) | ((base[1] & 0xFF) << 16) |
	    ((base[2] & 0xFF) << 8) | (base[3] & 0xFF);
	/* store away the found line (@base[4]) */
	++lines;
	if (histptr >= history && lno - 1 != s->line) {
		/* a replacement? */
		char **hp;

		if (lno >= s->line - (histptr - history) && lno <= s->line) {
			hp = &histptr[lno - s->line];
			if (*hp)
				afree(*hp, APERM);
			strdupx(*hp, (char *)(base + 4), APERM);
		}
	} else {
		s->line = lno--;
		histsave(&lno, (char *)(base + 4), false, false);
	}
	/* advance base pointer past NUL */
	bytes -= ++cp - base;
	base = cp;
	/* repeat until no more */
	goto histload_loop;
}

/*
 * write a command to the end of the history file
 *
 * This *MAY* seem easy but it's also necessary to check
 * that the history file has not changed in size.
 * If it has - then some other shell has written to it and
 * we should (re)read those commands to update our history
 */
static void
writehistfile(int lno, const char *cmd)
{
	off_t sizenow;
	size_t bytes;
	unsigned char *base, *news;

	mksh_lockfd(histfd);
	sizenow = lseek(histfd, (off_t)0, SEEK_END);
	if (sizenow < histfsize) {
		/* the file has shrunk; give up */
		goto bad;
	}
	if (
		/* ignore changes when the file is too large */
		sizenow <= MKSH_MAXHISTFSIZE
	    &&
		/* the size has changed, we need to do read updates */
		sizenow > histfsize
	    ) {
		/* both sizenow and histfsize are <= MKSH_MAXHISTFSIZE */
		bytes = (size_t)(sizenow - histfsize);
		base = (void *)mmap(NULL, (size_t)sizenow, PROT_READ,
		    MAP_FILE | MAP_PRIVATE, histfd, (off_t)0);
		if (base == (unsigned char *)MAP_FAILED)
			goto bad;
		news = base + (size_t)histfsize;
		if (*news == COMMAND) {
			hist_source->line--;
			histload(hist_source, news, bytes);
			hist_source->line++;
			lno = hist_source->line;
		} else
			bytes = 0;
		munmap(caddr_cast(base), (size_t)sizenow);
		if (!bytes)
			goto bad;
	}
	if (cmd && !writehistline(histfd, lno, cmd)) {
 bad:
		hist_finish();
		return;
	}
	histfsize = lseek(histfd, (off_t)0, SEEK_END);
	mksh_unlkfd(histfd);
}

static int
writehistline(int fd, int lno, const char *cmd)
{
	ssize_t n;
	unsigned char hdr[5];

	hdr[0] = COMMAND;
	hdr[1] = (lno >> 24) & 0xFF;
	hdr[2] = (lno >> 16) & 0xFF;
	hdr[3] = (lno >> 8) & 0xFF;
	hdr[4] = lno & 0xFF;
	n = strlen(cmd) + 1;
	return (write(fd, hdr, 5) == 5 && write(fd, cmd, n) == n);
}

void
hist_finish(void)
{
	if (histfd >= 0) {
		mksh_unlkfd(histfd);
		(void)close(histfd);
	}
	histfd = -1;
}
#endif


#if !HAVE_SYS_SIGNAME
static const struct mksh_sigpair {
	const char * const name;
	int nr;
} mksh_sigpairs[] = {
#include "signames.inc"
	{ NULL, 0 }
};
#endif

#if HAVE_SYS_SIGLIST
#if !HAVE_SYS_SIGLIST_DECL
extern const char * const sys_siglist[];
#endif
#endif

void
inittraps(void)
{
	int i;
	const char *cs;

	trap_exstat = -1;

	/* Populate sigtraps based on sys_signame and sys_siglist. */
	/*XXX this is idiotic, use a multi-key/value hashtable! */
	for (i = 0; i <= NSIG; i++) {
		sigtraps[i].signal = i;
		if (i == ksh_SIGERR) {
			sigtraps[i].name = "ERR";
			sigtraps[i].mess = "Error handler";
		} else {
#if HAVE_SYS_SIGNAME
			cs = sys_signame[i];
#else
			const struct mksh_sigpair *pair = mksh_sigpairs;
			while ((pair->nr != i) && (pair->name != NULL))
				++pair;
			cs = pair->name;
#endif
			if ((cs == NULL) ||
			    (cs[0] == '\0'))
				sigtraps[i].name = shf_smprintf("%d", i);
			else {
				char *s;

				/* this is not optimal, what about SIGSIG1? */
				if ((cs[0] & 0xDF) == 'S' &&
				    (cs[1] & 0xDF) == 'I' &&
				    (cs[2] & 0xDF) == 'G' &&
				    cs[3] != '\0') {
					/* skip leading "SIG" */
					cs += 3;
				}
				strdupx(s, cs, APERM);
				sigtraps[i].name = s;
				while ((*s = ksh_toupper(*s)))
					++s;
			}
#if HAVE_SYS_SIGLIST
			sigtraps[i].mess = sys_siglist[i];
#elif HAVE_STRSIGNAL
			sigtraps[i].mess = strsignal(i);
#else
			sigtraps[i].mess = NULL;
#endif
			if ((sigtraps[i].mess == NULL) ||
			    (sigtraps[i].mess[0] == '\0'))
				sigtraps[i].mess = shf_smprintf("%s %d",
				    "Signal", i);
		}
	}
	/* our name for signal 0 */
	sigtraps[ksh_SIGEXIT].name = "EXIT";

	(void)sigemptyset(&Sigact_ign.sa_mask);
	Sigact_ign.sa_flags = 0; /* interruptible */
	Sigact_ign.sa_handler = SIG_IGN;

	sigtraps[SIGINT].flags |= TF_DFL_INTR | TF_TTY_INTR;
	sigtraps[SIGQUIT].flags |= TF_DFL_INTR | TF_TTY_INTR;
	/* SIGTERM is not fatal for interactive */
	sigtraps[SIGTERM].flags |= TF_DFL_INTR;
	sigtraps[SIGHUP].flags |= TF_FATAL;
	sigtraps[SIGCHLD].flags |= TF_SHELL_USES;

	/* these are always caught so we can clean up any temporary files. */
	setsig(&sigtraps[SIGINT], trapsig, SS_RESTORE_ORIG);
	setsig(&sigtraps[SIGQUIT], trapsig, SS_RESTORE_ORIG);
	setsig(&sigtraps[SIGTERM], trapsig, SS_RESTORE_ORIG);
	setsig(&sigtraps[SIGHUP], trapsig, SS_RESTORE_ORIG);
}

static void alarm_catcher(int sig);

void
alarm_init(void)
{
	sigtraps[SIGALRM].flags |= TF_SHELL_USES;
	setsig(&sigtraps[SIGALRM], alarm_catcher,
		SS_RESTORE_ORIG|SS_FORCE|SS_SHTRAP);
}

/* ARGSUSED */
static void
alarm_catcher(int sig MKSH_A_UNUSED)
{
	/* this runs inside interrupt context, with errno saved */

	if (ksh_tmout_state == TMOUT_READING) {
		int left = alarm(0);

		if (left == 0) {
			ksh_tmout_state = TMOUT_LEAVING;
			intrsig = 1;
		} else
			alarm(left);
	}
}

Trap *
gettrap(const char *cs, bool igncase)
{
	int i;
	Trap *p;
	char *as;

	if (ksh_isdigit(*cs)) {
		return ((getn(cs, &i) && 0 <= i && i < NSIG) ?
		    (&sigtraps[i]) : NULL);
	}

	/* this breaks SIGSIG1, but we do that above anyway */
	if ((cs[0] & 0xDF) == 'S' &&
	    (cs[1] & 0xDF) == 'I' &&
	    (cs[2] & 0xDF) == 'G' &&
	    cs[3] != '\0') {
		/* skip leading "SIG" */
		cs += 3;
	}
	if (igncase) {
		char *s;

		strdupx(as, cs, ATEMP);
		cs = s = as;
		while ((*s = ksh_toupper(*s)))
			++s;
	} else
		as = NULL;

	p = sigtraps;
	for (i = 0; i <= NSIG; i++) {
		if (!strcmp(p->name, cs))
			goto found;
		++p;
	}
	p = NULL;
 found:
	afree(as, ATEMP);
	return (p);
}

/*
 * trap signal handler
 */
void
trapsig(int i)
{
	Trap *p = &sigtraps[i];
	int eno = errno;

	trap = p->set = 1;
	if (p->flags & TF_DFL_INTR)
		intrsig = 1;
	if ((p->flags & TF_FATAL) && !p->trap) {
		fatal_trap = 1;
		intrsig = 1;
	}
	if (p->shtrap)
		(*p->shtrap)(i);
	errno = eno;
}

/*
 * called when we want to allow the user to ^C out of something - won't
 * work if user has trapped SIGINT.
 */
void
intrcheck(void)
{
	if (intrsig)
		runtraps(TF_DFL_INTR|TF_FATAL);
}

/*
 * called after EINTR to check if a signal with normally causes process
 * termination has been received.
 */
int
fatal_trap_check(void)
{
	int i;
	Trap *p;

	/* todo: should check if signal is fatal, not the TF_DFL_INTR flag */
	for (p = sigtraps, i = NSIG+1; --i >= 0; p++)
		if (p->set && (p->flags & (TF_DFL_INTR|TF_FATAL)))
			/* return value is used as an exit code */
			return (128 + p->signal);
	return (0);
}

/*
 * Returns the signal number of any pending traps: ie, a signal which has
 * occurred for which a trap has been set or for which the TF_DFL_INTR flag
 * is set.
 */
int
trap_pending(void)
{
	int i;
	Trap *p;

	for (p = sigtraps, i = NSIG+1; --i >= 0; p++)
		if (p->set && ((p->trap && p->trap[0]) ||
		    ((p->flags & (TF_DFL_INTR|TF_FATAL)) && !p->trap)))
			return (p->signal);
	return (0);
}

/*
 * run any pending traps. If intr is set, only run traps that
 * can interrupt commands.
 */
void
runtraps(int flag)
{
	int i;
	Trap *p;

	if (ksh_tmout_state == TMOUT_LEAVING) {
		ksh_tmout_state = TMOUT_EXECUTING;
		warningf(false, "timed out waiting for input");
		unwind(LEXIT);
	} else
		/*
		 * XXX: this means the alarm will have no effect if a trap
		 * is caught after the alarm() was started...not good.
		 */
		ksh_tmout_state = TMOUT_EXECUTING;
	if (!flag)
		trap = 0;
	if (flag & TF_DFL_INTR)
		intrsig = 0;
	if (flag & TF_FATAL)
		fatal_trap = 0;
	++trap_nested;
	for (p = sigtraps, i = NSIG+1; --i >= 0; p++)
		if (p->set && (!flag ||
		    ((p->flags & flag) && p->trap == NULL)))
			runtrap(p, false);
	if (!--trap_nested)
		runtrap(NULL, true);
}

void
runtrap(Trap *p, bool is_last)
{
	int old_changed = 0, i;
	char *trapstr;

	if (p == NULL)
		/* just clean up, see runtraps() above */
		goto donetrap;
	i = p->signal;
	trapstr = p->trap;
	p->set = 0;
	if (trapstr == NULL) {
		/* SIG_DFL */
		if (p->flags & TF_FATAL) {
			/* eg, SIGHUP */
			exstat = (int)ksh_min(128U + (unsigned)i, 255U);
			unwind(LLEAVE);
		}
		if (p->flags & TF_DFL_INTR) {
			/* eg, SIGINT, SIGQUIT, SIGTERM, etc. */
			exstat = (int)ksh_min(128U + (unsigned)i, 255U);
			unwind(LINTR);
		}
		goto donetrap;
	}
	if (trapstr[0] == '\0')
		/* SIG_IGN */
		goto donetrap;
	if (i == ksh_SIGEXIT || i == ksh_SIGERR) {
		/* avoid recursion on these */
		old_changed = p->flags & TF_CHANGED;
		p->flags &= ~TF_CHANGED;
		p->trap = NULL;
	}
	if (trap_exstat == -1)
		trap_exstat = exstat & 0xFF;
	/*
	 * Note: trapstr is fully parsed before anything is executed, thus
	 * no problem with afree(p->trap) in settrap() while still in use.
	 */
	command(trapstr, current_lineno);
	if (i == ksh_SIGEXIT || i == ksh_SIGERR) {
		if (p->flags & TF_CHANGED)
			/* don't clear TF_CHANGED */
			afree(trapstr, APERM);
		else
			p->trap = trapstr;
		p->flags |= old_changed;
	}

 donetrap:
	/* we're the last trap of a sequence executed */
	if (is_last && trap_exstat != -1) {
		exstat = trap_exstat;
		trap_exstat = -1;
	}
}

/* clear pending traps and reset user's trap handlers; used after fork(2) */
void
cleartraps(void)
{
	int i;
	Trap *p;

	trap = 0;
	intrsig = 0;
	fatal_trap = 0;
	for (i = NSIG+1, p = sigtraps; --i >= 0; p++) {
		p->set = 0;
		if ((p->flags & TF_USER_SET) && (p->trap && p->trap[0]))
			settrap(p, NULL);
	}
}

/* restore signals just before an exec(2) */
void
restoresigs(void)
{
	int i;
	Trap *p;

	for (i = NSIG+1, p = sigtraps; --i >= 0; p++)
		if (p->flags & (TF_EXEC_IGN|TF_EXEC_DFL))
			setsig(p, (p->flags & TF_EXEC_IGN) ? SIG_IGN : SIG_DFL,
			    SS_RESTORE_CURR|SS_FORCE);
}

void
settrap(Trap *p, const char *s)
{
	sig_t f;

	if (p->trap)
		afree(p->trap, APERM);
	/* handles s == NULL */
	strdupx(p->trap, s, APERM);
	p->flags |= TF_CHANGED;
	f = !s ? SIG_DFL : s[0] ? trapsig : SIG_IGN;

	p->flags |= TF_USER_SET;
	if ((p->flags & (TF_DFL_INTR|TF_FATAL)) && f == SIG_DFL)
		f = trapsig;
	else if (p->flags & TF_SHELL_USES) {
		if (!(p->flags & TF_ORIG_IGN) || Flag(FTALKING)) {
			/* do what user wants at exec time */
			p->flags &= ~(TF_EXEC_IGN|TF_EXEC_DFL);
			if (f == SIG_IGN)
				p->flags |= TF_EXEC_IGN;
			else
				p->flags |= TF_EXEC_DFL;
		}

		/*
		 * assumes handler already set to what shell wants it
		 * (normally trapsig, but could be j_sigchld() or SIG_IGN)
		 */
		return;
	}

	/* todo: should we let user know signal is ignored? how? */
	setsig(p, f, SS_RESTORE_CURR|SS_USER);
}

/*
 * Called by c_print() when writing to a co-process to ensure SIGPIPE won't
 * kill shell (unless user catches it and exits)
 */
int
block_pipe(void)
{
	int restore_dfl = 0;
	Trap *p = &sigtraps[SIGPIPE];

	if (!(p->flags & (TF_ORIG_IGN|TF_ORIG_DFL))) {
		setsig(p, SIG_IGN, SS_RESTORE_CURR);
		if (p->flags & TF_ORIG_DFL)
			restore_dfl = 1;
	} else if (p->cursig == SIG_DFL) {
		setsig(p, SIG_IGN, SS_RESTORE_CURR);
		/* restore to SIG_DFL */
		restore_dfl = 1;
	}
	return (restore_dfl);
}

/* Called by c_print() to undo whatever block_pipe() did */
void
restore_pipe(int restore_dfl)
{
	if (restore_dfl)
		setsig(&sigtraps[SIGPIPE], SIG_DFL, SS_RESTORE_CURR);
}

/*
 * Set action for a signal. Action may not be set if original
 * action was SIG_IGN, depending on the value of flags and FTALKING.
 */
int
setsig(Trap *p, sig_t f, int flags)
{
	struct sigaction sigact;

	if (p->signal == ksh_SIGEXIT || p->signal == ksh_SIGERR)
		return (1);

	memset(&sigact, 0, sizeof(sigact));

	/*
	 * First time setting this signal? If so, get and note the current
	 * setting.
	 */
	if (!(p->flags & (TF_ORIG_IGN|TF_ORIG_DFL))) {
		sigaction(p->signal, &Sigact_ign, &sigact);
		p->flags |= sigact.sa_handler == SIG_IGN ?
		    TF_ORIG_IGN : TF_ORIG_DFL;
		p->cursig = SIG_IGN;
	}

	/*-
	 * Generally, an ignored signal stays ignored, except if
	 *	- the user of an interactive shell wants to change it
	 *	- the shell wants for force a change
	 */
	if ((p->flags & TF_ORIG_IGN) && !(flags & SS_FORCE) &&
	    (!(flags & SS_USER) || !Flag(FTALKING)))
		return (0);

	setexecsig(p, flags & SS_RESTORE_MASK);

	/*
	 * This is here 'cause there should be a way of clearing
	 * shtraps, but don't know if this is a sane way of doing
	 * it. At the moment, all users of shtrap are lifetime
	 * users (SIGALRM, SIGCHLD, SIGWINCH).
	 */
	if (!(flags & SS_USER))
		p->shtrap = (sig_t)NULL;
	if (flags & SS_SHTRAP) {
		p->shtrap = f;
		f = trapsig;
	}

	if (p->cursig != f) {
		p->cursig = f;
		(void)sigemptyset(&sigact.sa_mask);
		/* interruptible */
		sigact.sa_flags = 0;
		sigact.sa_handler = f;
		sigaction(p->signal, &sigact, NULL);
	}

	return (1);
}

/* control what signal is set to before an exec() */
void
setexecsig(Trap *p, int restore)
{
	/* XXX debugging */
	if (!(p->flags & (TF_ORIG_IGN|TF_ORIG_DFL)))
		internal_errorf("setexecsig: unset signal %d(%s)",
		    p->signal, p->name);

	/* restore original value for exec'd kids */
	p->flags &= ~(TF_EXEC_IGN|TF_EXEC_DFL);
	switch (restore & SS_RESTORE_MASK) {
	case SS_RESTORE_CURR:
		/* leave things as they currently are */
		break;
	case SS_RESTORE_ORIG:
		p->flags |= p->flags & TF_ORIG_IGN ? TF_EXEC_IGN : TF_EXEC_DFL;
		break;
	case SS_RESTORE_DFL:
		p->flags |= TF_EXEC_DFL;
		break;
	case SS_RESTORE_IGN:
		p->flags |= TF_EXEC_IGN;
		break;
	}
}

#if HAVE_PERSISTENT_HISTORY || defined(DF)
/*
 * File descriptor locking and unlocking functions.
 * Could use some error handling, but hey, this is only
 * advisory locking anyway, will often not work over NFS,
 * and you are SOL if this fails...
 */

void
mksh_lockfd(int fd)
{
#if defined(__OpenBSD__)
	/* flock is not interrupted by signals */
	(void)flock(fd, LOCK_EX);
#elif HAVE_FLOCK
	int rv;

	/* e.g. on Linux */
	do {
		rv = flock(fd, LOCK_EX);
	} while (rv == 1 && errno == EINTR);
#elif HAVE_LOCK_FCNTL
	int rv;
	struct flock lks;

	memset(&lks, 0, sizeof(lks));
	lks.l_type = F_WRLCK;
	do {
		rv = fcntl(fd, F_SETLKW, &lks);
	} while (rv == 1 && errno == EINTR);
#endif
}

/* designed to not define mksh_unlkfd if none triggered */
#if HAVE_FLOCK
void
mksh_unlkfd(int fd)
{
	(void)flock(fd, LOCK_UN);
}
#elif HAVE_LOCK_FCNTL
void
mksh_unlkfd(int fd)
{
	struct flock lks;

	memset(&lks, 0, sizeof(lks));
	lks.l_type = F_UNLCK;
	(void)fcntl(fd, F_SETLKW, &lks);
}
#endif
#endif
