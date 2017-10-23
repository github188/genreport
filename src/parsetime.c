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
#include    <time.h>

void parsetime(char *szInput, unsigned int *puiresult)
{
    struct tm result;
	unsigned int ts ;

	if ( (strptime(szInput, "%Y-%m-%dT%T",&result) == NULL)
		 && (strptime(szInput, "%Y-%m-%dT%TZ", &result) == NULL)
		 //&& (strptime(argv[1], "%Y-%m-%dT%T%z", &result) == NULL)
		 && (strptime(szInput, "%Y%m%dT%H%M%S", &result) == NULL)
		 && (strptime(szInput, "%Y%m%dT%H%M%SZ", &result) == NULL)
		 //&& (strptime(argv[1], "%Y%m%dT%H%M%S%z", &result) == NULL)
 	   )
	{
		if ((ts = atoi(szInput)) > 946684800)
		{
			//printf("%u",ts);
			*puiresult = ts;
		}
		else
		{
			//printf("\nstrptime failed %s\n", argv[1]);
			printf("-1");
		}
	}
    else
    {
		//printf("%04d-%02d-%02dT%02d:%02d:%02d UTC time\n",result.tm_year+1900, result.tm_mon+1, result.tm_mday, result.tm_hour, result.tm_min, result.tm_sec);
		//printf("%u", (unsigned int)mktime(&result));
		*puiresult = (unsigned int)mktime(&result);
    }

}
