/*
 *  Copyright (c) 2000-2009 Vivotek Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VIVOTEK INC.                                                    |
 *  +-----------------------------------------------------------------+
 */

#ifndef __TS2DATETIME_H__
#define __TS2DATETIME_H__

#include "common.h"
#include "typedef.h"
#include "type.h"
#define TS2DATETIME_MODULE_NAME		"ts2datetime"
#define	TS2DATETIME_VERSION			"0.0.1.0"
#define TS2DATETIME_MODIFY_DATETIME	"Last modified at 2015/09/30 12:00:00"
#define TS_2_DATETIME_VERSION MAKEFOURCC( 0, 0, 1, 0)

#define SECAMIN 60
#define SECANHR 3600
#define SECADAY 86400
#define DAYANORMALYR 365
#define FOURYEARCIRCLEDAYS 1461
#define BASE_YEAR 2000
#define FOURHUNDREDCYCLEDAYS 146097
#define ONEHUNDREDCYCLEDAYS 36524
#define LEAPOCH (946684800LL + 86400*(31+29))

#define DATETIME_FORMAT "%04d-%02d-%02dT%02d:%02d:%02dZ"
#define DATETIME_FORMAT_TZ "%04d-%02d-%02dT%02d:%02d:%02d%+03d%02d"

#define DATETIME_MAX_LEN 32

typedef struct _ttimeinfo
{
        int iYear;
        int iMonth;
        int iDay;
        int iHr;
        int iMin;
        int iSec;
        int bLeapYr;
} TTimeInfo;


//static const short int day_per_month[12] = {31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29};
static const short int acc_day_mon[13] = {0, 31, 61, 92, 122, 153, 184, 214, 245, 275, 306, 337, 366};

void ts2datetime(Int64 timestamp, int iTzOffset, int iDst, TTimeInfo * ptCurrTime);

void getDateTime(TTimeInfo *ptCurrTime, int iTzOffset, char *pcBuf, unsigned int iSize);

void VCAReport_GenISO8601DateTimeArr(const UInt64 iStart,const UInt64 iEnd, const Int64 iAggr, const int iTZ, const int iDst, BOOL bLocal, char *** pppcBuf, unsigned int * piSize );

void VCAReport_ReleaseDateTimeArr(char **ppcBuf);

#endif

