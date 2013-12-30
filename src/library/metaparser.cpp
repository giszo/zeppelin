#include "metaparser.h"
#include "musiclibrary.h"

#include <codec/basecodec.h>
#include <thread/blocklock.h>
#include <logger.h>

using library::MetaParser;

// =====================================================================================================================
MetaParser::MetaParser(MusicLibrary& library)
    : m_library(library)
{
}

// =====================================================================================================================
void MetaParser::add(const std::shared_ptr<File>& file)
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

	auto files = m_library.getStorage().getFilesWithoutMetadata();
	for (const auto& f : files)
	    m_files.push_back(f);
    }

    while (1)
    {
	std::shared_ptr<File> file;

	m_mutex.lock();

	while (m_files.empty())
	    m_cond.wait(m_mutex);

	file = m_files.front();
	m_files.pop_front();

	m_mutex.unlock();

	parse(*file);
	m_library.getStorage().updateFileMetadata(*file);
    }
}

// =====================================================================================================================
void MetaParser::parse(File& file)
{
    LOG("Parsing meta information of " << file.m_path << "/" << file.m_name);

    std::shared_ptr<codec::BaseCodec> codec = codec::BaseCodec::create(file.m_path + "/" + file.m_name);

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
    file.m_artist = meta.m_artist;
    file.m_album = meta.m_album;
    file.m_title = meta.m_title;
    file.m_year = meta.m_year;
    file.m_trackIndex = meta.m_trackIndex;
    file.m_type = meta.m_type;
    file.m_samplingRate = meta.m_rate;
}
