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

    // queue 2 files
    p.add(std::make_shared<library::File>(1, "tst", "a.mp3"));
    p.add(std::make_shared<library::File>(2, "tst", "b.mp3"));

    // the playlist should be invalid still
    BOOST_CHECK(!p.isValid());

    // reset it to be in a known state
    p.reset();

    // check the first item
    BOOST_CHECK(p.isValid());
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 0);
    auto f1 = p.file();
    BOOST_CHECK_EQUAL(f1->m_name, "a.mp3");

    // make sure prev() does not work now
    BOOST_CHECK(!p.prev());
    BOOST_CHECK(p.isValid());

    // step to the next item
    BOOST_CHECK(p.next());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 1);
    auto f2 = p.file();
    BOOST_CHECK_EQUAL(f2->m_name, "b.mp3");

    // make sure next() does not work now
    BOOST_CHECK(!p.next());
    BOOST_CHECK(p.isValid());

    // step to the previous (first) item
    BOOST_CHECK(p.prev());
    BOOST_CHECK(p.isValid());
    iter.clear();
    p.get(iter);
    BOOST_REQUIRE_EQUAL(iter.size(), 1);
    BOOST_CHECK_EQUAL(iter[0], 0);
}
