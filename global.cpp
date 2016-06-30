#include "global.h"
#include "curl/curl.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>

Global* Global::m_instance = NULL;
__thread char Global::m_log_buff[MSG_BUFF] = {};

Global::Global()
{
	m_init = false;
	m_path[0] = '\0';
	m_pic_path[0] = '\0';
	m_json_path[0] = '\0';
	m_terminated = false;
	m_spider_cnt = 0;
}

Global::~Global()
{
	if (m_init)
	{
		curl_global_cleanup();
	}

	on_end();
}

bool Global::check_path(const char* path)
{
	if (!opendir(path))
	{
		if (errno != ENOENT)
		{
			fprintf(stderr, "open path %s error.%s. \n", path, strerror(errno));
			return false;
		}

		if (mkdir(path, 0777) != 0)
		{
			fprintf(stderr, "create path %s error.%s. \n", path, strerror(errno));
			return false;
		}
	}

	return true;
}

bool Global::set_path(const char* path)
{
	if (!path || path[0] == '\0') return false;

	strncpy(m_path, path, MAX_PATH);

	int len = strlen(m_path);
	if (m_path[len - 1] == '/')
	{
		m_path[len - 1] = '\0';
	}

	strncpy(m_pic_path, path, MAX_PATH);
	strcat(m_pic_path, "/pics");
	strncpy(m_json_path, path, MAX_PATH);
	strcat(m_json_path, "/jsons");

	if (!check_path(path)) return false;
	if (!check_path(m_pic_path)) return false;
	if (!check_path(m_json_path)) return false;

	return true;
}

Global* Global::get_instance()
{
	if (!m_instance)
		m_instance = new Global();

	return m_instance;
}

bool Global::init_global_env()
{
	if (m_init) return false;

	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != ret)
	{
		return false;
	}

	m_init = true;
	return true;
}

void Global::out_log(const char* sFormat, ...)
{
	time_t tNow = time(NULL);
	struct tm ptm = {};
	localtime_r(&tNow, &ptm);
	strftime(m_log_buff, MSG_BUFF, "[%Y-%m-%d %H:%M:%S]", &ptm);
	
	int len = strlen(m_log_buff);
	sprintf(m_log_buff + len, "Thread[%lu]->", pthread_self());
	len = strlen(m_log_buff);

	va_list	ap;
	va_start(ap, sFormat);
	vsnprintf(m_log_buff + len, MSG_BUFF - len, sFormat, ap);
	va_end(ap);
	m_log_buff[MSG_BUFF - 1] = '\0';

	fprintf(stderr, m_log_buff);
}

void Global::on_end()
{
	char path[MAX_PATH];
	strcpy(path, m_path);
	strcat(path, "/spider_log.txt");

	FILE *fptr = fopen(path, "a");
	if (!fptr) return;
	
	time_t tNow = time(NULL);
	struct tm ptm = {};
	localtime_r(&tNow, &ptm);
	strftime(m_log_buff, MSG_BUFF, "[%Y-%m-%d %H:%M:%S]", &ptm);
	int len = strlen(m_log_buff);

	sprintf(m_log_buff + len, "Start-->[%d], End-->[%d], Available-->[%d].\r\n", m_log_start, m_target_start - 1, m_spider_cnt);

	fprintf(fptr, m_log_buff);

	fclose(fptr);
	fprintf(stderr, m_log_buff);
}
