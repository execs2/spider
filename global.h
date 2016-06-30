#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <pthread.h>

#define MAX_PATH 256

class Global
{
public:
	static const int MSG_BUFF = 256;

	virtual ~Global();

	static Global* get_instance();

	bool set_path(const char* path);

	const char* get_path()
	{
		return m_path;
	}

	const char* get_pic_path()
	{
		return m_pic_path;
	}

	const char* get_json_path()
	{
		return m_json_path;
	}

	bool init_global_env();

	void set_target_start(int target_id)
	{
		m_target_start = target_id;
		m_log_start = target_id;
	}

	int get_target_id()
	{
		return __sync_fetch_and_add(&m_target_start, 1);
	}

	void add_spider_cnt()
	{
		__sync_fetch_and_add(&m_spider_cnt, 1);
	}

	bool is_terminated()
	{
		return m_terminated;
	}

	void set_terminated()
	{
		m_terminated = true;
	}

	void out_log(const char* sFormat, ...);

	void on_end();

private:
	Global();
	bool check_path(const char* path);

private:
	char m_path[MAX_PATH];
	char m_pic_path[MAX_PATH];
	char m_json_path[MAX_PATH];

	bool m_init;
	static Global* m_instance;
	int m_target_start;
	int m_log_start;
	int m_spider_cnt;		//爬下来的id个数
	bool m_terminated;

	static __thread char m_log_buff[MSG_BUFF];
};


#endif
