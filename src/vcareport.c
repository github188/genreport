#include	<stdio.h>
#include	<stdlib.h>
#include    <string.h>
#include	<sqlite3.h>
#include 	<sys/stat.h>
#include 	<sys/socket.h>
#include    <sys/un.h>
#include 	<errno.h>
#include	<time.h>
#include    <unistd.h>
#include    "vcareport.h"
#include	"ts2datetime.h"

int VCAReport_GetFromConfiger(char* pszParam, char *pszOutBuffer)
{
	struct	sockaddr_un	sunx;
	int iFD;
	char szCmd[8192];
	char szBuf[1024];

	memset( szCmd, 0, sizeof( szCmd ) );
	snprintf(szCmd, sizeof( szCmd ) - 1, "get NameValue 99 0\n%s", pszParam);
	//printf( "Cmd=%s\n", szCmd );
	/* Create the unix socket */
	iFD = socket( AF_UNIX, SOCK_STREAM, 0 );
	if( iFD < 0 )
	{
		printf( "cannot create socket (%d).%s\n", errno, strerror( errno ) );
		return S_FAIL;
	}
	memset( &sunx, 0, sizeof( sunx ) );
	sunx.sun_family = AF_UNIX;
	(void) strncpy( sunx.sun_path, "/tmp/configer", sizeof( sunx.sun_path ) );


	if( connect( iFD, (struct sockaddr *)&sunx,	sizeof( sunx.sun_family ) + strlen( sunx.sun_path ) ) < 0)
	{
		printf( "connect failed!" );
		close( iFD );
		return S_FAIL;
	}
	write( iFD, szCmd, strlen( szCmd ) );

	/* Set socket recv timeout */
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int iRet;

	if (setsockopt(iFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0)
	{
		printf("Socket option SO_RCVTIMEO not support\n");
		close(iFD);
		return S_FAIL;
	}

	memset( szBuf, 0, sizeof( szBuf ) );
	while( (iRet = read( iFD, szBuf, sizeof( szBuf ) - 1 )) > 0 )
	{
		;//printf( "%s", szBuf );
	}
	char * pch = strtok (szBuf,"=");
	if (pch == NULL)
		return S_FAIL;
	pch = strtok(NULL, "=");
	memcpy(pszOutBuffer, pch, strlen(pch));
	pszOutBuffer[strlen(pch)-1] = '\0';

	if (iRet < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN )
		{
			printf("Read timeout (%d). %s\n", errno, strerror(errno));
		}
		else
		{
			printf("Read failed (%d). %s\n", errno, strerror(errno));
		}
		close(iFD);
		return S_FAIL;
	}
	close( iFD );

	return S_OK;
}

UInt64 VCAReport_GetUTCTime(UInt64 iTimestamp, int iTimeZone, int iDST)
{
	return iTimestamp - iTimeZone - iDST * 3600;
}

void VCAReport_GetISO8601Time(UInt64 iTime, int iTzOffset, int iDst, BOOL bLocal, char *pcTime)
{
	if (!bLocal)
		iTzOffset = 0;
	TTimeInfo tTimeInfo;
	ts2datetime(iTime, iTzOffset, iDst, &tTimeInfo);
	getDateTime(&tTimeInfo, iTzOffset, pcTime, DATETIME_MAX_LEN);
}

void VCAReport_PrintReportStartTag(TReportConf *ptConf)
{
	if (ptConf->eFmt == eXML)
		printf("%s", EVENT_ROOT_XML_STAG);
	else if (ptConf->eFmt == eJSON)
		printf("%s", EVENT_ROOT_JSON_STAG);
}

void VCAReport_PrintReportEndTag(TReportConf *ptConf)
{
	if (ptConf->eFmt == eXML)
		printf("%s", EVENT_ROOT_XML_ETAG);
	else if (ptConf->eFmt == eJSON)
		printf("%s", EVENT_DATA_JSON_ETAG);
}

SCODE VCAReport_GetSystemInfo(TReportConf *ptConf)
{
	// Get system info
	char acBuf[MAX_INFO_LENGTH] = {0};
	char szMac[MAX_INFO_LENGTH] = {0}, szIP[MAX_INFO_LENGTH] = {0}, szDeviceID[MAX_INFO_LENGTH] = {0},
		 szGroupID[MAX_INFO_LENGTH] = {0}, szModuleName[MAX_INFO_LENGTH] = {0}, szTimeZone[MAX_INFO_LENGTH] = {0},
		 szDST[MAX_INFO_LENGTH] = {0}, szCurr[MAX_INFO_LENGTH] = {0};
	int iLen = 0;

	VCAReport_GetFromConfiger("system_info_serialnumber", szMac);
	int i ,j;
	for (i = 0, j = 0; i < strlen(szMac); ++i, ++j)
	{
		if (i != 0 && i%2 == 0)
		{
			acBuf[j] = ':';
			acBuf[j+1] = szMac[i];
			j = j+1;
		}
		else
		{
			acBuf[j] = szMac[i];
		}
	}
	strncpy(szMac, acBuf, sizeof(szMac));

	VCAReport_GetFromConfiger("network_ipaddress", szIP);
	VCAReport_GetFromConfiger("system_location_deviceid", szDeviceID);
	VCAReport_GetFromConfiger("system_location_groupid", szGroupID);
	VCAReport_GetFromConfiger("system_info_modelname", szModuleName);
	VCAReport_GetFromConfiger("system_timezoneindex", szTimeZone);
	VCAReport_GetFromConfiger("system_daylight_dstactualmode", szDST);

	int iTimeZone = atoi(szTimeZone);
	iTimeZone -= iTimeZone%10;
	ptConf->iTzOffset = ptConf->bLocal ? iTimeZone * 90 : 0;
	iTimeZone /= 40;
	iTimeZone > 0 ? sprintf(szTimeZone, "+%d", iTimeZone) : sprintf(szTimeZone, "%d", iTimeZone);

	!strcmp(szDST, "3") ? strcpy(szDST, "1") : strcpy(szDST, "0");
	ptConf->iDST = atoi(szDST);

	struct timeval tv;
	TTimeInfo tTimeInfo;
	gettimeofday(&tv, NULL);  //Get current time
	UInt64 ullUtcTime = VCAReport_GetUTCTime(tv.tv_sec, ptConf->iTzOffset, ptConf->iDST);
	ts2datetime(ullUtcTime, 0, 0, &tTimeInfo);
	snprintf(szCurr, MAX_INFO_LENGTH, DATETIME_FORMAT, tTimeInfo.iYear, tTimeInfo.iMonth, tTimeInfo.iDay,
			tTimeInfo.iHr, tTimeInfo.iMin, tTimeInfo.iSec);

#ifdef _DEBUGSYSINFO_
	printf("===================================================================================================\n");
	printf("Mac: %s\nIP: %s\nDeviceID: %s\nGroupID: %s\nModule Name: %s\nTime Zone: %s\nDST: %s\nCurrent Time: %s\n",
			szMac, szIP, szDeviceID, szGroupID, szModuleName, szTimeZone, szDST, szCurr);
	printf("===================================================================================================\n");
#endif

	switch (ptConf->eFmt)
	{
		case eXML:
			if ((iLen = snprintf(ptConf->acSystemInfo, MAX_SYSINFO_LENGTH, EVENT_SOURCE_XML, szCurr, szGroupID,
								 szDeviceID, szModuleName, szMac, szIP, szTimeZone, szDST)) >= MAX_SYSINFO_LENGTH)
			{
				printf("The size of system info exceeds buffer\n");
				return S_FAIL;
			}
			break;
		case eJSON:
			if (!strcmp(ptConf->acVCAType, "heatmap"))
			{
				if ((iLen = snprintf(ptConf->acSystemInfo, MAX_SYSINFO_LENGTH, EVENT_SOURCE_HEATMAP_JSON, szCurr,
									 szModuleName, szMac, szIP, szTimeZone, szDST)) >= MAX_SYSINFO_LENGTH)
				{
					printf("The size of system info exceeds buffer\n");
					return S_FAIL;
				}
			}
			else
			{
				if ((iLen = snprintf(ptConf->acSystemInfo, MAX_SYSINFO_LENGTH, EVENT_SOURCE_JSON, szCurr, szGroupID,
									 szDeviceID, szModuleName, szMac, szIP, szTimeZone, szDST)) >= MAX_SYSINFO_LENGTH)
				{
					printf("The size of system info exceeds buffer\n");
					return S_FAIL;
				}
			}
			break;
		case eCSV:
			if ((iLen = snprintf(ptConf->acSystemInfo, MAX_SYSINFO_LENGTH, EVENT_SOURCE_CSV, szCurr, szGroupID,
								 szDeviceID, szModuleName, szMac, szIP, szTimeZone, szDST)) >= MAX_SYSINFO_LENGTH)
			{
				printf("The size of system info exceeds buffer\n");
				return S_FAIL;
			}
			break;
		default:
			break;
	}

	printf("%s", ptConf->acSystemInfo);
	if (ptConf->eFmt == eJSON)
	{
		printf("%s", EVENT_ROOT_JSON_ETAG);
	}

	return S_OK;
}

void VCAReport_GenISO8601Array(TReportConf *ptConf, char ***ppcDateTime)
{
	unsigned int uiDateTimeArrSize;
	VCAReport_GenISO8601DateTimeArr(ptConf->iStartTime, ptConf->iEndTime, ptConf->iAggregation,	ptConf->iTzOffset,
									ptConf->iDST, ptConf->bLocal, ppcDateTime, &uiDateTimeArrSize);

}

void UpdateBufferInfo(char **ppcTmp, int *piDataLen, int *piRemain, int iLen)
{
	*ppcTmp += iLen;
	*piDataLen += iLen;
	*piRemain -= iLen;
}

void PrintAndResetBufInfo(char **ppcBuf, char **ppcTmp, int *piDataLen, int *piRemain)
{
	*(*ppcBuf + *piDataLen) = '\0';
	printf("%s", *ppcBuf);
	memset(*ppcBuf, 0, MAX_BUFFER_SIZE);
	*ppcTmp = *ppcBuf;
	*piDataLen = 0;
	*piRemain = MAX_BUFFER_SIZE;
}

SCODE VCAReport_MakeTempDir()
{
	struct stat st = {0};
	if (stat(DEFAULT_SQLITE3_TEMP_DIR, &st) == -1)
	{
		if ((mkdir(DEFAULT_SQLITE3_TEMP_DIR, 0777)) < 0)
		{
			printf("Make temp error: %s\n", strerror(errno));
			return S_FAIL;
		}
	}
	return S_OK;
}

SCODE VCAReport_GetCountingData(TReportConf *ptConf, char **ppcDateTime)
{
	sqlite3 * db = NULL;
	sqlite3_stmt *stmt = NULL;
	SCODE status = S_OK;
	int rc, iDataLen = 0, iLen = 0;
	char acScript[MAX_SCRIPT_LENGTH] = {0};
	char acDataFormat[MAX_FORMAT_BUFLEN] = {0};

	int iIn = 0, iOut = 0, index = 0;
	Int64 iStartTime = 0;
	FILE *fp = NULL;

	char *pcBuf = calloc(MAX_BUFFER_SIZE, sizeof(char));
	if (pcBuf == NULL)
	{
		printf("Allocate buff failed\n");
		return S_FAIL;
	}
	char *pcTmp = pcBuf;

	if (ptConf->acCountDBPath[0] == '\0')
	{
		iLen = snprintf(ptConf->acCountDBPath, sizeof(ptConf->acCountDBPath), DEFAULT_COUNTING_DB_PATH);
		ptConf->acCountDBPath[iLen] = '\0';
	}

	switch (ptConf->eFmt)
	{
		case eXML:
			iLen = snprintf(acDataFormat, MAX_FORMAT_BUFLEN, "%s", REPORT_XML_FORMAT);
			acDataFormat[iLen] = '\0';
			printf("%s", EVENT_COUNT_HEAD_XML_STAG);
			break;
		case eJSON:
			iLen = snprintf(acDataFormat, MAX_FORMAT_BUFLEN, "%s", REPORT_JSON_FORMAT);
			acDataFormat[iLen] = '\0';
			printf("%s%s", EVENT_DATA_JSON_STAG, EVENT_COUNT_HEAD_JSON_STAG);
			break;
		case eCSV:
			iLen = snprintf(acDataFormat, MAX_FORMAT_BUFLEN, "%s", REPORT_CSV_FORMAT);
			acDataFormat[iLen] = '\0';
			printf("%s", EVENT_COUNT_HEAD_CSV);
			break;
		default:
			printf("ERROR format in counting\n");
			return S_FAIL;
			break;
	}

	rc = sqlite3_open(ptConf->acCountDBPath, &db);
	if ( rc != SQLITE_OK)
	{
		printf("ERROR: cannot open database %s\n",ptConf->acCountDBPath);
		return S_FAIL;
	}

	rc = sqlite3_exec(db,SQL_PRAGMA_TEMP_DIR, 0, 0, 0);
	if (rc != SQLITE_OK)
	{
		printf("ERROR: cannot setup PGRAMA : %s\n", SQL_PRAGMA_TEMP_DIR);
		sqlite3_close(db);
		return S_FAIL;
	}

	if (ptConf->bEventDB)
	{
		iLen = snprintf(acScript, MAX_SCRIPT_LENGTH, EVENTDB_SQL_QUERY_FORMAT, ptConf->iStartTime, ptConf->iEndTime);
	}
	else
	{
		iLen = snprintf(acScript, MAX_SCRIPT_LENGTH, COUNT_SQL_QUERY_FORMAT, ptConf->iStartTime, ptConf->iAggregation,
						ptConf->iAggregation, ptConf->iStartTime, ptConf->iStartTime, ptConf->iEndTime);
	}
	acScript[iLen]='\0';
	//printf("SQLite exec: %s\n", acScript);

	// ================================================
	// Dump counting query

#ifdef _DEBUG_
	char acCountQuery[MAX_FILENAME_BUFLEN] = {0};
	snprintf(acCountQuery, MAX_FILENAME_BUFLEN, COUNT_QUERY, (int)time(NULL));
	if ((fp = fopen(acCountQuery, "w+")) == NULL)
	{
		printf("Open count query dump failed\n");
		return S_FAIL;
	}
	fprintf(fp, "%s\n.separator \" \"\n%s", SQL_PRAGMA_TEMP_DIR, acScript);
	fclose(fp);
#endif
	// ================================================

	rc = sqlite3_prepare_v2(db, acScript, iLen, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("ERROR: cannot binding %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return S_FAIL;
	}

	char acEventDB[MAX_FORMAT_BUFLEN] = {0};
	char acIn[10] = {0}, acOut[10] = {0}, acCountName[40] = {0}, acRuleName[40] = {0},
		 acStart[DATETIME_MAX_LEN] = {0}, acEnd[DATETIME_MAX_LEN] = {0};
	iLen = 0, iDataLen = 0;
	int iRemain = MAX_BUFFER_SIZE;

		while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			iIn = sqlite3_column_int(stmt, 1);
			iOut = sqlite3_column_int(stmt, 2);
			strncpy(acCountName, sqlite3_column_text(stmt, 0), sizeof(acCountName));
			if (ptConf->bLite == 0 || iIn != 0 || iOut != 0)
			{
				iStartTime = sqlite3_column_int64(stmt, 3);
				index = (iStartTime - ptConf->iStartTime)/ptConf->iAggregation;
				if (ptConf->bEventDB)
				{
					VCAReport_GetISO8601Time(ptConf->iEndTime, ptConf->iTzOffset, ptConf->iDST, ptConf->bLocal, acEnd);
					if (strlen(acRuleName))
					{
						if (!strcmp(acRuleName, acCountName))
						{
							VCAReport_GetISO8601Time(iStartTime, ptConf->iTzOffset, ptConf->iDST,
														ptConf->bLocal, acEnd);
						}
						iLen = snprintf(pcTmp, iRemain, acDataFormat, acRuleName, atoi(acIn), atoi(acOut),
										acStart, acEnd);
					}
				}
				else
				{
					iLen = snprintf(pcTmp, iRemain, acDataFormat, acCountName, iIn, iOut,
									ppcDateTime[index] , ppcDateTime[index+1]);
				}

				if (iLen < iRemain) //Buffer length is enough
				{
					UpdateBufferInfo(&pcTmp, &iDataLen, &iRemain, iLen);
				}
				else //Buffer length is not enough, print and clear buffer
				{
					PrintAndResetBufInfo(&pcBuf, &pcTmp, &iDataLen, &iRemain);

					if (ptConf->bEventDB)
					{
						iLen = snprintf(pcTmp, iRemain, acDataFormat, acRuleName, atoi(acIn), atoi(acOut),
										acStart, acEnd);
					}
					else
					{
						iLen = snprintf(pcTmp, iRemain, acDataFormat, acCountName, iIn, iOut, ppcDateTime[index],
										ppcDateTime[index+1]);
					}

					UpdateBufferInfo(&pcTmp, &iDataLen, &iRemain, iLen);
				}
				if (ptConf->bEventDB)
				{
					snprintf(acRuleName, sizeof(acRuleName), "%s", acCountName);
					snprintf(acIn, sizeof(acIn), "%d", iIn);
					snprintf(acOut, sizeof(acOut), "%d", iOut);
					VCAReport_GetISO8601Time(iStartTime, ptConf->iTzOffset, ptConf->iDST, ptConf->bLocal, acStart);
				}
			}
		}
		if (rc != SQLITE_DONE)
		{
			printf("ERROR: in sqlite3_step : %s\n", sqlite3_errmsg(db));
			status = S_FAIL;
		}
		sqlite3_finalize(stmt);
		sqlite3_close(db);

	if (ptConf->bEventDB)
	{
		VCAReport_GetISO8601Time(ptConf->iEndTime, ptConf->iTzOffset, ptConf->iDST, ptConf->bLocal, acEnd);
		iLen = snprintf(pcTmp, iRemain, acDataFormat, acRuleName, atoi(acIn), atoi(acOut), acStart, acEnd);
		if (iLen < iRemain)
		{
			UpdateBufferInfo(&pcTmp, &iDataLen, &iRemain, iLen);
		}
		else
		{
			PrintAndResetBufInfo(&pcBuf, &pcTmp, &iDataLen, &iRemain);
			iLen = snprintf(pcTmp, iRemain, acDataFormat, acRuleName, atoi(acIn), atoi(acOut), acStart, acEnd);
			UpdateBufferInfo(&pcTmp, &iDataLen, &iRemain, iLen);
		}
	}
	if (ptConf->eFmt == eJSON)
	{
		pcBuf[--iDataLen] = '\0';
	}

	printf("%s", pcBuf);

	if (ptConf->eFmt == eXML)
		printf("%s", EVENT_COUNT_HEAD_XML_ETAG);
	if (ptConf->eFmt == eJSON)
		printf("%s", EVENT_COUNT_HEAD_JSON_ETAG);

	free(pcBuf);

	return status;
}

SCODE VCAReport_GetZoneData(TReportConf *ptConf, char **ppcDateTime)
{
	sqlite3 * db = NULL;
	sqlite3_stmt *stmt = NULL;
	SCODE status = S_OK;
	int rc, iDataLen = 0, iLen = 0, iZoneDataLen = 0;
	char acScript[MAX_SCRIPT_LENGTH] = {0};
	char acDataFormat[MAX_FORMAT_BUFLEN] = {0};
	char acZoneDataFormat[MAX_FORMAT_BUFLEN] = {0};

	int index = 0;
	int iInward = 0, iOutward = 0, iTotal = 0;
	float dAvgCnt = 0, dAvgDur = 0;
	char acZoneName[40] = {0};
	Int64 iStartTime = 0;
	FILE *fp = NULL;

	char *pcBuf = calloc(MAX_BUFFER_SIZE, sizeof(char));
	if (pcBuf == NULL)
	{
		printf("Allocate buff failed\n");
		return S_FAIL;
	}
	char *pcTmp = pcBuf;

	if (ptConf->acZoneDBPath[0] == '\0')
	{
		iLen = snprintf(ptConf->acZoneDBPath, sizeof(ptConf->acZoneDBPath), DEFAULT_ZONE_DB_PATH);
		ptConf->acZoneDBPath[iLen] = '\0';
	}

	switch (ptConf->eFmt)
	{
		case eXML:
			iLen = snprintf(acZoneDataFormat, MAX_FORMAT_BUFLEN, "%s", EVENT_ZONE_DATA_XML);
			acZoneDataFormat[iLen] = '\0';
			printf("%s", EVENT_ZONE_HEAD_XML_STAG);
			break;
		case eJSON:
			iLen = snprintf(acZoneDataFormat, MAX_FORMAT_BUFLEN, "%s", EVENT_ZONE_DATA_JSON);
			acZoneDataFormat[iLen] = '\0';
			printf("%s", EVENT_ZONE_HEAD_JSON_STAG);
			break;
		case eCSV:
			iLen = snprintf(acZoneDataFormat, MAX_FORMAT_BUFLEN, "%s", EVENT_ZONE_DATA_CSV);
			acZoneDataFormat[iLen] = '\0';
			printf("%s", EVENT_ZONE_HEAD_CSV);
			break;
		default:
			printf("ERROR format in zone\n");
			return S_FAIL;
			break;
	}

	if(!ptConf->bEventDB)
	{
		rc = sqlite3_open(ptConf->acZoneDBPath, &db);
		if ( rc != SQLITE_OK)
		{
			printf("ERROR: cannot open database %s\n",ptConf->acZoneDBPath);
			return S_FAIL;
		}

		rc = sqlite3_exec(db,SQL_PRAGMA_TEMP_DIR, 0, 0, 0);
		if (rc != SQLITE_OK)
		{
			printf("ERROR: cannot setup PGRAMA : %s\n", SQL_PRAGMA_TEMP_DIR);
			sqlite3_close(db);
			return S_FAIL;
		}

		iLen = snprintf(acScript, MAX_SCRIPT_LENGTH, ZONE_SQL_QUERY_FORMAT, ptConf->iStartTime, ptConf->iAggregation,
						ptConf->iAggregation, ptConf->iStartTime, ptConf->iStartTime, ptConf->iEndTime,
						ptConf->iAggregation, ptConf->iStartTime, ptConf->iEndTime);
		acScript[iLen]='\0';
		//printf("SQLite exec: %s\n", acScript);

		// ================================================
		// Dump zone query

#ifdef _DEBUG_
		char acZoneQuery[MAX_FILENAME_BUFLEN] = {0};
		snprintf(acZoneQuery, MAX_FILENAME_BUFLEN, ZONE_QUERY, (int)time(NULL));
		if ((fp = fopen(acZoneQuery, "w+")) == NULL)
		{
			printf("Open zone query dump failed\n");
			return S_FAIL;
		}
		fprintf(fp, "%s\n.separator \" \"\n%s", SQL_PRAGMA_TEMP_DIR, acScript);
		fclose(fp);
#endif
		// ================================================

		sqlite3_prepare_v2(db, acScript, iLen, &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			printf("ERROR: cannot binding %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return S_FAIL;
		}

		iLen = 0, iDataLen = 0;
		int iRemain = MAX_BUFFER_SIZE;

			while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
			{
				strncpy(acZoneName, sqlite3_column_text(stmt,0), sizeof(acZoneName));
				iInward = sqlite3_column_int(stmt, 1);
				iOutward = sqlite3_column_int(stmt, 2);
				if (ptConf->bLite == 0 || iInward != 0 || iOutward != 0)
				{
					iTotal = sqlite3_column_int(stmt, 3);
					dAvgDur = sqlite3_column_double(stmt, 4);
					dAvgCnt = sqlite3_column_double(stmt, 5);
					iStartTime = sqlite3_column_int64(stmt, 6);
					index = (iStartTime - ptConf->iStartTime)/ptConf->iAggregation;
					iLen = snprintf(pcTmp, iRemain, acZoneDataFormat, acZoneName, iInward, iOutward, iTotal, dAvgDur,
									dAvgCnt, ppcDateTime[index] , ppcDateTime[index+1]);

					if (iLen < iRemain)
					{
						UpdateBufferInfo(&pcTmp, &iDataLen, &iRemain, iLen);
					}
					else
					{
						PrintAndResetBufInfo(&pcBuf, &pcTmp, &iDataLen, &iRemain);

						iLen = snprintf(pcTmp, iRemain, acZoneDataFormat, acZoneName, iInward, iOutward, iTotal,
										dAvgDur, dAvgCnt, ppcDateTime[index] , ppcDateTime[index+1]);

						UpdateBufferInfo(&pcTmp, &iDataLen, &iRemain, iLen);
					}
				}
			}
			if (rc != SQLITE_DONE)
			{
				printf("ERROR: in sqlite3_step : %s\n", sqlite3_errmsg(db));
				status = S_FAIL;
			}

			sqlite3_finalize(stmt);
			sqlite3_close(db);

		if (ptConf->eFmt == eJSON)
		{
			pcBuf[--iDataLen] = '\0';
		}
		printf("%s", pcBuf);

		free(pcBuf);
	}

	if (ptConf->eFmt == eXML)
		printf("%s", EVENT_ZONE_HEAD_XML_ETAG);
	if (ptConf->eFmt == eJSON)
		printf("%s", EVENT_ZONE_HEAD_JSON_ETAG);

	return status;
}

SCODE VCAReport_GetHeatmapData(TReportConf *ptConf)
{
	sqlite3 * db = NULL;
	sqlite3_stmt *stmt = NULL;
	SCODE status = S_OK;
	int rc, iLen = 0, iHeatmapDataLen = 0;
	char acScript[MAX_SCRIPT_LENGTH] = {0};

	FILE *fp = NULL;

	char *pcBuf = calloc(MAX_BUFFER_SIZE, sizeof(char));
	if (pcBuf == NULL)
	{
		printf("Allocate buff failed\n");
		return S_FAIL;
	}

	if (ptConf == NULL || ptConf->iStartTime >= ptConf->iEndTime || ptConf->iStartTime <= 0)
	{
		return S_FAIL;
	}

	if (ptConf->acHeatmapDBPath[0] == '\0')
	{
		iLen = snprintf(ptConf->acHeatmapDBPath, sizeof(ptConf->acHeatmapDBPath), DEFAULT_HEATMAP_DB_PATH);
		ptConf->acHeatmapDBPath[iLen] = '\0';
	}
	printf("%s", EVENT_DATA_JSON_STAG);

	rc = sqlite3_open(ptConf->acHeatmapDBPath, &db);
	if ( rc != SQLITE_OK)
	{
		printf("ERROR: cannot open database %s\n",ptConf->acHeatmapDBPath);
		return S_FAIL;
	}

	rc = sqlite3_exec(db,SQL_PRAGMA_TEMP_DIR, 0, 0, 0);
	if (rc != SQLITE_OK)
	{
		printf("ERROR: cannot setup PGRAMA : %s\n", SQL_PRAGMA_TEMP_DIR);
		sqlite3_close(db);
		return S_FAIL;
	}

	iLen = snprintf(acScript, MAX_SCRIPT_LENGTH, HEATMAP_SQL_FORMAT, ptConf->iStartTime, ptConf->iEndTime);
	acScript[iLen]='\0';
	//printf("SQLite exec: %s\n", acScript);

	// ================================================
	// Dump heatmap query

#ifdef _DEBUG_
	char acHeatmapQuery[MAX_FILENAME_BUFLEN] = {0};
	snprintf(acHeatmapQuery, MAX_FILENAME_BUFLEN, HEATMAP_QUERY, (int)time(NULL));
	if ((fp = fopen(acHeatmapQuery, "w+")) == NULL)
	{
		printf("Open heatmap query dump file failed\n");
		return S_FAIL;
	}
	fprintf(fp, "%s\n.separator \" \"\n%s", SQL_PRAGMA_TEMP_DIR, acScript);
	fclose(fp);
#endif
	// ================================================

	sqlite3_prepare_v2(db, acScript, iLen, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("ERROR: cannot binding %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return S_FAIL;
	}

	iLen = 0;

		FILE *fpHeatmap = NULL;
		char acHeatmapTmp[MAX_FILENAME_BUFLEN] = {0};
		snprintf(acHeatmapTmp, MAX_FILENAME_BUFLEN, HEATMAP_TMP_FILE, (int)ptConf->iStartTime);
		if ((fpHeatmap = fopen(acHeatmapTmp, "w+")) == NULL)
		{
			printf("Open heatmap tmp file for writing error\n");
			return S_FAIL;
		}
		while((rc = sqlite3_step(stmt)) == SQLITE_ROW) //heatmap only executes one time since db contains entire json.
		{
			if ((char*) sqlite3_column_text(stmt,0)) // Check db is null or not
			{
				fprintf(fpHeatmap, "%s", sqlite3_column_text(stmt,0)); // Move result to temp file first.
			}
		}
		fclose(fpHeatmap);

		if (rc != SQLITE_DONE)
		{
			printf("ERROR: in sqlite3_step : %s\n", sqlite3_errmsg(db));
			status = S_FAIL;
		}
		sqlite3_finalize(stmt);
		sqlite3_close(db);

		if ((fpHeatmap = fopen(acHeatmapTmp, "r")) == NULL)
		{
			printf("Open heatmap tmp file for reading error\n");
			return S_FAIL;
		}
		while ((iLen = fread(pcBuf, 1, MAX_BUFFER_SIZE-1, fpHeatmap)) > 0) //Print result from temp file.
		{
			printf("%s", pcBuf);
			memset(pcBuf, 0, MAX_BUFFER_SIZE);
		}
		fclose(fpHeatmap);
		if (remove(acHeatmapTmp) < 0)
		{
			printf("Unable to delete the heatmap tmp file\n");
			return S_FAIL;
		}

	free(pcBuf);
	return status;
}

