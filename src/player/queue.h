#ifndef PLAYER_QUEUE_H_INCLUDED
#define PLAYER_QUEUE_H_INCLUDED

#include <library/musiclibrary.h>

#include <vector>
#include <memory>

namespace player
{

class QueueItem
{
    public:
	enum Type { PLAYLIST, FILE };

	virtual ~QueueItem() {}

	virtual Type type() const = 0;

	// Puts the iterator value of to the end of the given list
	virtual void get(std::vector<int>&) = 0;
	// Loads the iterator value from the end of the given list
	virtual void set(std::vector<int>&) = 0;

	// Returns true if the iterator of the item is valid.
	virtual bool isValid() const = 0;
	// Resets the iterator to the beginning of the item.
	virtual void reset() = 0;
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

class File : public QueueItem
{
    public:
	File(const std::shared_ptr<library::File>& f);

	Type type() const override;

	void get(std::vector<int>&) override;
	void set(std::vector<int>&) override;

	bool isValid() const override;
	void reset() override;
	bool prev() override;
	bool next() override;

	const std::shared_ptr<library::File>& file() const override;

	std::shared_ptr<QueueItem> clone() const override;

	std::vector<std::shared_ptr<QueueItem>> items() const override;

    private:
	std::shared_ptr<library::File> m_file;
};

class Playlist : public QueueItem
{
    public:
	Playlist();

	void add(const std::shared_ptr<library::File>& f);

	Type type() const override;

	void get(std::vector<int>& i) override;
	void set(std::vector<int>& i) override;

	bool isValid() const override;
	void reset() override;
	bool prev() override;
	bool next() override;

	const std::shared_ptr<library::File>& file() const override;

	std::shared_ptr<QueueItem> clone() const override;

	std::vector<std::shared_ptr<QueueItem>> items() const override;

    private:
	// the index of the currently active item in the playlit
	int m_index;

	// playlit items
	std::vector<std::shared_ptr<QueueItem>> m_items;
};

}

#endif
