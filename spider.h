#ifndef __SPIDER_H__
#define __SPIDER_H__

#include "curl/curl.h"
#include "json/json.h"

class Spider
{
public:
	const static int MAX_BUFF = 1024 * 512;

	Spider();
	virtual ~Spider();

	bool init_env();

	CURLcode json_spider(char* url, size_t max_len);

	static void* start_spider_thread(void* param);

	static size_t write_json_data(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t write_pic_data(void *ptr, size_t size, size_t nmemb, void *stream);

	void reset_data_shift();

	void pic_spider(const char* pic);

	void do_spider();

private:
	int get_buff_shift()
	{
		return m_data_shift;
	}

	void shift_buff(int shift)
	{
		m_data_shift += shift;
	}

	char* get_buff_ptr()
	{
		return m_buff;
	}

	void init_header();

	CURLcode perform();

	FILE* open_new_file(const char* path);

	FILE* get_open_file()
	{
		return m_file;
	}

	const char* parse_base_json();
	void parse_photo_json();

	void parse_space_json();

private:
	struct curl_slist *m_headers;
	CURL *m_json_curl;
	CURL *m_pic_curl;

	char m_buff[MAX_BUFF];	//0.5mb for store json
	int m_data_shift;
	int m_target_id;	//
	FILE* m_file;
	bool m_is_pic;

	Json::Value m_root;
	Json::Reader m_reader;
};

#endif
