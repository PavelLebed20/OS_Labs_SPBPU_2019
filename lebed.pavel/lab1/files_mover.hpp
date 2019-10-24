#ifndef __FILES_MOVER_H__
#define __FILES_MOVER_H__

#include "defines.hpp"
#include "logger.hpp"

static const time_t g_old_interval = 600;

class FilesMover
{
private:
	string src, dst;
	time_t interval;
	time_t last_move;
public:
	FilesMover(const string& Src, const string& Dest, const time_t Interval) :
		src(Src), dst(Dest), interval(Interval)
	{
		time(&last_move);
	}
	FilesMover() = default;

	bool MoveFiles()
	{
		struct stat t_stat;
		time_t cur_t;
		DIR *dir, *res_dir;
		struct dirent* dir_iterator;

		time(&cur_t);

		if (cur_t - last_move < interval)
			return true;

		dir = opendir(src.c_str());
		if (!dir)
		{
			g_logger.LogError("Cannot open dir: " + (src));
			return false;
		}

		while ((dir_iterator = readdir(dir)) != nullptr)
		{
			if (dir_iterator->d_type == DT_DIR)
				continue;

			std::string source_file_path = src + std::string("/") + std::string(dir_iterator->d_name);
			// check statistic
			stat(source_file_path.c_str(), &t_stat);
			time_t diff = cur_t - t_stat.st_atime;

			string dst_dir = dst + (diff >= interval ? "/OLD/" : "/NEW/");
			res_dir = opendir(dst_dir.c_str());
			if (!res_dir)
			{
				mkdir(dst_dir.c_str(), S_IRWXU);
			}
			closedir(res_dir);

			string dst_file_path = dst + (g_old_interval >= interval ? "/OLD/" : "/NEW/") +
				string(dir_iterator->d_name);

			if (!MoveFile(source_file_path.c_str(), dst_file_path.c_str()))
			{
				g_logger.LogError("Cannot move files: " + source_file_path + " to " + dst_file_path);
				closedir(dir);
				return false;
			}

		}
		closedir(dir);
		return true;
	}

	static bool MoveFile(const string& Src, const string& Dst)
	{
		return rename(Src.c_str(), Dst.c_str()) == 0;
	}

};

#endif // __FILES_MOVER_H__

