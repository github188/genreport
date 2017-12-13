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
#ifndef __VCAREPORT_H__
#define __VCAREPORT_H__

#include "common.h"
#include "typedef.h"
#include "type.h"
#define VCAREPORT_MODULE_NAME		"vcareport"
#define	VCAREPORT_VERSION			"1.1.3.0"
#define VCAREPORT_MODIFY_DATETIME	"Last modified at 2017/12/12 18:00:00"
#define VCA_REPORT_VERSION			MAKEFOURCC(1, 1, 3, 0)

#define MAX_BUFFER_SIZE 			102400
#define MAX_SCRIPT_LENGTH			1024
#define MAX_FORMAT_BUFLEN			256
#define MAX_FILENAME_BUFLEN			256
#define MAX_CMD_LENGTH 				512
#define MAX_INFO_LENGTH 			50
#define MAX_SYSINFO_LENGTH 			512

#define DEFAULT_COUNTING_DB_PATH		"/mnt/flash2/vcadb/counting.db"
#define DEFAULT_ZONE_DB_PATH			"/mnt/flash2/vcadb/zone.db"
#define DEFAULT_HEATMAP_DB_PATH			"/mnt/flash2/vcadb/heatmap.db"
#define DEFAULT_QUERY_FILE_PATH_FORMAT  "/mnt/flash2/reportpush/report_%ld.sql"
#define DEFAULT_SQLITE3_TEMP_DIR    	"/mnt/flash2/reportpush/sqlite_tmp"
#define COUNT_QUERY                     "/mnt/flash2/reportpush/Count_report_%d.sql"
#define ZONE_QUERY                      "/mnt/flash2/reportpush/Zone_report_%d.sql"
#define HEATMAP_QUERY                   "/mnt/flash2/reportpush/Heatmap_report_%d.sql"
#define HEATMAP_TMP_FILE                "/mnt/flash2/reportpush/HeatmapTmp_%d"

#define TYPE_NAME_COUNTING			"Counting"
#define TYPE_NAME_ZONE				"ZoneDetection"

#define COUNT_TABLE					"count_event_tb"
#define ZONE_TABLE                  "zone_event_tb"
#define HEATMAP_TABLE               "heatmap_event_tb"
#define COL_RULE					"rule_name"
#define COL_IN						"left"
#define COL_OUT						"right"
#define COL_TIME					"event_time"
#define COL_STARTTIME				"StartTime"
#define COL_AVGDURATION 			"AvgDuration"
#define COL_TOTALCOUNT 				"TotalCount"
#define COL_AVGCOUNT 				"AvgCount"

#define SQL_PRAGMA_TEMP_DIR		    "PRAGMA temp_store_directory=\'"DEFAULT_SQLITE3_TEMP_DIR"\';"

#define COUNT_SQL_QUERY_FORMAT		"select "COL_RULE",sum("COL_IN"),sum("COL_OUT"), ((("COL_TIME" - %llu ) / %lld ) * %lld + %llu) as "COL_STARTTIME" from "COUNT_TABLE" where "COL_TIME" >= %llu and "COL_TIME" < %llu group by "COL_STARTTIME", "COL_RULE" order by "COL_STARTTIME", "COL_RULE";"

#define EVENTDB_SQL_QUERY_FORMAT    "select "COL_RULE", "COL_IN", "COL_OUT", "COL_TIME" from "COUNT_TABLE" where "COL_TIME" >= %llu and "COL_TIME" < %llu order by "COL_RULE", "COL_TIME";"

#define ZONE_SQL_QUERY_FORMAT	    "select  a."COL_RULE", b.agg_new_enter_count as InwardCount, b.agg_total_leave_time as SumOutwardDuration, b.agg_total_count  as TotalCount, CASE WHEN b.agg_total_count == 0 THEN 0 ELSE (b.agg_total_leave_time + a."COL_AVGDURATION"* a."COL_TOTALCOUNT" - a.SumOutwardDuration)/b.agg_total_count END as AvgDuration, sum(a."COL_AVGCOUNT")/count(*) as AvgCount, b.StartTime as StartTime from "ZONE_TABLE" as a JOIN (select "COL_RULE", sum(InwardCount) as agg_new_enter_count, sum(SumOutwardDuration) as agg_total_leave_time, (sum(InwardCount)-InwardCount+TotalCount) as agg_total_count, min("COL_TIME"), ((( "COL_TIME"- %llu )/ %lld ) * %lld + %llu ) as StartTime from "ZONE_TABLE" where "COL_TIME"  >= %llu AND "COL_TIME" < %llu group by "COL_RULE",StartTime) as b ON a."COL_RULE" == b."COL_RULE" and a."COL_TIME" >= b.StartTime and (a."COL_TIME" < b.StartTime + %lld) where a."COL_TIME"  >= %llu AND a."COL_TIME" < %llu group by a."COL_RULE", b.StartTime order by a."COL_TIME", a.rowid asc;"

#define HEATMAP_SQL_FORMAT	        "select GROUP_CONCAT('{UTC:' || "COL_TIME" || case when data='' or data is null then '' else ',' || data end || '}') as all_data from "HEATMAP_TABLE" where "COL_TIME" >= %llu and "COL_TIME" <= %llu;"

// XML
//#define EVENT_ROOT_XML 			"<Message>%s%s%s</Message>"
#define EVENT_ROOT_XML_STAG 		"<Message>"
#define EVENT_ROOT_XML_ETAG 		"</Message>"

#define EVENT_SOURCE_XML            "<Source><UtcTime>%s</UtcTime><GroupID>%s</GroupID><DeviceID>%s</DeviceID><ModelName>%s</ModelName><MacAddress>%s</MacAddress><IPAddress>%s</IPAddress><TimeZone>%s</TimeZone><DST>%s</DST></Source>"

//#define EVENT_COUNT_HEAD_XML      "<Data RuleType=\"Counting\">%s</Data>"
#define EVENT_COUNT_HEAD_XML_STAG   "<Data RuleType=\"Counting\">"
#define EVENT_COUNT_HEAD_XML_ETAG   "</Data>"
#define REPORT_XML_FORMAT			"<CountingInfo RuleName=\"%s\"><In>%d</In><Out>%d</Out><StartTime>%s</StartTime><EndTime>%s</EndTime></CountingInfo>"

//#define EVENT_ZONE_HEAD_XML 		"<Data RuleType=\"ZoneDetection\">%s</Data>"
#define EVENT_ZONE_HEAD_XML_STAG 	"<Data RuleType=\"ZoneDetection\">"
#define EVENT_ZONE_HEAD_XML_ETAG 	"</Data>"
#define EVENT_ZONE_DATA_XML "<ZoneInfo RuleName=\"%s\"><InwardCount>%d</InwardCount><SumOutwardDuration>%d</SumOutwardDuration><TotalCount>%d</TotalCount><AvgDuration>%.2f</AvgDuration><AvgCount>%.2f</AvgCount><StartTime>%s</StartTime><EndTime>%s</EndTime></ZoneInfo>"

// JSON
//#define EVENT_ROOT_JSON 			"{\"Source\":{%s},\"Data\":[%s%s]}"
#define EVENT_ROOT_JSON_STAG 		"{\"Source\":{"
#define EVENT_ROOT_JSON_ETAG 		"},"
#define EVENT_DATA_JSON_STAG        "\"Data\":["
#define EVENT_DATA_JSON_ETAG        "]}"
#define EVENT_SOURCE_JSON 			"\"ReportTime\":\"%s\",\"GroupID\":\"%s\",\"DeviceID\":\"%s\",\"ModelName\":\"%s\",\"MacAddress\":\"%s\",\"IPAddress\":\"%s\",\"TimeZone\":\"%s\",\"DST\":\"%s\""

//#define EVENT_COUNT_HEAD_JSON 	"{\"RuleType\":\"Counting\",\"CountingInfo\":[%s]},"
#define EVENT_COUNT_HEAD_JSON_STAG 	"{\"RuleType\":\"Counting\",\"CountingInfo\":["
#define EVENT_COUNT_HEAD_JSON_ETAG	"]},"
#define REPORT_JSON_FORMAT			"{\"RuleName\":\"%s\",\"In\":%d,\"Out\":%d,\"StartTime\":\"%s\",\"EndTime\":\"%s\"},"

//#define EVENT_ZONE_HEAD_JSON      "{\"RuleType\": \"ZoneDetection\",\"ZoneInfo\": [%s]}"
#define EVENT_ZONE_HEAD_JSON_STAG   "{\"RuleType\": \"ZoneDetection\",\"ZoneInfo\": ["
#define EVENT_ZONE_HEAD_JSON_ETAG   "]}"
#define EVENT_ZONE_DATA_JSON        "{\"RuleName\":\"%s\",\"InwardCount\":%d,\"SumOutwardDuration\":%d,\"TotalCount\":%d,\"AvgDuration\":%.2f,\"AvgCount\":%.2f,\"StartTime\":\"%s\",\"EndTime\":\"%s\"},"

#define EVENT_SOURCE_HEATMAP_JSON 	"\"ReportTime\":\"%s\",\"ModelName\":\"%s\",\"MacAddress\":\"%s\",\"IPAddress\":\"%s\",\"TimeZone\":\"%s\",\"DST\":\"%s\""

// CSV
#define EVENT_SOURCE_CSV 			"ReportTime,GroupID,DeviceID,ModelName,MacAddress,IPAddress,TimeZone,DST\n%s,%s,%s,%s,%s,%s,%s,%s\n"
#define EVENT_COUNT_HEAD_CSV		"RuleType,RuleName,In,Out,StartTime,EndTime\n"
#define REPORT_CSV_FORMAT			"Counting,%s,%d,%d,%s,%s\n"
#define EVENT_ZONE_HEAD_CSV         "RuleType,RuleName,InwardCount,SumOutwardDuration,TotalCount,AvgDuration,AvgCount,StartTime,EndTime\n"
#define EVENT_ZONE_DATA_CSV         "ZoneDetection,%s,%d,%d,%d,%.2f,%.2f,%s,%s\n"

typedef enum {
	eUnKnown,
	eXML,
	eJSON,
	eCSV
} EReportFmt;

typedef struct tReportConf {
	EReportFmt eFmt;
	char	acCountDBPath[128];
	char	acZoneDBPath[128];
	char	acHeatmapDBPath[128];
	char    acSystemInfo[MAX_SYSINFO_LENGTH];
	char	acVCAType[128];
	UInt64  iStartTime;		// in unit of second
	Int64	iAggregation;	// in unit of second
	UInt64  iEndTime;		// in unit of second
	int		bLite;
	int		bLocal;
	int		bEventDB;
	int		iTzOffset;		// timezone offset in seconds
	int		iDST;		 	// timezone offset in seconds
	BOOL    bHeatmap;
} TReportConf;

#endif // __VCAREPORT_H__

