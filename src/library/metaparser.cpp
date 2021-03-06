/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "metaparser.h"
#include "musiclibrary.h"

#include <codec/codecmanager.h>
#include <codec/basecodec.h>
#include <thread/blocklock.h>

#include <zeppelin/logger.h>
#include <zeppelin/library/storage.h>

using library::MetaParser;

// =====================================================================================================================
MetaParser::MetaParser(const codec::CodecManager& codecManager,
		       zeppelin::library::Storage& storage)
    : m_running(false),
      m_storage(storage),
      m_codecManager(codecManager)
{
}

// =====================================================================================================================
bool MetaParser::isRunning() const
{
    thread::BlockLock bl(m_mutex);
    return m_running;
}

// =====================================================================================================================
void MetaParser::add(const std::shared_ptr<zeppelin::library::File>& file)
{
    thread::BlockLock bl(m_mutex);
    m_files.push_back(file);
    m_cond.signal();
}

// =====================================================================================================================
void MetaParser::run()
{
    // initially fill work queue with files from the database having no metadata...
    {
	thread::BlockLock bl(m_mutex);

	auto files = m_storage.getFilesWithoutMetadata();
	for (const auto& f : files)
	    m_files.push_back(f);
    }

    while (1)
    {
	std::shared_ptr<zeppelin::library::File> file;

	m_mutex.lock();

	while (m_files.empty())
	{
	    m_running = false;
	    m_cond.wait(m_mutex);
	}

	file = m_files.front();
	m_files.pop_front();
	m_running = true;

	m_mutex.unlock();

	if (parse(*file))
	    m_storage.setFileMetadata(*file);
    }
}

// =====================================================================================================================
bool MetaParser::parse(zeppelin::library::File& file)
{
    LOG("metaparser: parsing: " << file.m_path << "/" << file.m_name);

    std::shared_ptr<codec::BaseCodec> codec = m_codecManager.create(file.m_path + "/" + file.m_name);

    if (!codec)
	return false;

    try
    {
	file.m_metadata = codec->readMetadata();
    }
    catch (const codec::CodecException& e)
    {
	LOG("metaparser: error: " << e.what());
	return false;
    }

    return true;
}
