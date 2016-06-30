#include "spider.h"
#include "global.h"

#include <stdlib.h>
#include <string.h>

void show_usage()
{
	fprintf(stderr, "Usage:\nspider -t thread_count -p save path -i start_id\n");
}

void start_thread(int cnt)
{
	pthread_t *pid = new pthread_t[cnt];

	for (int i = 0; i < cnt; ++i)
	{
		if (pthread_create(&pid[i], NULL, Spider::start_spider_thread, NULL)) {
			fprintf(stderr, "Create server thread failed.\n");
			exit(1);
		}
	}

	for (int i = 0; i < cnt; ++i)
	{
		pthread_join(pid[i], NULL);
	}

	delete []pid;
}

int main(int argc, const char*argv[]) {
	if (argc < 7)
	{
		show_usage();
		return -1;
	}

	int thread_cnt = 0;
	const char* path = NULL;
	int start_id = 0;

	for (int i = 1; i < argc; ++i)
	{
		if (i % 2 == 0 || i + 1 >= argc) continue;

		if (strncmp(argv[i], "-t", 2) == 0)
		{
			thread_cnt = atoi(argv[i + 1]);
			continue;
		}

		if (strncmp(argv[i], "-p", 2) == 0)
		{
			path = argv[i + 1];
			continue;
		}

		if (strncmp(argv[i], "-i", 2) == 0)
		{
			start_id = atoi(argv[i + 1]);
			continue;
		}
	}

	if (thread_cnt <= 0 || start_id <= 0 || !path || path[0] == '\0')
	{
		show_usage();
		return -1;
	}

	Global* g = Global::get_instance();
	if (!g->set_path(path))
	{
		delete g;
		return -1;
	}

	g->set_target_start(start_id);
	g->init_global_env();

	start_thread(thread_cnt);

	delete g;
	return 0;
}
