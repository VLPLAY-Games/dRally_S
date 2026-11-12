#ifndef DATACHECK_H
#define DATACHECK_H

#include <stddef.h>

/* Looks for the Death Rally game files in `dir` the way the game itself checks them (the sizes of
   the original files), but all at once. Fills `report` with one line per problem and returns how
   many there are (0 when everything needed is there).

   The animations in CINEM/ (or the folder CDROM.INI names) are optional; if they are present
   they must be the original files. */
int DataCheck_scan(const char * dir, char * report, size_t report_size);

#endif // DATACHECK_H
