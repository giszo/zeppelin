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
    : m_storage(storage),
      m_codecManager(codecManager)
{
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
	    m_cond.wait(m_mutex);

	file = m_files.front();
	m_files.pop_front();

	m_mutex.unlock();

	parse(*file);
	m_storage.setFileMetadata(*file);
    }
}

// =====================================================================================================================
void MetaParser::parse(zeppelin::library::File& file)
{
    LOG("metaparser: parsing: " << file.m_path << "/" << file.m_name);

    std::shared_ptr<codec::BaseCodec> codec = m_codecManager.create(file.m_path + "/" + file.m_name);

    if (!codec)
	return;

    try
    {
	file.m_metadata = codec->readMetadata();
    }
    catch (const codec::CodecException& e)
    {
	LOG("metaparser: error: " << e.what());
    }
}
