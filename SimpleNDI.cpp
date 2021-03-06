// Multithread receiving example
#include <thread>
#include <Processing.NDI.Lib.h>

namespace
{
	NDIlib_recv_instance_t recv_instance;
	std::string source_name;
	int recv_count;
	bool recv_stop;

	void recv_listen()
	{
		auto find = NDIlib_find_create_v2(&NDIlib_find_create_t());

		if (find == nullptr)
		{
			std::fputs("Failed to create a find object.", stderr);
			std::exit(EXIT_FAILURE);
		}

		const NDIlib_source_t* sources = nullptr;

		while (true)
		{
			NDIlib_find_wait_for_sources(find, 1000);

			uint32_t count;
			sources = NDIlib_find_get_current_sources(find, &count);
			if (count > 0) break;
		}

		source_name = sources[0].p_ndi_name;

		auto recv = NDIlib_recv_create_v2(&NDIlib_recv_create_t(sources[0]));

		if (recv == nullptr)
		{
			std::fputs("Failed to create a receiver object.", stderr);
			std::exit(EXIT_FAILURE);
		}

		NDIlib_find_destroy(find);

		recv_instance = recv;
	}

	void recv_frame()
	{
		while (!recv_stop)
		{
			NDIlib_video_frame_v2_t frame;

			if (NDIlib_recv_capture_v2(recv_instance, &frame, nullptr, nullptr, 0) == NDIlib_frame_type_video)
			{
				recv_count++;
				NDIlib_recv_free_video_v2(recv_instance, &frame);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	if (!NDIlib_initialize())
	{
		std::fputs("Failed to initialize the NDI library.", stderr);
		std::exit(EXIT_FAILURE);
	}

	std::thread listen_thread(recv_listen);

	while (recv_instance == nullptr)
	{
		std::puts("Listening...");
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::printf("Source found (%s).\n", source_name.c_str());

	listen_thread.join();

	std::thread frame_thread(recv_frame);

	while (recv_count < 300)
	{
		std::printf("%d frames received.\n", recv_count);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	recv_stop = true;
	frame_thread.join();

	NDIlib_recv_destroy(recv_instance);
	NDIlib_destroy();

	return 0;
}