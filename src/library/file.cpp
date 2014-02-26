/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <zeppelin/library/file.h>

using zeppelin::library::File;

// =====================================================================================================================
File::File(int id)
    : m_id(id),
      m_size(0),
      m_artistId(-1),
      m_albumId(-1)
{
}
