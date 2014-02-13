#include <zeppelin/library/file.h>

using zeppelin::library::File;

// =====================================================================================================================
File::File(int id)
    : m_id(id),
      m_size(0),
      m_artistId(-1),
      m_albumId(-1)
{
}
