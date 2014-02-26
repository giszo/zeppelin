/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef ZEPPELIN_LIBRARY_ALBUM_H_INCLUDED
#define ZEPPELIN_LIBRARY_ALBUM_H_INCLUDED

#include <string>

namespace zeppelin
{
namespace library
{

struct Album
{
    Album(int id, const std::string& name, int artistId, int songs);

    int m_id;
    std::string m_name;
    int m_artistId;
    /// number of songs in this album
    int m_songs;
};

}
}

#endif
