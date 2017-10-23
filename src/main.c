#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vcareport.h"
#include <limits.h>

// ===Version Check=======================================
//const char rcsid[] = "$Id: " TS2DATETIME_VERSION " " TS2DATETIME_MODULE_NAME " "TS2DATETIME_MODIFY_DATETIME" $";
// =======================================================

void usage(void)
{
	printf("\n");
    printf("genReport app version: %d.%d.%d.%d\n"
           "Usage:\n"
           "        genReport -s start_time -e end_time -a aggreagation -f format -t vcatype [-i lite] [-l localtime] [-d eventdb] [-b backup] [-h]\n"
           "Options:\n""  -s Start time of query (Time stamp or date time)\n  -e End time of query (Time stamp or date time)\n  -a Aggregation period for each data in second\n  -f Report format such as xml, csv, json\n  -t VCA type: counting/heatmap\n  -i Set 1 to ignore the zero data\n  -l Set 1 to specify input time is local time otherwise it is UTC time\n  -d Get counting report with event time\n  -b Dump query result in specific path\n  -h Show usage\n  Ex1: genReport -f csv -s 2017-04-25T00:00:00 -e 2017-04-27T00:00:00 -a 86400 -t counting -l 1\n  Ex2: genReport -f json -s 1498032900 -e 1498033800 -a 900 -t heatmap\n",
           ((unsigned int)VCA_REPORT_VERSION      ) % 256,
           ((unsigned int)VCA_REPORT_VERSION >> 8 ) % 256,
           ((unsigned int)VCA_REPORT_VERSION >> 16) % 256,
           ((unsigned int)VCA_REPORT_VERSION >> 24)
           );
}

SCODE VCAReport_ParseRequest(int argc, char *argv[], TReportConf * ptConf)
{
	int iCh;
	int iLen = 0;
	FILE *pCmd;
	unsigned int uiResult;

    while ((iCh = getopt(argc, argv, "f:s:e:a:i:l:d:b:t:h")) != -1)
    {
        switch (iCh) {
		case 'f': // format
			if (!strcmp(optarg, "xml"))
			{
				ptConf->eFmt = eXML;
				break;
			}
			else if (!strcmp(optarg, "json"))
			{
				ptConf->eFmt = eJSON;
				break;
			}
			else if (!strcmp(optarg, "csv"))
			{
				ptConf->eFmt = eCSV;
				break;
			}
			else
			{
				printf("Invalid format, please indicate a valid format: xml, csv, json\n");
				usage();
				return S_FAIL;
			}
		case 's':  // Start time, convert input to timestamp format
			parsetime(optarg, &uiResult);
			ptConf->iStartTime = uiResult;
			break;
		case 'e':  // End time, convert input to timestamp format
			parsetime(optarg, &uiResult);
			ptConf->iEndTime = uiResult;
			break;
		case 'a':
			ptConf->iAggregation = (Int64) atoll(optarg);
			break;
		case 'i':
			ptConf->bLite = (atoi(optarg)>0);
			break;
		case 'l':
			ptConf->bLocal = (atoi(optarg)>0);
			break;
		case 'd':
			ptConf->bEventDB = (atoi(optarg)>0);
			break;
		case 'b':
			strncpy(ptConf->acSample, optarg, sizeof(ptConf->acSample));
			break;
		case 't':
			if (strcmp(optarg, "counting") && strcmp(optarg, "heatmap"))
			{
				printf("Invalid VCA type, please indicate counting or heatmap\n");
				usage();
				return S_FAIL;
			}
			strncpy(ptConf->acVCAType, optarg, sizeof(ptConf->acVCAType));
			break;
        case 'h':
            usage();
			return S_FAIL;
        default:
			printf("Invalid flag: %c\n", iCh);
            usage();
			return S_FAIL;
        }
    }

	if (!ptConf->eFmt)
	{
		printf("Please indicate a valid format: xml, csv, json\n");
		usage();
		return S_FAIL;
	}
	if (!strlen(ptConf->acVCAType))
	{
		printf("Please indicate a valid vca type: counting or heatmap\n");
		usage();
		return S_FAIL;
	}
	if (ptConf->iStartTime >= ptConf->iEndTime || ptConf->iAggregation <= 0 || ptConf->iStartTime <= 0)
	{
		printf("Invalid time interval\n");
		usage();
		return S_FAIL;
	}
	return S_OK;
}

int main(int argc, char *argv[])
{
	TReportConf tConf;
	memset(&tConf, 0, sizeof(TReportConf));
	char **ppcDateTime = NULL;

	if (VCAReport_ParseRequest(argc, argv, &tConf) != S_OK)
	{
		printf("Parse request error\n");
		return S_FAIL;
	}

	VCAReport_PrintReportStartTag(&tConf);

	if (VCAReport_GetSystemInfo(&tConf) != S_OK)
	{
		printf("Get system infomation error\n");
		return S_FAIL;
	}

	if (!strcmp(tConf.acVCAType, "counting"))  //Counting and Zone
	{
		if (!tConf.bEventDB)
		{
			VCAReport_GenISO8601Array(&tConf, &ppcDateTime);
		}
		if (VCAReport_GetCountingData(&tConf, ppcDateTime) != S_OK)
		{
			printf("Get report data error\n");
			return S_FAIL;
		}
		if (VCAReport_GetZoneData(&tConf, ppcDateTime) != S_OK)
		{
			printf("Get report data error\n");
			return S_FAIL;
		}
		if (!tConf.bEventDB)
		{
			VCAReport_ReleaseDateTimeArr(ppcDateTime);
		}
	}
	else //tConf.acVCAType: heatmap. Query heatmap DB
	{
		if (VCAReport_GetHeatmapData(&tConf) != S_OK)
		{
			printf("Get report data error\n");
			return S_FAIL;
		}
	}

	VCAReport_PrintReportEndTag(&tConf);

	return S_OK;
}
