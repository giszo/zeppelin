#ifndef ZEPPELIN_LIBRARY_ARTIST_H_INCLUDED
#define ZEPPELIN_LIBRARY_ARTIST_H_INCLUDED

#include <string>

namespace zeppelin
{
namespace library
{

struct Artist
{
    Artist(int id, const std::string& name, int albums);

    int m_id;
    std::string m_name;
    int m_albums;
};

}
}

#endif
