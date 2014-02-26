/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <zeppelin/library/artist.h>

using zeppelin::library::Artist;

// =====================================================================================================================
Artist::Artist(int id, const std::string& name, int albums)
    : m_id(id),
      m_name(name),
      m_albums(albums)
{
}
