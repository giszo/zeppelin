/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef ZEPPELIN_LIBRARY_PICTURE_H_INCLUDED
#define ZEPPELIN_LIBRARY_PICTURE_H_INCLUDED

#include <string>
#include <vector>

namespace zeppelin
{
namespace library
{

class Picture
{
    public:
	enum Type
	{
	    FrontCover,
	    BackCover
	};

	Picture(const std::string& mimeType, const unsigned char* data, size_t size);
	Picture(const std::string& mimeType, std::vector<unsigned char>&& data);

	const std::string& getMimeType() const;
	const std::vector<unsigned char>& getData() const;

    private:
	std::string m_mimeType;
	std::vector<unsigned char> m_data;
};

}
}

#endif
