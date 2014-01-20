#ifndef UTILS_PIDFILE_H_INCLUDED
#define UTILS_PIDFILE_H_INCLUDED

#include <string>
#include <memory>

namespace utils
{

class PidFile
{
    private:
	PidFile(const std::string& path);

    public:
	~PidFile();

	static std::unique_ptr<PidFile> create(const std::string& path);

    private:
	std::string m_path;
};

}

#endif
