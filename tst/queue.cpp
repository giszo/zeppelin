#include <boost/test/unit_test.hpp>

#include <zeppelin/player/queue.h>

using zeppelin::player::Playlist;

static std::shared_ptr<zeppelin::library::File> createFile(int id, const std::string& name)
{
    std::shared_ptr<zeppelin::library::File> file = std::make_shared<zeppelin::library::File>(id);
    file->m_name = name;
    return file;
}

BOOST_AUTO_TEST_CASE(TestPlaylist)
{
    Playlist p;
    std::vector<int> iter;

    BOOST_CHECK_EQUAL(p.type(), zeppelin::player::QueueItem::PLAYLIST);
    BOOST_CHECK(!p.isValid());
    BOOST_CHECK(!p.prev());
    BOOST_CHECK(!p.next());

    // queue 2 files and an album
    p.add(createFile(1, "a.mp3"));
    p.add(std::make_shared<zeppelin::library::Album>(57, "Album", 42, 0, 0), {
	createFile(3, "1.mp3"),
	createFile(4, "2.mp3")
    });
    p.add(createFile(2, "b.mp3"));

    // the playlist should be invalid still
    BOOST_CHECK(!p.isValid());

    // reset it to be in a known state
    p.reset(zeppelin::player::QueueItem::FIRST);

    // check the first item
    BOOST_CHECK(p.isValid());
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(p.file()->m_name, "a.mp3");

    // make sure prev() does not work now
    BOOST_CHECK(!p.prev());
    BOOST_CHECK(p.isValid());

    // step to the next item
    BOOST_CHECK(p.next());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(p.file()->m_name, "1.mp3");

    // ... next again
    BOOST_CHECK(p.next());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 1);
    BOOST_CHECK_EQUAL(p.file()->m_name, "2.mp3");

    // ... next
    BOOST_CHECK(p.next());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 2);
    BOOST_CHECK_EQUAL(p.file()->m_name, "b.mp3");

    // make sure next() does not work now
    BOOST_CHECK(!p.next());
    BOOST_CHECK(p.isValid());

    // step to the previous item
    BOOST_CHECK(p.prev());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 1);
    BOOST_CHECK_EQUAL(p.file()->m_name, "2.mp3");

    // prev
    BOOST_CHECK(p.prev());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(p.file()->m_name, "1.mp3");

    // ... prev
    BOOST_CHECK(p.prev());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(p.file()->m_name, "a.mp3");

    // make sure prev is still not working at the beginnig
    BOOST_CHECK(!p.prev());

    // test index setting
    iter.clear();
    iter.push_back(1);
    iter.push_back(0);
    BOOST_CHECK(p.set(iter));
    BOOST_CHECK(p.isValid());
    BOOST_CHECK_EQUAL(p.file()->m_name, "1.mp3");
}

BOOST_AUTO_TEST_CASE(TestFileDeletionBeforeActive)
{
    Playlist p;
    std::vector<int> iter;

    p.add(createFile(1, "a.mp3"));
    p.add(createFile(2, "b.mp3"));
    p.add(createFile(3, "c.mp3"));

    // go to the second song
    p.reset(zeppelin::player::QueueItem::FIRST);
    p.next();

    // validate the iterator
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 1);

    // remove the first song
    iter.clear();
    iter.push_back(0);
    p.remove(iter);

    // validate the iterator again
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(p.file()->m_name, "b.mp3");
}

BOOST_AUTO_TEST_CASE(TestActiveFileDeletionInsideAlbum)
{
    Playlist p;
    std::vector<int> iter;

    p.add(std::make_shared<zeppelin::library::Album>(42, "Album", 42, 0, 0), {
	createFile(1, "1.mp3"),
	createFile(2, "2.mp3"),
	createFile(3, "3.mp3")
    });

    // go to the second song of the album
    p.reset(zeppelin::player::QueueItem::FIRST);
    p.next();

    // validate the iterator
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 1);

    // remove the current file
    p.remove(iter);

    // validate the iterator now
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 1);
    BOOST_CHECK_EQUAL(p.file()->m_name, "3.mp3");
}

BOOST_AUTO_TEST_CASE(TestActiveFileDeletionAtTheEndOfAlbum)
{
    Playlist p;
    std::vector<int> iter;

    p.add(std::make_shared<zeppelin::library::Album>(42, "Album", 42, 0, 0), {
	createFile(1, "1.mp3"),
	createFile(2, "2.mp3")
    });
    p.add(std::make_shared<zeppelin::library::Album>(57, "Album2", 57, 0, 0), {
	createFile(3, "a.mp3"),
	createFile(4, "b.mp3")
    });

    // go to the second song of the first album
    p.reset(zeppelin::player::QueueItem::FIRST);
    p.next();

    // validate the iterator
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 1);

    // remove the current file
    p.remove(iter);

    // validate the iterator now
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 1);
    BOOST_CHECK_EQUAL(iter[1], 0);
    BOOST_CHECK_EQUAL(p.file()->m_name, "a.mp3");
}

BOOST_AUTO_TEST_CASE(TestInvalidationAfterDeleteActive)
{
    Playlist p;
    std::vector<int> iter;

    p.add(createFile(1, "a.mp3"));

    BOOST_CHECK(!p.isValid());
    p.reset(zeppelin::player::QueueItem::FIRST);
    BOOST_CHECK(p.isValid());

    iter.push_back(0);
    p.remove(iter);
    BOOST_CHECK(!p.isValid());

    // queue should be still invalid after adding a new item
    p.add(createFile(2, "b.mp3"));

    BOOST_CHECK(!p.isValid());
}

BOOST_AUTO_TEST_CASE(TestRemovalOfEmptyAlbum)
{
    Playlist p;
    std::vector<int> iter = {0, 0};

    p.add(std::make_shared<zeppelin::library::Album>(42, "Album", 42, 0, 0), {
	createFile(1, "1.mp3")
    });
    p.reset(zeppelin::player::QueueItem::FIRST);

    BOOST_CHECK(p.isValid());

    // remove the only file from the album, the album should be removed automatically because it is empty
    p.remove(iter);

    BOOST_CHECK(!p.isValid());
    BOOST_CHECK(p.items().empty());
}

BOOST_AUTO_TEST_CASE(TestInvalidIndexSet)
{
    Playlist p;
    std::vector<int> iter = {2, 1, 0};

    p.add(std::make_shared<zeppelin::library::Album>(42, "Album", 42, 0, 0), {
	createFile(1, "1.mp3"),
	createFile(2, "2.mp3")
    });
    p.reset(zeppelin::player::QueueItem::FIRST);

    BOOST_CHECK(p.isValid());

    // try to set az invalid index
    BOOST_CHECK(!p.set(iter));

    BOOST_CHECK(p.isValid());

    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 2);
    BOOST_CHECK_EQUAL(iter[0], 0);
    BOOST_CHECK_EQUAL(iter[1], 0);
}
