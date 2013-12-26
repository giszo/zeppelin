#include <boost/test/unit_test.hpp>

#include <player/fifo.h>

using player::Fifo;

static bool s_notified = false;

static void notifyCallback()
{
    s_notified = true;
}

BOOST_AUTO_TEST_CASE(TestFifo)
{
    Fifo fifo(3);
    fifo.setNotifyCallback(3, std::bind(&notifyCallback));

    int16_t tst1[] = {1, 2};
    int16_t tst2[] = {3};

    // empty fifo
    BOOST_CHECK_EQUAL(fifo.getNextEvent(), Fifo::NONE);
    BOOST_CHECK_EQUAL(fifo.getBytes(), 0);

    // put some sample into the fifo
    fifo.addSamples(tst1, sizeof(tst1));
    BOOST_CHECK_EQUAL(fifo.getBytes(), 4);

    // add a marker
    fifo.addMarker();

    // put some more samples into the fifo
    fifo.addSamples(tst2, sizeof(tst2));
    BOOST_CHECK_EQUAL(fifo.getBytes(), 6);

    // get the first event
    BOOST_REQUIRE_EQUAL(fifo.getNextEvent(), Fifo::SAMPLES);
    int16_t tst3;
    BOOST_REQUIRE_EQUAL(fifo.readSamples(&tst3, sizeof(tst3)), sizeof(tst3));
    BOOST_CHECK_EQUAL(tst3, 1);
    BOOST_CHECK_EQUAL(fifo.getBytes(), 4);

    // get the second part of the first chunk of samples
    BOOST_REQUIRE_EQUAL(fifo.getNextEvent(), Fifo::SAMPLES);
    int16_t tst4;
    BOOST_REQUIRE_EQUAL(fifo.readSamples(&tst4, sizeof(tst4)), sizeof(tst4));
    BOOST_CHECK_EQUAL(tst4, 2);
    BOOST_CHECK_EQUAL(fifo.getBytes(), 2);
    BOOST_CHECK(s_notified);

    // try to read samples again while we should have a marker at the front of the fifo
    int16_t tst5;
    BOOST_CHECK_EQUAL(fifo.readSamples(&tst5, sizeof(tst5)), 0);
    BOOST_CHECK_EQUAL(fifo.getBytes(), 2);

    // make sure we have that marker now
    BOOST_REQUIRE_EQUAL(fifo.getNextEvent(), Fifo::MARKER);

    // now we can read the last chuck of samples
    BOOST_REQUIRE_EQUAL(fifo.getNextEvent(), Fifo::SAMPLES);
    int16_t tst6;
    BOOST_REQUIRE_EQUAL(fifo.readSamples(&tst6, sizeof(tst6)), sizeof(tst6));
    BOOST_CHECK_EQUAL(tst6, 3);
    BOOST_CHECK_EQUAL(fifo.getBytes(), 0);

    // the fifo should have no more events now
    BOOST_CHECK_EQUAL(fifo.getNextEvent(), Fifo::NONE);
}
