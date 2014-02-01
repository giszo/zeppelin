#ifndef ZEPPELIN_LIBRARY_DIRECTORY_H_INCLUDED
#define ZEPPELIN_LIBRARY_DIRECTORY_H_INCLUDED

#include <string>

namespace zeppelin
{
namespace library
{

struct Directory
{
    Directory(int id, const std::string& name, int parentId = 0);

    // the id of the directory
    int m_id;
    // the name of the directory
    std::string m_name;
    // the parent id of the directory
    int m_parentId;
};

}
}

#endif
