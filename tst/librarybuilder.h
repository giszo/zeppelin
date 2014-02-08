#ifndef LIBRARYBUILDER_H_INCLUDED
#define LIBRARYBUILDER_H_INCLUDED

#include <zeppelin/library/file.h>
#include <zeppelin/library/album.h>

struct LibraryBuilder
{
    std::shared_ptr<zeppelin::library::File> createFile(int id, const std::string& name)
    {
	std::shared_ptr<zeppelin::library::File> file(new zeppelin::library::File(id));
	file->m_name = name;
	return file;
    }

    std::shared_ptr<zeppelin::library::Album> createAlbum(int id, const std::string& name)
    {
	return std::make_shared<zeppelin::library::Album>(id, name, 0, 0);
    }
};

#endif
