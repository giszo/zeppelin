/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef ZEPPELIN_PLAYER_QUEUE_H_INCLUDED
#define ZEPPELIN_PLAYER_QUEUE_H_INCLUDED

#include <zeppelin/library/file.h>
#include <zeppelin/library/directory.h>
#include <zeppelin/library/album.h>

#include <vector>
#include <memory>

namespace zeppelin
{
namespace player
{

class QueueItem
{
    public:
	enum Type { PLAYLIST, ALBUM, DIRECTORY, FILE };
	enum Position { FIRST, LAST };

	virtual ~QueueItem() {}

	virtual Type type() const = 0;

	// Puts the iterator value of to the end of the given list
	virtual void get(std::vector<int>&) = 0;
	// Loads the iterator value from the end of the given list
	virtual bool set(std::vector<int>&) = 0;

	// Removes the selected item from the tree
	virtual void remove(std::vector<int>&) = 0;

	// Returns true if the iterator of the item is valid.
	virtual bool isValid() const = 0;
	// Resets the iterator to the given position of the item.
	virtual void reset(Position position) = 0;
	/**
	 * Moves the iterator to the previous item.
	 * @return true is returned if the iterator is still valid
	 */
	virtual bool prev() = 0;
	/**
	 * Moves the iterator to the next item.
	 * @return true is returned if the iterator is still valid
	 */
	virtual bool next() = 0;

	// Returns the library file according to the iterator position.
	virtual const std::shared_ptr<library::File>& file() const = 0;

	// Clones the current item.
	virtual std::shared_ptr<QueueItem> clone() const = 0;

	virtual std::vector<std::shared_ptr<QueueItem>> items() const = 0;
};

/**
 * Base class for queue items that can contain other items.
 */
class ContainerQueueItem : public QueueItem
{
    public:
	void add(const std::shared_ptr<QueueItem>& item);

	void get(std::vector<int>& i) override;
	bool set(std::vector<int>& i) override;

	void remove(std::vector<int>& i) override;

	bool isValid() const override;
	void reset(Position position) override;
	bool prev() override;
	bool next() override;

	const std::shared_ptr<library::File>& file() const override;

	std::vector<std::shared_ptr<QueueItem>> items() const override;

    protected:
	ContainerQueueItem();

    protected:
	// the index of the currently active item in the playlit
	int m_index;

	// playlit items
	std::vector<std::shared_ptr<QueueItem>> m_items;
};

class File : public QueueItem
{
    public:
	File(const std::shared_ptr<library::File>& f);

	Type type() const override;

	void get(std::vector<int>&) override;
	bool set(std::vector<int>&) override;

	void remove(std::vector<int>&) override;

	bool isValid() const override;
	void reset(Position position) override;
	bool prev() override;
	bool next() override;

	const std::shared_ptr<library::File>& file() const override;

	std::shared_ptr<QueueItem> clone() const override;

	std::vector<std::shared_ptr<QueueItem>> items() const override;

    private:
	std::shared_ptr<library::File> m_file;
};

class Directory : public ContainerQueueItem
{
    public:
	Directory(const std::shared_ptr<library::Directory>& directory);

	Type type() const override;

	std::shared_ptr<QueueItem> clone() const override;

	const library::Directory& directory() const;

    private:
	// used for cloning
	Directory()
	{}

	std::shared_ptr<library::Directory> m_directory;
};

class Album : public ContainerQueueItem
{
    public:
	Album(const std::shared_ptr<library::Album>& album,
	      const std::vector<std::shared_ptr<library::File>>& files);

	Type type() const override;

	std::shared_ptr<QueueItem> clone() const override;

	const library::Album& album() const;

    private:
	// used for cloning
	Album()
	{}

	std::shared_ptr<library::Album> m_album;
};

class Playlist : public ContainerQueueItem
{
    public:
	Playlist(int id);

	void add(const std::shared_ptr<QueueItem>& item);

	// removes all items from the playlist
	void clear();

	int getId() const;

	Type type() const override;

	std::shared_ptr<QueueItem> clone() const override;

    private:
	int m_id;
};

}
}

#endif
