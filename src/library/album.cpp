#include <zeppelin/library/album.h>

using zeppelin::library::Album;

// =====================================================================================================================
Album::Album(int id, const std::string& name, int artist, int songs, int length)
    : m_id(id),
      m_name(name),
      m_artist(artist),
      m_songs(songs),
      m_length(length)
{
}
