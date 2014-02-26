/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <boost/test/unit_test.hpp>

#include "librarybuilder.h"

#define private public

#include <output/baseoutput.h>
#include <player/controller.h>
#include <codec/codecmanager.h>
#include <utils/makestring.h>

using zeppelin::player::Controller;

class FakeOutput : public output::BaseOutput
{
    public:
	FakeOutput(const config::Config& config)
	    : BaseOutput(config, "fake")
	{}

	player::Format getFormat() const override
	{ return player::Format(44100, 2); }

	int getFreeSize() override
	{ return 0; }

	void setup(int rate, int channels) override
	{}

	void prepare() override
	{}

	void drop() override
	{}

	void write(const float* samples, size_t count) override
	{}
};

class FakeDecoder : public player::Decoder
{
    public:
	FakeDecoder(player::Fifo& fifo,
		    const config::Config& config)
	    : Decoder(1024, player::Format(44100, 2), fifo, config)
	{}

	void setInput(const std::shared_ptr<codec::BaseCodec>& input) override
	{
	    if (input)
		m_cmds.push_back("input file");
	    else
		m_cmds.push_back("input null");
	}

	void startDecoding() override
	{ m_cmds.push_back("start"); }
	void stopDecoding() override
	{ m_cmds.push_back("stop"); }

	void seek(off_t seconds) override
	{ m_cmds.push_back("seek"); }
	void notify() override
	{ m_cmds.push_back("notify"); }

	std::vector<std::string> m_cmds;
};

class FakePlayer : public player::Player
{
    public:
	FakePlayer(const std::shared_ptr<output::BaseOutput>& output,
		   player::Fifo& fifo,
		   const config::Config& config)
	    : Player(output, fifo, config)
	{}

	void startPlayback() override
	{ m_cmds.push_back("start"); }
	void pausePlayback() override
	{ m_cmds.push_back("pause"); }
	void stopPlayback() override
	{ m_cmds.push_back("stop"); }
	void seek(off_t seconds) override
	{ m_cmds.push_back("seek"); }

	std::vector<std::string> m_cmds;
};

class FakeCodec : public codec::BaseCodec
{
    public:
	FakeCodec(const std::string& file, bool openShouldFail)
	    : BaseCodec(file),
	      m_openShouldFail(openShouldFail)
	{}

	void open() override
	{
	    if (m_openShouldFail)
		throw codec::CodecException("unable to open");
	}

	player::Format getFormat() const override
	{ return player::Format(44100, 2); }

	bool decode(float*& samples, size_t& count) override
	{ return false; }

	void seek(off_t sample) override
	{}

	std::unique_ptr<zeppelin::library::Metadata> readMetadata() override
	{
	    return nullptr;
	}

	bool m_openShouldFail;
};

class FakeCodecManager : public codec::CodecManager
{
    public:
	FakeCodecManager()
	    : m_codecValid(true),
	      m_openShouldFail(false)
	{}

	std::shared_ptr<codec::BaseCodec> create(const std::string& file) const override
	{
	    if (!m_codecValid)
		return nullptr;

	    return std::make_shared<FakeCodec>(file, m_openShouldFail);
	}

	bool m_codecValid;
	bool m_openShouldFail;
};

struct EventListener : public zeppelin::player::EventListener
{
    EventListener(std::vector<std::string>& events)
	: m_events(events)
    {}

    // event listener implenentation
    void started() override
    { m_events.push_back("started"); }
    void paused() override
    { m_events.push_back("paused"); }
    void stopped() override
    { m_events.push_back("stopped"); }
    void positionChanged(unsigned pos) override
    { m_events.push_back(utils::MakeString() << "position-changed " << pos); }
    void songChanged(const std::vector<int>& idx) override
    {
	std::ostringstream ss;
	for (int i : idx)
	    ss << "," << i;
	std::string idxstr = ss.str().empty() ? "" : ss.str().substr(1);
	m_events.push_back(utils::MakeString() << "song-changed " << idxstr);
    }
    void queueChanged() override
    { m_events.push_back("queue-changed"); }
    void volumeChanged(int level) override
    { m_events.push_back(utils::MakeString() << "volume-changed " << level); }

    std::vector<std::string>& m_events;
};

struct ControllerFixture : public LibraryBuilder
{
    ControllerFixture()
	: m_fifo(1024),
	  m_output(new FakeOutput(m_config)),
	  m_decoder(new FakeDecoder(m_fifo, m_config)),
	  m_player(new FakePlayer(m_output, m_fifo, m_config)),
	  m_ctrl(player::ControllerImpl::create(m_codecManager, m_decoder, m_player, m_config))
    {
	m_ctrl->addListener(std::make_shared<EventListener>(m_events));
    }

    void queueFile(const std::shared_ptr<zeppelin::library::File>& file)
    {
	m_ctrl->queue(std::make_shared<zeppelin::player::File>(file));
    }

    void queueAlbum(const std::shared_ptr<zeppelin::library::Album>& album,
		    const std::vector<std::shared_ptr<zeppelin::library::File>>& files)
    {
	m_ctrl->queue(std::make_shared<zeppelin::player::Album>(album,files));
    }

    void startPlayback()
    {
	m_ctrl->play();
	process();
	m_decoder->m_cmds.clear();
	m_player->m_cmds.clear();
    }

    void process()
    { m_ctrl->processCommands(); }

    config::Config m_config;

    player::Fifo m_fifo;

    std::shared_ptr<FakeOutput> m_output;
    std::shared_ptr<FakeDecoder> m_decoder;
    std::shared_ptr<FakePlayer> m_player;

    FakeCodecManager m_codecManager;

    std::shared_ptr<player::ControllerImpl> m_ctrl;

    std::vector<std::string> m_events;
};

BOOST_FIXTURE_TEST_CASE(check_initial_state, ControllerFixture)
{
    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
}

BOOST_FIXTURE_TEST_CASE(playback_not_started_without_queue, ControllerFixture)
{
    m_ctrl->play();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());
    BOOST_CHECK(m_events.empty());
}

BOOST_FIXTURE_TEST_CASE(pause_not_working_in_stopped_state, ControllerFixture)
{
    m_ctrl->pause();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());
    BOOST_CHECK(m_events.empty());
}

BOOST_FIXTURE_TEST_CASE(seek_not_working_in_stopped_state, ControllerFixture)
{
    m_ctrl->seek(3);
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());
    BOOST_CHECK(m_events.empty());
}

BOOST_FIXTURE_TEST_CASE(stop_not_working_in_stopped_state, ControllerFixture)
{
    m_ctrl->stop();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());
    BOOST_CHECK(m_events.empty());
}

BOOST_FIXTURE_TEST_CASE(playback_started_first_time, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));

    BOOST_REQUIRE_EQUAL(m_events.size(), 1);
    BOOST_CHECK_EQUAL(m_events[0], "queue-changed");

    m_ctrl->play();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 2);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "input file");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "start");
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "start");

    BOOST_REQUIRE_EQUAL(m_events.size(), 3);
    BOOST_CHECK_EQUAL(m_events[1], "song-changed 0");
    BOOST_CHECK_EQUAL(m_events[2], "started");
}

BOOST_FIXTURE_TEST_CASE(playback_not_started_for_unknown_codec, ControllerFixture)
{
    // make the codec of the  input file invalid
    m_codecManager.m_codecValid = false;

    queueFile(createFile(42, "hello.mp3"));
    m_ctrl->play();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());

    BOOST_REQUIRE_EQUAL(m_events.size(), 2);
    BOOST_CHECK_EQUAL(m_events[1], "song-changed 0");
}

BOOST_FIXTURE_TEST_CASE(playback_not_started_for_unopenable_file, ControllerFixture)
{
    // make the open() function of the codec fail
    m_codecManager.m_openShouldFail = true;

    queueFile(createFile(42, "hello.mp3"));
    m_ctrl->play();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());

    BOOST_REQUIRE_EQUAL(m_events.size(), 2);
    BOOST_CHECK_EQUAL(m_events[1], "song-changed 0");
}

BOOST_FIXTURE_TEST_CASE(playback_paused, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    startPlayback();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    m_ctrl->pause();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::PAUSED);

    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "pause");

    BOOST_REQUIRE_EQUAL(m_events.size(), 4);
    BOOST_CHECK_EQUAL(m_events[3], "paused");
}

BOOST_FIXTURE_TEST_CASE(playback_stopped, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    startPlayback();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    m_ctrl->stop();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 2);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "stop");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "input null");
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "stop");

    BOOST_REQUIRE_EQUAL(m_events.size(), 4);
    BOOST_CHECK_EQUAL(m_events[3], "stopped");
}

BOOST_FIXTURE_TEST_CASE(remove_non_current, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    queueFile(createFile(57, "world.mp3"));
    startPlayback();
    m_events.clear();

    m_ctrl->remove({1});
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    // decoder and player should not be touched
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());

    // look for queue changed event
    BOOST_REQUIRE_EQUAL(m_events.size(), 1);
    BOOST_CHECK_EQUAL(m_events[0], "queue-changed");
}

BOOST_FIXTURE_TEST_CASE(remove_current_while_paused, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    queueFile(createFile(57, "world.mp3"));
    m_ctrl->play();
    m_ctrl->pause();
    process();

    m_decoder->m_cmds.clear();
    m_player->m_cmds.clear();
    m_events.clear();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PAUSED);

    m_ctrl->remove({0});
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    // check the decoder
    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 3);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "stop");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "input null");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[2], "input file");
    // check the player
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "stop");

    // check generated events
    BOOST_REQUIRE_EQUAL(m_events.size(), 3);
    BOOST_CHECK_EQUAL(m_events[0], "stopped");
    BOOST_CHECK_EQUAL(m_events[1], "queue-changed");
    BOOST_CHECK_EQUAL(m_events[2], "song-changed 0");
}

BOOST_FIXTURE_TEST_CASE(remove_current_while_playing, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    queueFile(createFile(57, "world.mp3"));
    startPlayback();
    m_events.clear();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    m_ctrl->remove({0});
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    // check the decoder
    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 4);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "stop");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "input null");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[2], "input file");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[3], "start");
    // check the player
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 2);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "stop");
    BOOST_CHECK_EQUAL(m_player->m_cmds[1], "start");

    // check generated events
    BOOST_REQUIRE_EQUAL(m_events.size(), 4);
    BOOST_CHECK_EQUAL(m_events[0], "stopped");
    BOOST_CHECK_EQUAL(m_events[1], "queue-changed");
    BOOST_CHECK_EQUAL(m_events[2], "song-changed 0");
    BOOST_CHECK_EQUAL(m_events[3], "started");
}

BOOST_FIXTURE_TEST_CASE(remove_last_while_playing, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    startPlayback();
    m_events.clear();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    m_ctrl->remove({0});
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    // check the decoder
    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 2);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "stop");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "input null");
    // check the player
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "stop");

    // check generated events
    BOOST_REQUIRE_EQUAL(m_events.size(), 3);
    BOOST_CHECK_EQUAL(m_events[0], "stopped");
    BOOST_CHECK_EQUAL(m_events[1], "queue-changed");
    BOOST_CHECK_EQUAL(m_events[2], "song-changed ");
}

BOOST_FIXTURE_TEST_CASE(remove_all_while_playing, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    startPlayback();
    m_events.clear();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    m_ctrl->removeAll();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    // check decoder commands
    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 2);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "stop");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "input null");
    BOOST_REQUIRE_EQUAL(m_player->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_player->m_cmds[0], "stop");

    BOOST_REQUIRE_EQUAL(m_events.size(), 3);
    BOOST_CHECK_EQUAL(m_events[0], "stopped");
    BOOST_CHECK_EQUAL(m_events[1], "queue-changed");
    BOOST_CHECK_EQUAL(m_events[2], "song-changed ");
}

BOOST_FIXTURE_TEST_CASE(remove_all_while_stopped, ControllerFixture)
{
    queueFile(createFile(42, "hello.mp3"));
    // issue play and stop to make the queue index valid
    m_ctrl->play();
    m_ctrl->stop();
    process();
    m_events.clear();
    m_decoder->m_cmds.clear();
    m_player->m_cmds.clear();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    m_ctrl->removeAll();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    // make sure decoder and player were not touched
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());

    // check events
    BOOST_REQUIRE_EQUAL(m_events.size(), 2);
    BOOST_CHECK_EQUAL(m_events[0], "queue-changed");
    BOOST_CHECK_EQUAL(m_events[1], "song-changed ");
}

BOOST_FIXTURE_TEST_CASE(queue_updated_at_decoder_and_song_finished, ControllerFixture)
{
    // I hope you like these :)
    queueAlbum(createAlbum(1, "Led Zeppelin"),
	       {createFile(2, "good_times_bad_times.mp3"), createFile(3, "ramble_on.mp3")});
    startPlayback();

    BOOST_REQUIRE_EQUAL(m_ctrl->m_state, Controller::PLAYING);

    // check status
    auto s = m_ctrl->getStatus();
    BOOST_REQUIRE_EQUAL(s.m_index.size(), 2);
    BOOST_CHECK_EQUAL(s.m_index[0], 0);
    BOOST_CHECK_EQUAL(s.m_index[1], 0);

    // decoder finished on the first track
    m_ctrl->command(player::ControllerImpl::DECODER_FINISHED);
    process();

    // check status - it should be the same as before
    s = m_ctrl->getStatus();
    BOOST_REQUIRE_EQUAL(s.m_index.size(), 2);
    BOOST_CHECK_EQUAL(s.m_index[0], 0);
    BOOST_CHECK_EQUAL(s.m_index[1], 0);

    // make sure this generated no new event (current: queue-changed, song-changed, started)
    BOOST_CHECK_EQUAL(m_events.size(), 3);
    m_events.clear();

    // the second file should be loaded into the decoder
    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 2);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "input file");
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[1], "start");
    m_decoder->m_cmds.clear();

    // player finished on the first track
    m_ctrl->command(player::ControllerImpl::SONG_FINISHED);
    process();

    // check status - it should be at the second track now
    s = m_ctrl->getStatus();
    BOOST_REQUIRE_EQUAL(s.m_index.size(), 2);
    BOOST_CHECK_EQUAL(s.m_index[0], 0);
    BOOST_CHECK_EQUAL(s.m_index[1], 1);

    // make sure a song-changed event is generated now
    BOOST_REQUIRE_EQUAL(m_events.size(), 1);
    BOOST_CHECK_EQUAL(m_events[0], "song-changed 0,1");
    m_events.clear();

    // decoder finished on the second track - it should be invalidated now
    m_ctrl->command(player::ControllerImpl::DECODER_FINISHED);
    process();

    // make sure it is really invalidated
    BOOST_REQUIRE_EQUAL(m_decoder->m_cmds.size(), 1);
    BOOST_CHECK_EQUAL(m_decoder->m_cmds[0], "input null");

    // make sure no new event has been generated
    BOOST_CHECK(m_events.empty());

    // player finished the second track too
    m_ctrl->command(player::ControllerImpl::SONG_FINISHED);
    process();

    // player should receive no commands at this point
    BOOST_CHECK(m_player->m_cmds.empty());

    // playback should be stopped by now
    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);

    // check the generated stopped event
    BOOST_REQUIRE_EQUAL(m_events.size(), 1);
    BOOST_CHECK_EQUAL(m_events[0], "stopped");
}
