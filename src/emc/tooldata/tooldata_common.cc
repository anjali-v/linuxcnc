/*
** Copyright: 2020
** Author:    Dewey Garrett <dgarrett@panix.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <stdio.h>
#include <string.h>
#include <rtapi_string.h>
#include "tooldata.hh"

struct CANON_TOOL_TABLE tooldata_entry_init()
{
    struct CANON_TOOL_TABLE tdata;
    tdata.toolno = -1;
    tdata.pocketno = -1;
    ZERO_EMC_POSE(tdata.offset);
    tdata.diameter = 0.0;
    tdata.frontangle = 0.0;
    tdata.backangle = 0.0;
    tdata.orientation = 0;

    return tdata;
} // tooldata_entry_init()

static bool is_random_toolchanger = 0;
static int  nonrandom_idx = 0; // 'fakepocket' counter
static bool initialized = 0;

void tooldata_parse_init(bool random_toolchanger)
{
    is_random_toolchanger = random_toolchanger ? 1 : 0;
    nonrandom_idx = 0;
    initialized   = 1;
    return;
} // tooldata_parse_init()

int tooldata_parse_line(CANON_TOOL_TABLE *result,
                        char *input_line,
                        char *ttcomments[])
{
    char orig_line[CANON_TOOL_ENTRY_LEN];
    const char *token;
    char *buff, *comment;
    int toolno,orientation,valid=1;
    EmcPose offset; //tlo
    double diameter, frontangle, backangle;
    int idx = 0;
    int realpocket = 0;

    if (!initialized) {
        fprintf(stderr,"!!! PROBLEM no init %s\n",__FILE__);
        return -1;
    }
    strcpy(orig_line, input_line);

    CANON_TOOL_TABLE empty = tooldata_entry_init();
    toolno      = empty.toolno;
    diameter    = empty.diameter;
    frontangle  = empty.frontangle;
    backangle   = empty.backangle;
    orientation = empty.orientation;
    offset      = empty.offset;

    buff = strtok(input_line, ";");
    if (strlen(buff) <=1) {
        //fprintf(stderr,"skip blankline %s\n",__FILE__);
        return 0;
    }
    comment = strtok(NULL, "\n");

    token = strtok(buff, " ");
    while (token != NULL) {
        switch (toupper(token[0])) {
        case 'T':
            if (sscanf(&token[1], "%d", &toolno) != 1)
                valid = 0;
            break;
        case 'P':
            if (sscanf(&token[1], "%d", &idx) != 1) {
                valid = 0;
                break;
            }
            realpocket = idx;  //random toolchanger
            if (!is_random_toolchanger) {
                (nonrandom_idx)++;
                if (nonrandom_idx >= CANON_POCKETS_MAX) {
                    printf("too many tools. skipping tool %d\n", toolno);
                    valid = 0;
                    break;
                }
                idx = nonrandom_idx; // nonrandom toolchanger
            }
            if (idx < 0 || idx >= CANON_POCKETS_MAX) {
                printf("max pocket number is %d. skipping tool %d\n",
                       CANON_POCKETS_MAX - 1, toolno);
                valid = 0;
                break;
            }
            break;
        case 'D':
            if (sscanf(&token[1], "%lf", &diameter) != 1)
                valid = 0;
            break;
        case 'X':
            if (sscanf(&token[1], "%lf", &offset.tran.x) != 1)
                valid = 0;
            break;
        case 'Y':
            if (sscanf(&token[1], "%lf", &offset.tran.y) != 1)
                valid = 0;
            break;
        case 'Z':
            if (sscanf(&token[1], "%lf", &offset.tran.z) != 1)
                valid = 0;
            break;
        case 'A':
            if (sscanf(&token[1], "%lf", &offset.a) != 1)
                valid = 0;
            break;
        case 'B':
            if (sscanf(&token[1], "%lf", &offset.b) != 1)
                valid = 0;
            break;
        case 'C':
            if (sscanf(&token[1], "%lf", &offset.c) != 1)
                valid = 0;
            break;
        case 'U':
            if (sscanf(&token[1], "%lf", &offset.u) != 1)
                valid = 0;
            break;
        case 'V':
            if (sscanf(&token[1], "%lf", &offset.v) != 1)
                valid = 0;
            break;
        case 'W':
            if (sscanf(&token[1], "%lf", &offset.w) != 1)
                valid = 0;
            break;
        case 'I':
            if (sscanf(&token[1], "%lf", &frontangle) != 1)
                valid = 0;
            break;
        case 'J':
            if (sscanf(&token[1], "%lf", &backangle) != 1)
                valid = 0;
            break;
        case 'Q':
            if (sscanf(&token[1], "%d", &orientation) != 1)
                valid = 0;
            break;
        default:
            if (strncmp(token, "\n", 1) != 0)
                valid = 0;
            break;
        }
        token = strtok(NULL, " ");
    }

    if (valid) {
        result->toolno = toolno;
        result->pocketno = realpocket;
        result->offset = offset;
        result->diameter = diameter;
        result->frontangle = frontangle;
        result->backangle = backangle;
        result->orientation = orientation;
        tooldata_put(*result,idx);
        if (ttcomments && comment) {
             strcpy(ttcomments[idx], comment);
        }
    } else {
         return -1;
    }
    return 0;
} // tooldata_parse_line()
