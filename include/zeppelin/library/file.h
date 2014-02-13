#ifndef ZEPPELIN_LIBRARY_FILE_H_INCLUDED
#define ZEPPELIN_LIBRARY_FILE_H_INCLUDED

#include "metadata.h"

#include <string>
#include <memory>

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

    int m_artistId;
    int m_albumId;

    std::unique_ptr<Metadata> m_metadata;
};

}
}

#endif
