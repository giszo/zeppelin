#ifndef ZEPPELIN_LIBRARY_ALBUM_H_INCLUDED
#define ZEPPELIN_LIBRARY_ALBUM_H_INCLUDED

#include <string>

namespace zeppelin
{
namespace library
{

struct Album
{
    Album(int id, const std::string& name, int artist, int songs, int length);

    int m_id;
    std::string m_name;
    int m_artist;
    /// number of songs in this album
    int m_songs;
    /// length of the album in seconds
    int m_length;
};

}
}

#endif
