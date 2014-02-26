/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef ZEPPELIN_LIBRARY_PLAYLIST_H_INCLUDED
#define ZEPPELIN_LIBRARY_PLAYLIST_H_INCLUDED

#include <string>
#include <vector>

namespace zeppelin
{
namespace library
{

struct PlaylistItem
{
    int m_id;
    std::string m_type;
    int m_itemId;
};

struct Playlist
{
    int m_id;
    std::string m_name;
    std::vector<PlaylistItem> m_items;
};

}
}

#endif
