/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <boost/test/unit_test.hpp>

#include <player/format.h>

using player::Format;

BOOST_AUTO_TEST_CASE(TestFormat)
{
    player::Format fmt(44100, 2);

    BOOST_CHECK_EQUAL(fmt.sizeOfSamples(42), sizeof(float) * 42 * 2 /* channels */);
    BOOST_CHECK_EQUAL(fmt.sizeOfSeconds(57), sizeof(float) * 57 * 44100 /* sampling rate */ * 2 /* channels */);

    BOOST_CHECK_EQUAL(fmt.numOfSamples(12 * sizeof(float) * 2 /* channels */), 12);
    BOOST_CHECK_THROW(fmt.numOfSamples(1), player::FormatException);
}
