#ifndef __FILES_MOVER_H__
#define __FILES_MOVER_H__

#include "defines.hpp"
#include "logger.hpp"

class FilesMover
{
private:
	string src, dst;
	time_t update_interval, old_interval;
	time_t last_move;
public:
	FilesMover(const string& Src, const string& Dest, 
			   const time_t UpdateInterval, const time_t OldInterval) :
		src(Src), dst(Dest), update_interval(UpdateInterval), 
		old_interval(OldInterval), last_move(0)
	{
	}
	FilesMover() = default;

	bool MoveFiles()
	{
		struct stat t_stat;
		time_t cur_t;
		DIR *dir;
		struct dirent* dir_iterator;

		time(&cur_t);

		if (last_move != 0 && cur_t - last_move < update_interval)
			return true;

		last_move = cur_t;

		dir = opendir(dst.c_str());
		if (!dir)
		{
			g_logger.LogError("Cannot open dir: " + (dst));
			return false;
		}
		closedir(dir);

		// Create dst dirs 
		CreateDirIfNotExists(dst + "/OLD");
		CreateDirIfNotExists(dst + "/NEW");

		// Check source
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

			string dst_file_path = dst + (diff >= old_interval ? "/OLD/" : "/NEW/") +
				string(dir_iterator->d_name);

			if (!CopyFile(source_file_path.c_str(), dst_file_path.c_str()))
			{
				g_logger.LogError("Cannot move files: " + source_file_path + " to " + dst_file_path);
				closedir(dir);
				return false;
			}

		}
		closedir(dir);
		return true;
	}

	static bool CopyFile(const string& Src, const string& Dst)
	{
		ifstream input(Src, std::ios::binary);
		if (!input.is_open())
		{
			g_logger.LogError("Failed to open file for reading: " + Src);
			return false;
		}

		ofstream output(Dst, std::ios::binary);
		output.clear();

		if (!output.is_open())
		{		
			input.close();
			g_logger.LogError("Failed to open file for writing: " + Dst);
			return false;
		}

		std::copy(std::istreambuf_iterator<char>(input),
				  std::istreambuf_iterator<char>(),
			      std::ostreambuf_iterator<char>(output));

		input.close();
		output.close();

		return true;
	}

	static void CreateDirIfNotExists(const string &Src)
	{
		DIR *tmp_dir = opendir(Src.c_str());
		if (!tmp_dir)
		{
			mkdir(Src.c_str(), S_IRWXU);
		}
		closedir(tmp_dir);
	}

};

#endif // __FILES_MOVER_H__

