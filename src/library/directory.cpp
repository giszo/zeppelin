#include <zeppelin/library/directory.h>

using zeppelin::library::Directory;

// =====================================================================================================================
Directory::Directory(int id, const std::string& name, int parentId)
    : m_id(id),
      m_name(name),
      m_parentId(parentId)
{
}
