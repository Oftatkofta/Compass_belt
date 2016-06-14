/* This is free software. See COPYING for details. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "diag.h"
#include "debug.h"
#include "fgetrec.h"

/***
fgets_nonul() -- like fgets(), but don't stop when a nul byte is read

This function behaves identically to fgets()(3), execpt that it does not
stop when it reads (and copies) a nul byte. Only newline, EOF, error or
hitting the size limit causes reading to stop.
***/
char *fgets_nonul(char * const s, const int size, FILE * const stream) {
   int c, pos;

   if(!s || size < 0 || !stream) return NULL;
   pos = 0;
   while(pos < size-1 && (c = getc(stream)) != EOF) {
     if(c == '\0') c = ' '; /* fold nuls to spaces */
     s[pos++] = c;
     if(c == '\n') break;
   }
   if(!pos) return NULL;
   s[pos] = '\0';
   return s;
}

/***
fgetrec() -- get a single record from a file

Reads the next record from a file of records terminated by newlines. This
function is superficially similar to fgets(), but differs in several
details, including:
   1) fgets() stops on reading nuls; this doesn't
   2) fgets() stops reading when the line length exceeds the buffer
      size; this discards characters up to EOF or the next newline
      and emits a warning to stderr
   3) any problem in fgetrec() yields a diagnostic message via diag()
      with input line number and filename

Arguments:
   name -- file name as a nul-terminated string (used in diagnostics)
   in -- input source
   buf -- pointer to buffer in which data read is to be stored
   len -- number of bytes buffer is able to hold
   line -- pointer to integer containing line number of the last line
          read prior to this call to fgetrec(); will be incremented

Returns:
   >0 -- length of record successfully read including newline
   0 -- skipped bad (e.g., too-long) record - try again; note that
        buffer contents are unspecified
   -1 -- EOF reached; not failure but no more records available
   <-1 -- error
***/
int fgetrec(const char * const name, FILE * const in, char * const buf,
 const size_t len, int * const line) {
   TSTA(!name); TSTA(!in); TSTA(!buf); TSTA(len < 1);
   TSTA(!line); TSTA(*line < 0);
   int i;

   if(fgets_nonul(buf, len, in) == NULL) goto nothing_read;
   (*line)++;

   /* make sure we got something sensible */
   if((i = strlen(buf)) < 1) /* "never happens" */
      return 2*sysdiag("fgets_nonul", "%s:%d: 0-byte read?", name, *line);

   if(buf[i-1] != '\n') { /* deal with too-long/incomplete lines */
      while((i = getc(in)) != EOF && i != '\n');
      if(i == EOF) {
         diag("%s:%d incomplete last line", name, *line);
         goto nothing_read;
      }
      diag("%s:%d line too long - skipped", name, *line);
      return 0;
   }

   return i; /* success */

   nothing_read:
   if(ferror(in)) return 2*sysdiag("fgets_nonul", "%s:%d read error",
    name, *line);
   if(!feof(in)) /* "never happens" */
      return 2*sysdiag("fgets_nonul", "%s:%d read problem (not EOF nor err)",
       name, *line);
   return -1; /* EOF */
}

/*** for_each_rec() -- read all records in file, caller handler by type

This function reads a file containing newline-delimited records. For
each such record, it attempts to find a matching record type in the
rec_handler_t array given by the caller.

When a match is found, an optional handler function or functions (pointed to
by fields in the matching rec_handler_t) may be called.

Arguments:
   fname -- pointer to nul-terminated string containing filename
   recsize -- maximum size in bytes of any valid record (including both
      the trailing newline and the terminating nul); records over this
      size will be discarded and a warning diagnostic emitted
   handler_lst -- pointer to base of array of rec_handler_t elements,
      terminated by an element with len_min set to zero
   userdata -- pointer to any desired data, passed unchanged to handler
      functions; use NULL if you don't need this

Each element within the handler_lst array should have values as follows:
   len_min -- minimum record length in bytes, counting the trailing newline
      but not the terminating nul; this must be at least 1 (for the record
      type which is a newline by itself). A zero in this field indicates
      the end of the handler array.
   len_max -- maximum record length, or zero if no upper bound should be
      used in determining record type
   sig_offset -- offset in bytes from start of record to start of signature;
      ignored if sig is NULL
   sig_len -- length in bytes of signature; ignored if sig is NULL
   sig -- unterminated string containing record signature, or NULL if
      length alone is to be used to determine type
   handler -- pointer to function to be called when a record of this type
      is read, or NULL
   handler2 -- pointer to an alternate handler function (which gets passed
      some additional arguments), or NULL

The handler function (if non-NULL) will be called with the following arguments:
   rec -- pointer to start of record data
   len -- length in bytes of record, counting the trailing newline
      but not the terminating nul
   line -- line number (starting from 1) of the line in the file from
      which this record was read; possibly useful for diagnostics
   fname -- pointer to nul-terminated string containing filename from
      which this record was read; also for diagnostics
   userdata -- pointer passed in to for_each_rec() by caller; may
      point to any desired data or be NULL

The handler2 function (in non-NULL) will be called with the same arguments
as described for handler, above, plus a couple additional arguments:
   in -- pointer to FILE currently being read; assume this is open read-only,
      and exercise caution when seeking (as it will make the line numbering
      wrong, and seeking backward risks endless loops)
   offset -- offset in bytes from the beginning of the file to the start of
      the record just read

If both handler and handler2 are NULL, then the matching record will be
silently discarded.

Handler function returns are treated as follows:
   0 -- continue reading
   non-0 -- stop reading, return same value to caller

Hints:
   Order matters. The first matching rec_handler_t in the array is the
   one that'll be used. Put more specific definitions first.

   If you have old and new formats of a given record, with the same
   signature but different lengths, it is probably a good idea to
   specify two entries with those exact lengths (rather than one entry
   with a min and max spanning both).

   If you want unrecognized records to be silently ignored (or want
   to roll your own error handler), put a catch-all entry at the end
   of the list: len_min=1, len_max=0, sig=NULL .

   If you want to just read until you find a record with some specific
   property, then stop: Have your handler return zero for no match, >0
   for a match and <-1 for error. This will let you distinguish: found,
   not found, handler error, for_each_rec() error.

Returns:
   0 -- success; all records read and no non-zero handler returns for
        any recognized record type
   -1 -- failure, or -1 returned by handler function
   other -- handler function returned this same value
*/
int for_each_rec(const char * const fname, const int recsize,
 const rec_handler_t * const handler_lst, void * const userdata) {
   char *buf = NULL;
   FILE *in = NULL;
   int n, line = 0, rtn = 0;
   long offset;
   const rec_handler_t *p;

   TSTA(!fname); TSTA(recsize < 2); TSTA(!handler_lst);

   if((buf = malloc(recsize)) == NULL)
      return sysdiag("malloc", "can't allocate %d-byte buffer", recsize);

   if((in = fopen(fname, "r")) == NULL) {
      rtn = sysdiag("fopen", "can't open %s", fname); goto done;
   }

   while(1) { /* for each line in the input file... */
      /* get the byte offset to the start of this line: */
      if((offset = ftell(in)) < 0L) {
         rtn = sysdiag("ftell", "can't get position in %s", fname); goto done;
      }

      /* read the contents of the line: */
      if((n = fgetrec(fname, in, buf, recsize, &line)) < 0) break;
      if(n == 0) continue; /* try again after soft error */

      /* try to find a matching record type */
      for(p = handler_lst; p->len_min > 0; p++) {
//diag("handler type %s (%d,%d) compare to \"%3.3s\" (%d)", p->sig, p->len_min, p->len_max, buf, n);
         if(n < p->len_min) continue;
         if(p->len_max && n > p->len_max) continue;
         if(p->sig && p->sig_offset >= 0 && p->sig_len != 0) {
            if(p->sig_offset + p->sig_len > n) continue;
            if(memcmp(buf+p->sig_offset, p->sig, p->sig_len)) continue;
         }
         break;
      }

      /* if this record doesn't match any known type, complain */
      if(p->len_min <= 0) {
         diag("%s:%d unknown record type (%d byte%s) -- skipped", fname, line,
          n, n==1?"":"s");
         continue;
      }

      /* call handler function(s) */
      if(p->handler && (rtn = p->handler(buf, n, line, fname, userdata)) != 0)
         goto done;
      if(p->handler2 && (rtn = p->handler2(buf, n, line, fname, userdata, in,
       offset)) != 0) goto done;
   }
   if(n < -1) { rtn = diag("failure reading %s", fname); goto done; }

   done:
   if(buf) free(buf);
   if(in && fclose(in))
      rtn = sysdiag("fclose", "can't close %s", fname);
   return rtn;
}

/*** for_each_rec_fixed() -- read, handle records of a single fixed type
A simplified wrapper around for_each_rec(), for the (common) case where
there is only one expected record type and it is always of a specific
fixed length.

Arguments:
   fname -- pointer to nul-terminated string containing filename
   recsize -- maximum size in bytes of any valid record (including both
      the trailing newline and the terminating nul); records over this
      size will be discarded and a warning diagnostic emitted
   handler -- pointer to handler function; see for_each_rec() for details
   userdata -- pointer to any desired data, passed unchanged to handler
      functions; use NULL if you don't need this

Returns as per for_each_rec().
***/
int for_each_rec_fixed(const char * const fname, const int recsize,
  int(*handler)(const void * const rec, const int len, const int line,
    const char * const fname, void * const userdata), void * const userdata) {
   rec_handler_t lst[2];

   memset(lst, 0, sizeof(lst));

   lst[0].len_min = lst[0].len_max = recsize-1;
   lst[0].handler = handler;

   return for_each_rec(fname, recsize, lst, userdata);
}
