#include <boost/test/unit_test.hpp>

#include <player/queue.h>

using player::Playlist;

BOOST_AUTO_TEST_CASE(TestPlaylist)
{
    player::Playlist p;
    std::vector<int> iter;

    BOOST_CHECK_EQUAL(p.type(), player::QueueItem::PLAYLIST);
    BOOST_CHECK(!p.isValid());
    BOOST_CHECK(!p.prev());
    BOOST_CHECK(!p.next());

    // queue 2 files and an album
    p.add(std::make_shared<library::File>(1, "tst", "a.mp3"));
    p.add(std::make_shared<library::Album>(57, "Album", 42, 0, 0), {
	std::make_shared<library::File>(3, "album", "1.mp3"),
	std::make_shared<library::File>(4, "album", "2.mp3")
    });
    p.add(std::make_shared<library::File>(2, "tst", "b.mp3"));

    // the playlist should be invalid still
    BOOST_CHECK(!p.isValid());

    // reset it to be in a known state
    p.reset(player::QueueItem::FIRST);

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
    p.set(iter);
    BOOST_CHECK(p.isValid());
    BOOST_CHECK_EQUAL(p.file()->m_name, "1.mp3");
}

BOOST_AUTO_TEST_CASE(TestFileDeletionBeforeActive)
{
    player::Playlist p;
    std::vector<int> iter;

    p.add(std::make_shared<library::File>(1, "tst", "a.mp3"));
    p.add(std::make_shared<library::File>(2, "tst", "b.mp3"));
    p.add(std::make_shared<library::File>(3, "tst", "c.mp3"));

    // go to the second song
    p.reset(player::QueueItem::FIRST);
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
    player::Playlist p;
    std::vector<int> iter;

    p.add(std::make_shared<library::Album>(42, "Album", 42, 0, 0), {
	std::make_shared<library::File>(1, "album", "1.mp3"),
	std::make_shared<library::File>(2, "album", "2.mp3"),
	std::make_shared<library::File>(3, "album", "3.mp3")
    });

    // go to the second song of the album
    p.reset(player::QueueItem::FIRST);
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
    player::Playlist p;
    std::vector<int> iter;

    p.add(std::make_shared<library::Album>(42, "Album", 42, 0, 0), {
	std::make_shared<library::File>(1, "album", "1.mp3"),
	std::make_shared<library::File>(2, "album", "2.mp3")
    });
    p.add(std::make_shared<library::Album>(57, "Album2", 57, 0, 0), {
	std::make_shared<library::File>(3, "album", "a.mp3"),
	std::make_shared<library::File>(4, "album", "b.mp3")
    });

    // go to the second song of the first album
    p.reset(player::QueueItem::FIRST);
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
    player::Playlist p;
    std::vector<int> iter;

    p.add(std::make_shared<library::File>(1, "tst", "a.mp3"));

    BOOST_CHECK(!p.isValid());
    p.reset(player::QueueItem::FIRST);
    BOOST_CHECK(p.isValid());

    iter.push_back(0);
    p.remove(iter);
    BOOST_CHECK(!p.isValid());

    // queue should be still invalid after adding a new item
    p.add(std::make_shared<library::File>(2, "tst", "b.mp3"));

    BOOST_CHECK(!p.isValid());
}

BOOST_AUTO_TEST_CASE(TestRemovalOfEmptyAlbum)
{
    player::Playlist p;
    std::vector<int> iter = {0, 0};

    p.add(std::make_shared<library::Album>(42, "Album", 42, 0, 0), {
	std::make_shared<library::File>(1, "album", "1.mp3")
    });
    p.reset(player::QueueItem::FIRST);

    BOOST_CHECK(p.isValid());

    // remove the only file from the album, the album should be removed automatically because it is empty
    p.remove(iter);

    BOOST_CHECK(!p.isValid());
    BOOST_CHECK(p.items().empty());
}
