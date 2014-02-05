#include <boost/test/unit_test.hpp>

#define private public

#include <output/baseoutput.h>
#include <player/controller.h>
#include <codec/codecmanager.h>

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

	codec::Metadata readMetadata() override
	{
	    codec::Metadata m;
	    return m;
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
    void positionChanged() override
    { m_events.push_back("position-changed"); }
    void songChanged() override
    { m_events.push_back("song-changed"); }
    void queueChanged() override
    { m_events.push_back("queue-changed"); }
    void volumeChanged() override
    { m_events.push_back("volume-changed"); }

    std::vector<std::string>& m_events;
};

struct ControllerFixture
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

    std::shared_ptr<zeppelin::library::File> createFile(int id, const std::string& name)
    {
	std::shared_ptr<zeppelin::library::File> file(new zeppelin::library::File(id));
	file->m_name = name;
	return file;
    }

    void queueFile(int id, const std::string& name)
    {
	m_ctrl->queue(std::make_shared<zeppelin::player::File>(createFile(id, name)));
    }

    void queueAlbum(int id, const std::string& name, const std::vector<std::shared_ptr<zeppelin::library::File>>& files)
    {
	m_ctrl->queue(
	    std::make_shared<zeppelin::player::Album>(
		std::make_shared<zeppelin::library::Album>(id, name, 0, 0),
		files));
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
    queueFile(42, "hello.mp3");

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
    BOOST_CHECK_EQUAL(m_events[1], "song-changed");
    BOOST_CHECK_EQUAL(m_events[2], "started");
}

BOOST_FIXTURE_TEST_CASE(playback_not_started_for_unknown_codec, ControllerFixture)
{
    // make the codec of the  input file invalid
    m_codecManager.m_codecValid = false;

    queueFile(42, "hello.mp3");
    m_ctrl->play();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());

    BOOST_REQUIRE_EQUAL(m_events.size(), 2);
    BOOST_CHECK_EQUAL(m_events[1], "song-changed");
}

BOOST_FIXTURE_TEST_CASE(playback_not_started_for_unopenable_file, ControllerFixture)
{
    // make the open() function of the codec fail
    m_codecManager.m_openShouldFail = true;

    queueFile(42, "hello.mp3");
    m_ctrl->play();
    process();

    BOOST_CHECK_EQUAL(m_ctrl->m_state, Controller::STOPPED);
    BOOST_CHECK(m_decoder->m_cmds.empty());
    BOOST_CHECK(m_player->m_cmds.empty());

    BOOST_REQUIRE_EQUAL(m_events.size(), 2);
    BOOST_CHECK_EQUAL(m_events[1], "song-changed");
}

BOOST_FIXTURE_TEST_CASE(playback_paused, ControllerFixture)
{
    queueFile(42, "hello.mp3");
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
    queueFile(42, "hello.mp3");
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

BOOST_FIXTURE_TEST_CASE(queue_updated_at_decoder_and_song_finished, ControllerFixture)
{
    // I hope you like these :)
    queueAlbum(1, "Led Zeppelin", {createFile(2, "good_times_bad_times.mp3"), createFile(3, "ramble_on.mp3")});
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

    // player finished on the first track
    m_ctrl->command(player::ControllerImpl::SONG_FINISHED);
    process();

    // check status - it should be at the second track now
    s = m_ctrl->getStatus();
    BOOST_REQUIRE_EQUAL(s.m_index.size(), 2);
    BOOST_CHECK_EQUAL(s.m_index[0], 0);
    BOOST_CHECK_EQUAL(s.m_index[1], 1);

    // make sure a song-changed event is generated now
    BOOST_REQUIRE_EQUAL(m_events.size(), 4);
    BOOST_CHECK_EQUAL(m_events[3], "song-changed");
}
