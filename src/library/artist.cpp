#include <zeppelin/library/artist.h>

using zeppelin::library::Artist;

// =====================================================================================================================
Artist::Artist(int id, const std::string& name, int albums)
    : m_id(id),
      m_name(name),
      m_albums(albums)
{
}
