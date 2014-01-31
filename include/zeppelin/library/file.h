#ifndef ZEPPELIN_LIBRARY_FILE_H_INCLUDED
#define ZEPPELIN_LIBRARY_FILE_H_INCLUDED

#include <string>

namespace zeppelin
{
namespace library
{

struct File
{
    File(int id);

    int m_id;
    int m_directoryId;
    std::string m_path;
    std::string m_name;
    // size of the file in bytes
    int64_t m_size;

    /// the length of the music file in seconds
    int m_length;

    // TODO: now we are using these fields only for inserting metadata into the database... this should
    // be replaced with some kind of public metadata structure
    std::string m_artist;
    std::string m_album;

    int m_artistId;
    int m_albumId;
    std::string m_title;
    int m_year;
    int m_trackIndex;

    // the audio codec of the file (mp3, flac, etc.)
    std::string m_codec;
    // sampling rate of the file (44100Hz, 48000Hz, etc.)
    int m_samplingRate;
};

}
}

#endif
