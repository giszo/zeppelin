/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

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
