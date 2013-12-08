#include "parsefiles.h"
#include "musiclibrary.h"

#include <codec/basecodec.h>

#include <iostream>

using library::ParseFiles;

// =====================================================================================================================
ParseFiles::ParseFiles(MusicLibrary& library)
    : m_library(library)
{
}

// =====================================================================================================================
void ParseFiles::run()
{
    std::vector<std::shared_ptr<library::File>> files;

    // get a set of new files to work on
    files = m_library.getNewFiles(10 /* 10 files per round */);

    for (const auto& f : files)
    {
	if (!parse(*f))
	    m_library.updateMeta(f->m_id, 0);
    }

    // schedule a new round of file parsing work if the currnet one got files because there could be more ...
    if (!files.empty())
	m_library.addWork(std::make_shared<ParseFiles>(m_library));
}

// =====================================================================================================================
bool ParseFiles::parse(const library::File& file)
{
    std::cout << "Parsing meta information of " << file.m_path << "/" << file.m_name << std::endl;

    std::shared_ptr<codec::BaseCodec> codec = codec::BaseCodec::openFile(file.m_path + "/" + file.m_name);

    if (!codec)
	return false;

    codec::MediaInfo info;

    try
    {
	info = codec->getMediaInfo();
    }
    catch (const codec::CodecException& e)
    {
	return false;
    }

    size_t seconds = info.m_samples / info.m_rate;
    std::cout << "Length: " << seconds << " secs" << std::endl;

    // update the metadate of the file
    m_library.updateMeta(file.m_id, seconds);

    return true;
}
