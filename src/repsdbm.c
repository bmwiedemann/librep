/* repsdbm.c -- rep wrapper to libsdbm
   $Id$ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "rep.h"
#include "sdbm.h"
#include <fcntl.h>

static int dbm_type;

#define rep_DBM(v)  ((rep_dbm *) rep_PTR(v))
#define rep_DBMP(v) (rep_CELL16_TYPEP(v, dbm_type) && rep_DBM(v)->dbm != 0)

typedef struct rep_dbm_struct {
    repv car;
    struct rep_dbm_struct *next;
    SDBM *dbm;
    repv path;
    repv access;
    repv mode;
} rep_dbm;

static rep_dbm *dbm_chain;

DEFSYM(sdbm, "sdbm");
DEFSYM(insert, "insert");
DEFSYM(replace, "replace");

DEFUN("sdbm-open", Fsdbm_open, Ssdbm_open, (repv file, repv flags, repv mode),
      rep_Subr3) /*
::doc:Ssdbm-open::
sdbm-open PATH ACCESS-TYPE [MODE]
::end:: */
{
    int uflags, umode;
    rep_dbm *dbm;
    rep_DECLARE1(file, rep_STRINGP);
    rep_DECLARE2(flags, rep_SYMBOLP);
    uflags = (flags == Qwrite ? O_RDWR | O_CREAT | O_TRUNC
	      : (flags == Qappend ? O_RDWR | O_CREAT : O_RDONLY));
    umode = rep_INTP(mode) ? rep_INT(mode) : 0666;
    dbm = rep_ALLOC_CELL (sizeof (rep_dbm));
    if (dbm == 0)
	return rep_mem_error();
    dbm->car = dbm_type;
    dbm->path = file;
    dbm->access = flags;
    dbm->mode = rep_MAKE_INT(umode);
    dbm->dbm = sdbm_open (rep_STR(file), uflags, umode);
    if (dbm->dbm != 0)
    {
	dbm->next = dbm_chain;
	dbm_chain = dbm;
	return rep_VAL(dbm);
    }
    else
    {
	rep_FREE_CELL (dbm);
	return rep_signal_file_error (file);
    }
}

DEFUN("sdbm-close", Fsdbm_close, Ssdbm_close, (repv dbm), rep_Subr1) /*
::doc:Ssdbm-close::
sdbm-close DBM
::end:: */
{
    rep_DECLARE1 (dbm, rep_DBMP);
    sdbm_close (rep_DBM(dbm)->dbm);
    rep_DBM(dbm)->dbm = 0;
    rep_DBM(dbm)->path = Qnil;
    rep_DBM(dbm)->access = Qnil;
    rep_DBM(dbm)->mode = Qnil;
    return Qt;
}

DEFUN("sdbm-fetch", Fsdbm_fetch, Ssdbm_fetch, (repv dbm, repv key), rep_Subr2) /*
::doc:Ssdbm-fetch::
sdbm-fetch DBM KEY
::end:: */
{
    datum dkey, dvalue;
    rep_DECLARE1 (dbm, rep_DBMP);
    rep_DECLARE2 (key, rep_STRINGP);
    dkey.dptr = rep_STR (key);
    dkey.dsize = rep_STRING_LEN (key);
    dvalue = sdbm_fetch (rep_DBM(dbm)->dbm, dkey);
    if (dvalue.dptr == 0)
	return Qnil;
    else
	return rep_string_dupn (dvalue.dptr, dvalue.dsize);
}

DEFUN("sdbm-store", Fsdbm_store, Ssdbm_store, (repv dbm, repv key, repv val, repv flags), rep_Subr4) /*
::doc:Ssdbm-store::
sdbm-store DBM KEY VALUE [FLAGS]
::end:: */
{
    int dflags;
    datum dkey, dvalue;
    rep_DECLARE1 (dbm, rep_DBMP);
    rep_DECLARE2 (key, rep_STRINGP);
    rep_DECLARE3 (val, rep_STRINGP);
    dkey.dptr = rep_STR (key);
    dkey.dsize = rep_STRING_LEN (key);
    dvalue.dptr = rep_STR (val);
    dvalue.dsize = rep_STRING_LEN (val);
    dflags = (flags == Qinsert ? SDBM_INSERT : SDBM_REPLACE);
    return (sdbm_store (rep_DBM(dbm)->dbm, dkey, dvalue, dflags) == 0
	    ? Qt : Qnil);
}

DEFUN("sdbm-delete", Fsdbm_delete, Ssdbm_delete, (repv dbm, repv key), rep_Subr2) /*
::doc:Ssdbm-delete::
sdbm-delete DBM KEY
::end:: */
{
    datum dkey;
    rep_DECLARE1 (dbm, rep_DBMP);
    rep_DECLARE2 (key, rep_STRINGP);
    dkey.dptr = rep_STR (key);
    dkey.dsize = rep_STRING_LEN (key) + 1;
    return sdbm_delete (rep_DBM(dbm)->dbm, dkey) == 0 ? Qt : Qnil;
}

DEFUN("sdbm-firstkey", Fsdbm_firstkey, Ssdbm_firstkey, (repv dbm), rep_Subr1) /*
::doc:Ssdbm-firstkey::
sdbm-firstkey DBM
::end:: */
{
    datum dkey;
    rep_DECLARE1 (dbm, rep_DBMP);
    dkey = sdbm_firstkey (rep_DBM(dbm)->dbm);
    if (dkey.dptr == 0)
	return Qnil;
    else
	return rep_string_dupn (dkey.dptr, dkey.dsize);
}

DEFUN("sdbm-nextkey", Fsdbm_nextkey, Ssdbm_nextkey, (repv dbm), rep_Subr1) /*
::doc:Ssdbm-nextkey::
sdbm-nextkey DBM
::end:: */
{
    datum dkey;
    rep_DECLARE1 (dbm, rep_DBMP);
    dkey = sdbm_nextkey (rep_DBM(dbm)->dbm);
    if (dkey.dptr == 0)
	return Qnil;
    else
	return rep_string_dupn (dkey.dptr, dkey.dsize);
}

DEFUN("sdbm-rdonly", Fsdbm_rdonly, Ssdbm_rdonly, (repv dbm), rep_Subr1) /*
::doc:Ssdbm-rdonly::
sdbm-rdonly DBM
::end:: */
{
    rep_DECLARE1 (dbm, rep_DBMP);
    return sdbm_rdonly (rep_DBM(dbm)->dbm) ? Qt : Qnil;
}

DEFUN("sdbm-error", Fsdbm_error, Ssdbm_error, (repv dbm), rep_Subr1) /*
::doc:Ssdbm-error::
sdbm-error DBM
::end:: */
{
    rep_DECLARE1 (dbm, rep_DBMP);
    return sdbm_error (rep_DBM(dbm)->dbm) ? Qt : Qnil;
}

DEFUN("sdbmp", Fsdbmp, Ssdbmp, (repv arg), rep_Subr1) /*
::doc:Ssdbmp::
sdbmp ARG

Returns t if ARG is an sdbm object (created by `sdbm-open').
::end:: */
{
    return rep_DBMP(arg) ? Qt : Qnil;
}



static void
dbm_mark (repv val)
{
    rep_MARKVAL (rep_DBM(val)->path);
    rep_MARKVAL (rep_DBM(val)->access);
    rep_MARKVAL (rep_DBM(val)->mode);
}

static void
dbm_sweep (void)
{
    rep_dbm *x = dbm_chain;
    dbm_chain = 0;
    while (x != 0)
    {
	rep_dbm *next = x->next;
	if (!rep_GC_CELL_MARKEDP (rep_VAL(x)))
	{
	    if (x->dbm != 0)
		sdbm_close (x->dbm);
	    rep_FREE_CELL (x);
	}
	else
	{
	    rep_GC_CLR_CELL (rep_VAL(x));
	    x->next = dbm_chain;
	    dbm_chain = x;
	}
	x = next;
    }
}

static void
dbm_print (repv stream, repv dbm)
{
    rep_stream_puts (stream, "#<dbm ", -1, rep_FALSE);
    if (rep_STRINGP(rep_DBM(dbm)->path))
	rep_stream_puts (stream, rep_PTR(rep_DBM(dbm)->path), -1, rep_TRUE);
    else
	rep_stream_puts (stream, "nil", -1, rep_FALSE);
    rep_stream_putc (stream, '>');
}

static int
dbm_compare (repv v1, repv v2)
{
    return (v1 == v2) ? 0 : 1;
}

rep_xsubr *rep_dl_subrs[] = { &Ssdbm_open, &Ssdbm_close, &Ssdbm_fetch,
			      &Ssdbm_store, &Ssdbm_delete, &Ssdbm_firstkey,
			      &Ssdbm_nextkey, &Ssdbm_rdonly, &Ssdbm_error,
			      &Ssdbmp, 0 };

void
rep_dl_init (void)
{
    dbm_type = rep_register_new_type ("sdbm", dbm_compare,
				      dbm_print, dbm_print,
				      dbm_sweep, dbm_mark,
				      0, 0, 0, 0, 0, 0, 0);
    rep_INTERN (sdbm);
    rep_INTERN (insert);
    rep_INTERN (replace);
    Fprovide (Qsdbm);
}