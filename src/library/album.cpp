/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <zeppelin/library/album.h>

using zeppelin::library::Album;

// =====================================================================================================================
Album::Album(int id, const std::string& name, int artistId, int songs)
    : m_id(id),
      m_name(name),
      m_artistId(artistId),
      m_songs(songs)
{
}
