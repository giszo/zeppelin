/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef CODEC_CODECMANAGER_H_INCLUDED
#define CODEC_CODECMANAGER_H_INCLUDED

#include <boost/algorithm/string.hpp>

#include <string>
#include <unordered_map>
#include <functional>

namespace codec
{

class BaseCodec;

class CodecManager
{
    public:
	typedef std::function<std::shared_ptr<BaseCodec>(const std::string&)> CreateCodec;

	void registerCodec(const std::string& type, const CreateCodec& func);

	virtual std::shared_ptr<BaseCodec> create(const std::string& file) const;

	bool isMediaFile(const std::string& file) const;

    private:
	struct Hasher
	{
	    size_t operator()(const std::string& s) const
	    { return std::hash<std::string>()(boost::algorithm::to_lower_copy(s)); }
	};

	struct Comparator
	{
	    bool operator()(const std::string& s1, const std::string& s2) const
	    { return boost::iequals(s1, s2); }
	};

	typedef std::unordered_map<std::string,
				   CreateCodec,
				   Hasher,
				   Comparator> CodecMap;

    private:
	CodecMap::const_iterator findCodec(const std::string& file) const;

	CodecMap m_codecs;
};

}

#endif
