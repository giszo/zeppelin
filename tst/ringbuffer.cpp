#include <boost/test/unit_test.hpp>

#include <buffer/ringbuffer.h>

BOOST_AUTO_TEST_CASE(TestRingBuffer)
{
    buffer::RingBuffer b(3);

    BOOST_CHECK_EQUAL(b.getAvailableSize(), 0);
    BOOST_CHECK_EQUAL(b.getFreeSize(), 3);

    char c = 'a';
    b.write(&c, 1);

    BOOST_CHECK_EQUAL(b.getAvailableSize(), 1);
    BOOST_CHECK_EQUAL(b.getFreeSize(), 2);

    const char* d = "bc";
    b.write(d, 2);

    BOOST_CHECK_EQUAL(b.getAvailableSize(), 3);
    BOOST_CHECK_EQUAL(b.getFreeSize(), 0);

    char t1[2];
    b.read(t1, 2);

    BOOST_CHECK_EQUAL(b.getAvailableSize(), 1);
    BOOST_CHECK_EQUAL(b.getFreeSize(), 2);
    BOOST_CHECK_EQUAL(t1[0], 'a');
    BOOST_CHECK_EQUAL(t1[1], 'b');

    char t2;
    b.read(&t2, 1);

    BOOST_CHECK_EQUAL(b.getAvailableSize(), 0);
    BOOST_CHECK_EQUAL(b.getFreeSize(), 3);
    BOOST_CHECK_EQUAL(t2, 'c');
}
