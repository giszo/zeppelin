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
    files = m_library.getStorage().getFilesWithoutMetadata(10 /* 10 files per round */);

    for (const auto& f : files)
    {
	parse(*f);
	m_library.getStorage().updateFileMetadata(*f);
    }

    // schedule a new round of file parsing work if the currnet one got files because there could be more ...
    if (!files.empty())
	m_library.addWork(std::make_shared<ParseFiles>(m_library));
}

// =====================================================================================================================
void ParseFiles::parse(library::File& file)
{
    std::cout << "Parsing meta information of " << file.m_path << "/" << file.m_name << std::endl;

    std::shared_ptr<codec::BaseCodec> codec = codec::BaseCodec::openFile(file.m_path + "/" + file.m_name);

    if (!codec)
	return;

    codec::Metadata meta;

    try
    {
	meta = codec->getMetadata();
    }
    catch (const codec::CodecException& e)
    {
	return;
    }

    file.m_length = meta.m_samples / meta.m_rate;
    file.m_artist = meta.m_artist;
    file.m_album = meta.m_album;
    file.m_title = meta.m_title;
    file.m_year = meta.m_year;
}
