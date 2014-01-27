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

    codec::Metadata meta;

    try
    {
	meta = codec->readMetadata();
    }
    catch (const codec::CodecException& e)
    {
	LOG("metaparser: error: " << e.what());
	return;
    }

    file.m_length = meta.m_samples / meta.m_rate;
    file.m_artist = meta.getArtist();
    file.m_album = meta.getAlbum();
    file.m_title = meta.getTitle();
    file.m_year = meta.m_year;
    file.m_trackIndex = meta.m_trackIndex;
    file.m_codec = meta.m_codec;
    file.m_samplingRate = meta.m_rate;
}
