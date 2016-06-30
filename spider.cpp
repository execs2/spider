#include "spider.h"
#include "global.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

Spider::Spider()
{
	m_headers = NULL;
	m_json_curl = NULL;
	m_pic_curl = NULL;
	m_file = NULL;
	m_data_shift = 0;
	m_target_id = 0;
}

Spider::~Spider()
{
	if (m_headers)
	{
		curl_slist_free_all(m_headers);
	}

	if (m_json_curl)
	{
		curl_easy_cleanup(m_json_curl);
	}

	if (m_pic_curl)
	{
		curl_easy_cleanup(m_pic_curl);
	}
}

void Spider::init_header()
{
	m_headers = curl_slist_append(m_headers, "Accept-Encoding:gzip; charset=UTF-8");
	m_headers = curl_slist_append(m_headers, "Accept:application/json");
	m_headers = curl_slist_append(m_headers, "Accept-Language:zh-Hans-CN, en-CN, en-us;q=0.8");
	m_headers = curl_slist_append(m_headers, "Authorization:Basic MTY3MDM5NTA6NHkzVzJaTVdjM01wdmpxMXQ2dHJIN1dDRWllb1JnYlh6Nkg2NmhYS0Z2WldHQkpXS2doZ0pMMmdUZFRodDhjMQ==");
	m_headers = curl_slist_append(m_headers, "Connection:keep-alive");
	m_headers = curl_slist_append(m_headers, "Host:api.nyx.l99.com");
	m_headers = curl_slist_append(m_headers, "api-version:2");
	m_headers = curl_slist_append(m_headers, "X-UDID:B22C3C4F-BDC1-46AD-A825-B5657AE49A89");
	m_headers = curl_slist_append(m_headers, "X-DoveBox-API-Version:2");
	m_headers = curl_slist_append(m_headers, "Accept-Language:zh-Hans-CN, en-CN, en-us;q=0.8");
	m_headers = curl_slist_append(m_headers, "User-Agent:com.l99.newchuangshang/4.5.4(11828, iPhone OS 9.1, iPhone6,2, Scale/2.0");
}

bool Spider::init_env()
{
	m_json_curl = curl_easy_init();
	if (!m_json_curl)
	{
		return false;
	}

	m_pic_curl = curl_easy_init();
	if (!m_pic_curl)
	{
		return false;
	}

	init_header();

	return true;
}

size_t Spider::write_json_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	Spider* spider_ptr = (Spider*)stream;
	int shift = spider_ptr->get_buff_shift();
	int max_size = MAX_BUFF - shift;
	assert(max_size >= 0);

	size_t res_size = size * nmemb;
	assert(res_size <= (size_t)max_size);

	char* data = spider_ptr->get_buff_ptr();
	memcpy(data + shift, ptr, res_size);
	spider_ptr->shift_buff(res_size);

	FILE* file = spider_ptr->get_open_file();
	if (file)
	{
		fwrite(ptr, size, nmemb, file);
	}

	return res_size;
}

size_t Spider::write_pic_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t res_size = size * nmemb;
	Spider* spider_ptr = (Spider*)stream;

	fwrite(ptr, size, nmemb, spider_ptr->get_open_file());

	return res_size;
}

CURLcode Spider::perform()
{
	reset_data_shift();
	return curl_easy_perform(m_json_curl);
}

CURLcode Spider::json_spider(char* url, size_t max_len)
{
	char tmp[64];
	sprintf(tmp, "&target_id=%d", m_target_id);
	size_t len = strlen(tmp) + 1;
	size_t str_len = strlen(url);
	assert(len + str_len < max_len);
	strcpy(url + str_len, tmp);

	curl_easy_setopt(m_json_curl, CURLOPT_URL, url);
	//curl_easy_setopt(m_curl, CURLOPT_HTTPGET);
	curl_easy_setopt(m_json_curl, CURLOPT_USERAGENT, "com.l99.newchuangshang/4.5.4(11828, iPhone OS 9.1, iPhone6,2, Scale/2.0");

	//curl_easy_setopt(m_curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(m_json_curl, CURLOPT_TIMEOUT, 20);					//下载超时时间
	curl_easy_setopt(m_json_curl, CURLOPT_CONNECTTIMEOUT, 10);	//连接超时时间

	curl_easy_setopt(m_json_curl, CURLOPT_WRITEFUNCTION, &Spider::write_json_data);
	curl_easy_setopt(m_json_curl, CURLOPT_WRITEDATA, this);

	curl_easy_setopt(m_json_curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(m_json_curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(m_json_curl, CURLOPT_HTTPHEADER, m_headers);

	curl_easy_setopt(m_json_curl, CURLOPT_FOLLOWLOCATION, 1);

	return perform();		//do it
}

void Spider::pic_spider(const char* pic)
{
	char path[MAX_PATH];
	char url[512];

	const char* pic_name = strrchr(pic, '/');
	if (!pic_name) return; //pic_name = "/xxxxxx.jpg"
	
	Global* g = Global::get_instance();
	const char* pic_dir = g->get_pic_path();
	strncpy(path, pic_dir, MAX_PATH);
	strncat(path, pic_name, strlen(pic_name));

	FILE *fptr = open_new_file(path);
	if (!fptr) return;

	static const char* cdn[] = {
		"http://7xavvq.com2.z0.glb.qiniucdn.com/",
		"http://7xavvn.com2.z0.glb.qiniucdn.com/"
	};

	bool real_url = false;
	if (strncmp(pic, "http://", strlen("http://")) ==  0)
	{
		real_url = true;
	}
	else
	{
		strncpy(url, cdn[0], 512);
		strcat(url, pic);
	}

	CURLcode ret = CURLE_OK;
	m_file = fptr;

	for (int i = 0; i < 2; ++i)
	{
		curl_easy_setopt(m_pic_curl, CURLOPT_URL, real_url ? pic : url);
		curl_easy_setopt(m_pic_curl, CURLOPT_USERAGENT, "com.l99.newchuangshang/4.5.4(11828, iPhone OS 9.1, iPhone6,2, Scale/2.0");
		curl_easy_setopt(m_pic_curl, CURLOPT_TIMEOUT, 200);         //设置下载超时时间
		curl_easy_setopt(m_pic_curl, CURLOPT_CONNECTTIMEOUT, 10); //设置连接超时时间
		curl_easy_setopt(m_pic_curl, CURLOPT_WRITEFUNCTION, &Spider::write_pic_data);
		curl_easy_setopt(m_pic_curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(m_pic_curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(m_pic_curl, CURLOPT_HTTPHEADER, 0);

		ret = curl_easy_perform(m_pic_curl);

		long retcode = 200;
		if (ret == CURLE_OK)
		{
			ret = curl_easy_getinfo(m_pic_curl, CURLINFO_RESPONSE_CODE, &retcode);
		}

		if ((ret != CURLE_OK || retcode != 200) && i + 1 < 2 && !real_url)
		{
			strncpy(url, cdn[i + 1], 512);
			strcat(url, pic);
			rewind(fptr);
		}
	}

	fclose(fptr);
	m_file = NULL;

	if (ret != CURLE_OK)
	{
		remove(path);
	}
	else
	{
		g->out_log("Download pic %s success.\n", pic_name + 1);
	}
}

const char* Spider::parse_base_json()
{
	bool ret = m_reader.parse(m_buff, m_buff + m_data_shift, m_root);
	if (!ret) return NULL;

	Global* g = Global::get_instance();

	Json::Value &code = m_root["code"];
	if (code == Json::nullValue)
	{
		g->out_log("parse_base_json error, code is null.\n");
		return NULL;
	}

	if (code.asInt() != 1000)
	{
		Json::Value &msg = m_root["message"];
		const char* str = msg != Json::nullValue ? msg.asCString() : "";
		g->out_log("parse_base_json error, code is %d. msg is %s.\n", code.asInt(), str);
		return NULL;
	}

	Json::Value &data = m_root["data"];
	if (data != Json::nullValue && data["spaceInfo"] != Json::nullValue && data["spaceInfo"]["gender"] != Json::nullValue)
	{
		Json::Value &spaceInfo = data["spaceInfo"];
		int gender = spaceInfo["gender"].asInt();
		if (gender != 0)  //only female
		{
			return NULL;
		}

		Json::Value &photo_path = spaceInfo["photo_path"];
		if (photo_path == Json::nullValue) return NULL;

		const char *pp = photo_path.asCString();
		if (!pp) return NULL;

		//不要使用默认头像的资料
		if (strcmp(pp, "male.jpg") == 0 || strcmp(pp, "female.jpg") == 0)
		{
			g->out_log("target_id %d use  defaul pic %s, pass.\n", m_target_id, pp);
			return NULL;
		}

		return pp;
	}
	else
	{
		g->out_log("parse_base_json error, data format wrong. target_id %d.\n", m_target_id);
		return NULL;
	}
}

void Spider::parse_photo_json()
{
	bool ret = m_reader.parse(m_buff, m_buff + m_data_shift, m_root);
	if (!ret) return;

	Global* g = Global::get_instance();

	Json::Value &code = m_root["code"];
	if (code == Json::nullValue)
	{
		g->out_log("parse_photo_json error, code is null.\n");
		return;
	}

	if (code.asInt() != 1000)
	{
		Json::Value &msg = m_root["message"];
		const char* str = msg != Json::nullValue ? msg.asCString() : "";
		g->out_log("parse_photo_json error, code is %d. msg is %s.\n", code.asInt(), str);
		return;
	}

	Json::Value &data = m_root["data"];
	if (data != Json::nullValue && data["photos"] != Json::nullValue)
	{
		Json::Value &photos = data["photos"];
		unsigned int size = photos.size();
		for (unsigned int i = 0; i < size; ++i)
		{
			Json::Value &one_photo = photos[i];
			const char* path = one_photo["path"].asCString();
			pic_spider(path);
		}
	}
}

void Spider::parse_space_json()
{
	bool ret = m_reader.parse(m_buff, m_buff + m_data_shift, m_root);
	if (!ret) return;

	Global* g = Global::get_instance();
	Json::Value &code = m_root["code"];
	if (code == Json::nullValue)
	{
		g->out_log("parse_space_json error, code is null.\n");
		return;
	}

	if (code.asInt() != 200)
	{
		Json::Value &msg = m_root["message"];
		const char* str = msg != Json::nullValue ? msg.asCString() : "";
		g->out_log("parse_photo_json error, code is %d. msg is %s.\n", code.asInt(), str);
		return;
	}

	Json::Value &data = m_root["data"];
	if (data != Json::nullValue && data["dashboards"] != Json::nullValue)
	{
		Json::Value &dashboards = data["dashboards"];
		unsigned int size = dashboards.size();
		for (unsigned int i = 0; i < size; ++i)
		{
			Json::Value &photos = dashboards[i]["text_images"];
			if (photos != Json::nullValue)
			{
				unsigned int p_cnt = photos.size();
				for (unsigned int j = 0; j < p_cnt; ++j)
				{
					const char* path = photos[j].asCString();
					pic_spider(path);
				}
			}
		}
	}
}

void Spider::do_spider()
{
	Global* g = Global::get_instance();
	m_target_id = g->get_target_id();

	char json[MAX_PATH];
	char tmp[32];
	const char* json_path = g->get_json_path();
	strncpy(json, json_path, MAX_PATH);
	int len = strlen(json);

	sprintf(tmp, "/%d_base.json", m_target_id);
	strcat(json, tmp);

	FILE* fptr = open_new_file(json);
	if (!fptr)
	{
		g->out_log("target_id %d had downloaded. \n", m_target_id);
		return;
	}

	g->out_log("Target id %d start . \n", m_target_id);

	//基本信息 step 1
	char url_base[512] = { "https://api.nyx.l99.com/space/info/viewall?machine_code=B22C3C4F-BDC1-46AD-A825-B5657AE49A89&version=4.5.4&idfa=CFD0100E-C97E-4E7B-AB08-325564F2E47C&dev_token=32b3f249%20c395f65a%2059908cb0%20619a1320%209d69f4d3%20b912e5bc%20f70f2434%202ebd48f0&api_version=2&type=0&lat=23.128961&format=json&language=zh_CN&limit=20&lng=113.333376&client=key%3ABedForiPhone" };
	CURLcode ret = json_spider(url_base, 512);
	if (ret != CURLE_OK)
	{
		fclose(fptr);
		remove(json);

		g->out_log("target_id %d get base json failed. code:%d.\n", m_target_id, ret);
		return;
	}

	const char* head_pic = parse_base_json();
	if (!head_pic)
	{
		fclose(fptr);
		remove(json);
		return;
	}

	fwrite(m_buff, m_data_shift, 1, fptr);
	fclose(fptr);
	
	pic_spider(head_pic);
	
	//相册 step 2
	sprintf(tmp, "/%d_photo.json", m_target_id);
	strcpy(json + len, tmp);

	fptr = open_new_file(json);
	if (fptr)
	{
		m_file = fptr;
		char url_photos[512] = { "https://api.nyx.l99.com/account/photo/view?format=json&machine_code=B22C3C4F-BDC1-46AD-A825-B5657AE49A89&client=key%3ABedForiPhone&dev_token=32b3f249%20c395f65a%2059908cb0%20619a1320%209d69f4d3%20b912e5bc%20f70f2434%202ebd48f0&limit=15&language=zh_CN&version=4.5.4&lat=23.128950&lng=113.333376&idfa=CFD0100E-C97E-4E7B-AB08-325564F2E47C" };
		CURLcode ret = json_spider(url_photos, 512);
		fclose(fptr);
		m_file = NULL;

		if (ret != CURLE_OK)
		{
			remove(json);
		}
		else
		{
			parse_photo_json();
		}
	}

	//动态 step 3
	sprintf(tmp, "/%d_space.json", m_target_id);
	strcpy(json + len, tmp);

	fptr = open_new_file(json);
	if (fptr)
	{
		m_file = fptr;
		char url_space[512] = { "https://api.nyx.l99.com/content/dovebox/list?machine_code=B22C3C4F-BDC1-46AD-A825-B5657AE49A89&version=4.5.4&idfa=CFD0100E-C97E-4E7B-AB08-325564F2E47C&dev_token=32b3f249%20c395f65a%2059908cb0%20619a1320%209d69f4d3%20b912e5bc%20f70f2434%202ebd48f0&api_version=2&lat=23.129054&format=json&language=zh_CN&limit=20&lng=113.333431&client=key%3ABedForiPhone" };
		CURLcode ret = json_spider(url_space, 512);
		fclose(fptr);
		m_file = NULL;

		if (ret != CURLE_OK)
		{
			remove(json);
		}
		else
		{
			parse_space_json();
		}
	}

	g->add_spider_cnt();
	g->out_log("Target id %d end . \n", m_target_id);
}

FILE* Spider::open_new_file(const char* path)
{
	if (access(path, F_OK) == 0)
	{
		//已经有这个文件了
		return NULL;
	}

	FILE *fptr = fopen(path, "w");
	if (!fptr)
	{
		//errno
		return NULL;
	}

	return fptr;
}

void* Spider::start_spider_thread(void* param)
{
	Global* g = Global::get_instance();

	Spider s;
	bool ret = s.init_env();
	if (!ret)
	{
		g->out_log("init_env error, exiting... \n");
		return NULL;
	}

	while (!g->is_terminated())
	{
		s.do_spider();
	}

	g->out_log("Exiting... \n");
	return NULL;
}

void Spider::reset_data_shift()
{
	m_data_shift = 0;
}
