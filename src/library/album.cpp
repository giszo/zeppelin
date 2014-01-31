#include <zeppelin/library/album.h>

using zeppelin::library::Album;

// =====================================================================================================================
Album::Album(int id, const std::string& name, int artistId, int songs)
    : m_id(id),
      m_name(name),
      m_artistId(artistId),
      m_songs(songs)
{
}
