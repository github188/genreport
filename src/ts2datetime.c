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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"ts2datetime.h"

void getDateTime(TTimeInfo *ptCurrTime, int iTzOffset, char * pcBuf, unsigned int iSize)
{
	int iLen = 0;
	if (iTzOffset == 0)
	{
		iLen = snprintf(pcBuf, iSize, DATETIME_FORMAT, ptCurrTime->iYear, ptCurrTime->iMonth, ptCurrTime->iDay, ptCurrTime->iHr, ptCurrTime->iMin, ptCurrTime->iSec);
	} else {
		int ihh = iTzOffset/3600;
		int imm = (iTzOffset%3600)/60;
		if (imm < 0)
		{
			imm = -imm;
		}

		iLen = snprintf(pcBuf, iSize, DATETIME_FORMAT_TZ, ptCurrTime->iYear, ptCurrTime->iMonth, ptCurrTime->iDay, ptCurrTime->iHr, ptCurrTime->iMin, ptCurrTime->iSec, ihh, imm);
	}

	if (iLen < iSize)
	{
		pcBuf[iLen] = '\0';
	}
}

void VCAReport_GenISO8601DateTimeArr(const UInt64 iStart,const UInt64 iEnd, const Int64 iAggr, int iTZ, int iDst, BOOL bLocal, char *** pppcBuf, unsigned int * piSize )
{
	UInt64 iTime = 1;
	int iAllocatedSize, index, SIZE_CHAR = sizeof(char);
	char ** ppcBuf ;
	TTimeInfo tTimeInfo;

	*piSize = ((iEnd - iStart)/iAggr) + 2;
	ppcBuf = (char **) malloc((*piSize) * sizeof(char *));

	iAllocatedSize = DATETIME_MAX_LEN * SIZE_CHAR;
	ppcBuf[0] = (char *) malloc(iAllocatedSize * (*piSize));
	memset(ppcBuf[0], 0, DATETIME_MAX_LEN * (*piSize));

	if (!bLocal)
	{
		iTZ = 0;
		iDst = 0;
	}

	for (;iTime < (*piSize) ; ++iTime)
	{
		ppcBuf[iTime] = ppcBuf[iTime-1] + iAllocatedSize;
	}

	for (iTime = iStart, index = 0 ; index < (*piSize); iTime+=iAggr, ++index)
	{
		ts2datetime(iTime, iTZ, iDst, &tTimeInfo);
		getDateTime(&tTimeInfo, iTZ, ppcBuf[index], DATETIME_MAX_LEN);
	}

	*pppcBuf = ppcBuf;
}

void VCAReport_ReleaseDateTimeArr(char **ppcBuf)
{
	free(ppcBuf[0]);
	free(ppcBuf);
}

void ts2datetime(Int64 timestamp, int iTzOffset, int iDst, TTimeInfo * ptCurrTime)
{
	long int days, rem;
	//int hit = 0;
	int qc_cycle, c_cycle, q_cycle ;

	timestamp += (iTzOffset + iDst * 3600);

	timestamp -= LEAPOCH; // start from 2000/3/1 00:00:00
	days = timestamp/SECADAY;
	rem = timestamp%SECADAY;

	if (rem < 0)
	{
		rem += SECADAY;
		--days;
	}

	ptCurrTime->iHr = rem/SECANHR;
	rem %= SECANHR;
	ptCurrTime->iMin = rem/SECAMIN;
	ptCurrTime->iSec = rem%SECAMIN; //+ hit; // for leap second 23:59:60

	/* --- Date ---*/

	qc_cycle = days/FOURHUNDREDCYCLEDAYS;
	days %= FOURHUNDREDCYCLEDAYS;

	if (days < 0)
	{
		days += FOURHUNDREDCYCLEDAYS;
		--qc_cycle;
	}

	c_cycle = days/ONEHUNDREDCYCLEDAYS;
	if (c_cycle == 4)
	{
		--c_cycle;
	}

	days -= c_cycle*ONEHUNDREDCYCLEDAYS;

	q_cycle = days/FOURYEARCIRCLEDAYS;
	if (q_cycle == 25)
	{
		--q_cycle;
	}

	days -= q_cycle*FOURYEARCIRCLEDAYS;

	rem = days/DAYANORMALYR;
	if (rem == 4)
	{
		--rem;
	}

	days -= rem*DAYANORMALYR;

	ptCurrTime->iYear = BASE_YEAR + rem + 4*q_cycle + 100*c_cycle + 400*qc_cycle;

	for (ptCurrTime->iMonth = (int)(days/31); days + 1 > acc_day_mon[ptCurrTime->iMonth]; ++(ptCurrTime->iMonth));

	ptCurrTime->iDay = days - acc_day_mon[ptCurrTime->iMonth-1] + 1;
	if ((ptCurrTime->iMonth += 2) > 12)
	{
		ptCurrTime->iMonth -= 12;
		++(ptCurrTime->iYear);
	}

}

