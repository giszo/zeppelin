#include <boost/test/unit_test.hpp>

#include <zeppelin/player/queue.h>

using zeppelin::player::Playlist;

struct PlaylistFixture
{
    PlaylistFixture()
	: m_playlist(-1)
    {}

    void queueFile(const std::shared_ptr<zeppelin::library::File>& file)
    {
	m_playlist.add(std::make_shared<zeppelin::player::File>(file));
    }

    void queueAlbum(int id, const std::string& name, const std::vector<std::shared_ptr<zeppelin::library::File>>& files)
    {
	m_playlist.add(
		std::make_shared<zeppelin::player::Album>(
		    std::make_shared<zeppelin::library::Album>(id, name, 0, 0),
		    files));
    }

    std::shared_ptr<zeppelin::library::File> createFile(int id, const std::string& name)
    {
	std::shared_ptr<zeppelin::library::File> file(new zeppelin::library::File(id));
	file->m_name = name;
	return file;
    }

    zeppelin::player::Playlist m_playlist;
};

BOOST_FIXTURE_TEST_CASE(TestPlaylist, PlaylistFixture)
{
    std::vector<int> iter;

    BOOST_CHECK_EQUAL(m_playlist.type(), zeppelin::player::QueueItem::PLAYLIST);
    BOOST_CHECK(!m_playlist.isValid());
    BOOST_CHECK(!m_playlist.prev());
    BOOST_CHECK(!m_playlist.next());

    // queue 2 files and an album
    queueFile(createFile(1, "a.mp3"));
    queueAlbum(57, "Album", {createFile(3, "1.mp3"), createFile(4, "2.mp3")});
    queueFile(createFile(2, "b.mp3"));

    // the playlist should be invalid still
    BOOST_CHECK(!m_playlist.isValid());

    // reset it to be in a known state
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);

    // check the first item
    BOOST_CHECK(m_playlist.isValid());
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "a.mp3");

    // make sure prev() does not work now
    BOOST_CHECK(!m_playlist.prev());
    BOOST_CHECK(m_playlist.isValid());

    // step to the next item
    BOOST_CHECK(m_playlist.next());
    BOOST_CHECK(m_playlist.isValid());
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "1.mp3");

    // ... next again
    BOOST_CHECK(m_playlist.next());
    BOOST_CHECK(m_playlist.isValid());
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 1);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "2.mp3");

    // ... next
    BOOST_CHECK(m_playlist.next());
    BOOST_CHECK(m_playlist.isValid());
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 2);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "b.mp3");

    // make sure next() does not work now
    BOOST_CHECK(!m_playlist.next());
    BOOST_CHECK(m_playlist.isValid());

    // step to the previous item
    BOOST_CHECK(m_playlist.prev());
    BOOST_CHECK(m_playlist.isValid());
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 1);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "2.mp3");

    // prev
    BOOST_CHECK(m_playlist.prev());
    BOOST_CHECK(m_playlist.isValid());
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "1.mp3");

    // ... prev
    BOOST_CHECK(m_playlist.prev());
    BOOST_CHECK(m_playlist.isValid());
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "a.mp3");

    // make sure prev is still not working at the beginnig
    BOOST_CHECK(!m_playlist.prev());

    // test index setting
    iter.clear();
    iter.push_back(1);
    iter.push_back(0);
    BOOST_CHECK(m_playlist.set(iter));
    BOOST_CHECK(m_playlist.isValid());
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "1.mp3");
}

BOOST_FIXTURE_TEST_CASE(TestFileDeletionBeforeActive, PlaylistFixture)
{
    std::vector<int> iter;

    queueFile(createFile(1, "a.mp3"));
    queueFile(createFile(2, "b.mp3"));
    queueFile(createFile(3, "c.mp3"));

    // go to the second song
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);
    m_playlist.next();

    // validate the iterator
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 1);

    // remove the first song
    iter.clear();
    iter.push_back(0);
    m_playlist.remove(iter);

    // validate the iterator again
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "b.mp3");
}

BOOST_FIXTURE_TEST_CASE(TestActiveFileDeletionInsideAlbum, PlaylistFixture)
{
    std::vector<int> iter;

    queueAlbum(42, "Album", {createFile(1, "1.mp3"), createFile(2, "2.mp3"), createFile(3, "3.mp3")});

    // go to the second song of the album
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);
    m_playlist.next();

    // validate the iterator
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 1);

    // remove the current file
    m_playlist.remove(iter);

    // validate the iterator now
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 1);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "3.mp3");
}

BOOST_FIXTURE_TEST_CASE(TestActiveFileDeletionAtTheEndOfAlbum, PlaylistFixture)
{
    std::vector<int> iter;

    queueAlbum(42, "Album", {createFile(1, "1.mp3"), createFile(2, "2.mp3")});
    queueAlbum(57, "Album2", {createFile(3, "a.mp3"), createFile(4, "b.mp3")});

    // go to the second song of the first album
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);
    m_playlist.next();

    // validate the iterator
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 1);

    // remove the current file
    m_playlist.remove(iter);

    // validate the iterator now
    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(m_playlist.file()->m_name, "a.mp3");
}

BOOST_FIXTURE_TEST_CASE(TestInvalidationAfterDeleteActive, PlaylistFixture)
{
    std::vector<int> iter;

    queueFile(createFile(1, "a.mp3"));

    BOOST_CHECK(!m_playlist.isValid());
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);
    BOOST_CHECK(m_playlist.isValid());

    iter.push_back(0);
    m_playlist.remove(iter);
    BOOST_CHECK(!m_playlist.isValid());

    // queue should be still invalid after adding a new item
    queueFile(createFile(2, "b.mp3"));

    BOOST_CHECK(!m_playlist.isValid());
}

BOOST_FIXTURE_TEST_CASE(TestRemovalOfEmptyAlbum, PlaylistFixture)
{
    std::vector<int> iter = {0, 0};

    queueAlbum(42, "Album", {createFile(1, "1.mp3")});
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);

    BOOST_CHECK(m_playlist.isValid());

    // remove the only file from the album, the album should be removed automatically because it is empty
    m_playlist.remove(iter);

    BOOST_CHECK(!m_playlist.isValid());
    BOOST_CHECK(m_playlist.items().empty());
}

BOOST_FIXTURE_TEST_CASE(TestInvalidIndexSet, PlaylistFixture)
{
    std::vector<int> iter = {2, 1, 0};

    queueAlbum(42, "Album", {createFile(1, "1.mp3"), createFile(2, "2.mp3")});
    m_playlist.reset(zeppelin::player::QueueItem::FIRST);

    BOOST_CHECK(m_playlist.isValid());

    // try to set az invalid index
    BOOST_CHECK(!m_playlist.set(iter));

    BOOST_CHECK(m_playlist.isValid());

    iter.clear();
    m_playlist.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 0);
}
