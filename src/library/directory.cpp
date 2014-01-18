#include <zeppelin/library/directory.h>

using zeppelin::library::Directory;

// =====================================================================================================================
Directory::Directory(int id, const std::string& name)
    : m_id(id),
      m_name(name)
{
}
