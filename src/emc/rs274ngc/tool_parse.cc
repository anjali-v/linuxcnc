//    Copyright (C) 2009-2010 Jeff Epler <jepler@unpythonic.net>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "emcglb.h"
#include "emctool.h"
#include "tool_parse.h"
#include <rtapi_string.h>
#include "tooldata.hh"

/********************************************************************
*
* Description: saveToolTable(const char *filename, ...
*		Saves the tool table into file filename.
*		  Array is CANON_TOOL_MAX + 1 entries, since 0 is included.
*
* Return Value: Zero on success or -1 if file not found.
*
* Side Effects: Default setting used if the parameter not found in
*		the ini file.
*
* Called By: ioControl
*
********************************************************************/
int saveToolTable(const char *filename,
#ifdef TOOL_MMAP //{
    // toolTable[] param not used for mmap
#else //}{
	CANON_TOOL_TABLE toolTable[],
#endif //}
	char *ttcomments[CANON_POCKETS_MAX],
	int random_toolchanger)
{
    int idx;
    FILE *fp;
    const char *name;
    int start_idx;

    // check filename
    if (filename[0] == 0) {
	name = tool_table_file;
    } else {
	// point to name provided
	name = filename;
    }

    // open tool table file
    if (NULL == (fp = fopen(name, "w"))) {
	// can't open file
	return -1;
    }

    if(random_toolchanger) {
        start_idx = 0;
    } else {
        start_idx = 1;
    }
    for (idx = start_idx; idx < CANON_POCKETS_MAX; idx++) {
        CANON_TOOL_TABLE tdata;
        tdata = tooldata_get(idx);
        if (tdata.toolno != -1) {
            fprintf(fp, "T%d P%d", tdata.toolno,
                    random_toolchanger? idx : tdata.pocketno);
            if (tdata.diameter) fprintf(fp, " D%f", tdata.diameter);
            if (tdata.offset.tran.x) fprintf(fp, " X%+f", tdata.offset.tran.x);
            if (tdata.offset.tran.y) fprintf(fp, " Y%+f", tdata.offset.tran.y);
            if (tdata.offset.tran.z) fprintf(fp, " Z%+f", tdata.offset.tran.z);
            if (tdata.offset.a)      fprintf(fp, " A%+f", tdata.offset.a);
            if (tdata.offset.b)      fprintf(fp, " B%+f", tdata.offset.b);
            if (tdata.offset.c)      fprintf(fp, " C%+f", tdata.offset.c);
            if (tdata.offset.u)      fprintf(fp, " U%+f", tdata.offset.u);
            if (tdata.offset.v)      fprintf(fp, " V%+f", tdata.offset.v);
            if (tdata.offset.w)      fprintf(fp, " W%+f", tdata.offset.w);
            if (tdata.frontangle)    fprintf(fp, " I%+f", tdata.frontangle);
            if (tdata.backangle)     fprintf(fp, " J%+f", tdata.backangle);
            if (tdata.orientation)   fprintf(fp, " Q%d",  tdata.orientation);
            fprintf(fp, " ;%s\n", ttcomments[idx]);
        }
    }
    fclose(fp);
    return 0;
}

int loadToolTable(const char *filename,
#ifdef TOOL_MMAP //{
             // toolTable[] param not used for mmap
#else //}{
			 CANON_TOOL_TABLE toolTable[],
#endif //}
			 char *ttcomments[],
			 int random_toolchanger)
{
    int  idx;
    FILE *fp;
    char input_line[CANON_TOOL_ENTRY_LEN];
    char orig_line[CANON_TOOL_ENTRY_LEN];

    if(!filename) return -1;

    // open tool table file
    if (NULL == (fp = fopen(filename, "r"))) {
	// can't open file
	return -1;
    }
    // clear out tool table
    // (Set vars to indicate no tool in pocket):
    for (idx = random_toolchanger? 0: 1; idx < CANON_POCKETS_MAX; idx++) {
        CANON_TOOL_TABLE tdata = tooldata_entry_init();
        tooldata_put(tdata,idx);
        if(ttcomments) ttcomments[idx][0] = '\0';
    }

    // after initializing all available pockets:
    tooldata_last_index_set(0);
    // subsequent read from filename will update the last_index
    // which becomes available by tooldata_last_index_get()

    tooldata_parse_init(random_toolchanger);

    while (!feof(fp)) {
        // for nonrandom machines, just read the tools into pockets 1..n
        // no matter their tool numbers.  NB leave the spindle pocket 0
        // unchanged/empty.
        if (NULL == fgets(input_line, CANON_TOOL_ENTRY_LEN, fp)) {
            break;
        }
        CANON_TOOL_TABLE tdata;
        rtapi_strxcpy(orig_line, input_line);

        // parse and store one line from tool table file
        if (tooldata_parse_line(&tdata, //result
                                input_line,
                                ttcomments)) {
            printf("File: %s Unrecognized line skipped:\n    %s",filename, orig_line);
            continue;
        }

        CANON_TOOL_TABLE zdata = tooldata_get(0);
        if (!random_toolchanger && zdata.toolno == tdata.toolno) {
            zdata = tdata;
            tooldata_put(zdata,0);
        }
    } // while

    // close the file
    fclose(fp);

    return 0;
} // loadToolTable()
