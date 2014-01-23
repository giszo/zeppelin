#include <zeppelin/library/file.h>

using zeppelin::library::File;

// =====================================================================================================================
File::File(int id)
    : m_id(id),
      m_size(0),
      m_length(0),
      m_artistId(-1),
      m_albumId(-1),
      m_year(0),
      m_trackIndex(0),
      m_samplingRate(0)
{
}
