/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

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
