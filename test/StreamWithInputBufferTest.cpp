// StreamUtils - github.com/bblanchon/StreamUtils
// Copyright Benoit Blanchon 2019
// MIT License

#include "FailingAllocator.hpp"
#include "Stream.hpp"
#include "StreamSpy.hpp"
#include "StreamStub.hpp"

#include "StreamUtils/StreamWithInputBuffer.hpp"

#include "doctest.h"

#include <sstream>
#include <string>

using namespace StreamUtils;

TEST_CASE("StreamWithInputBuffer") {
  StreamStub stub;
  StreamSpy spy{stub};

  SUBCASE("capacity = 4") {
    auto bufferedStream = bufferizeInput(spy, 4);
    Stream& stream = bufferedStream;

    SUBCASE("available()") {
      stub.setup("ABCDEFGH");

      SUBCASE("empty input") {
        stub.setup("");
        CHECK(stream.available() == 0);
        CHECK(spy.log() == "available() -> 0");
      }

      SUBCASE("read empty input") {
        stub.setup("");

        stream.read();

        CHECK(stream.available() == 0);
        CHECK(spy.log() ==
              "readBytes(4) -> 0"
              "available() -> 0");
      }

      SUBCASE("same a upstream") {
        CHECK(stream.available() == 8);
        CHECK(spy.log() == "available() -> 8");
      }

      SUBCASE("upstream + in buffer") {
        stream.read();

        CHECK(stream.available() == 7);
        CHECK(spy.log() ==
              "readBytes(4) -> 4"
              "available() -> 4");
      }
    }

    SUBCASE("peek()") {
      SUBCASE("returns -1 when empty") {
        stub.setup("");

        int result = stream.peek();

        CHECK(result == -1);
        CHECK(spy.log() == "peek() -> -1");
      }

      SUBCASE("doesn't call readBytes() when buffer is empty") {
        stub.setup("A");

        int result = stream.peek();

        CHECK(result == 'A');
        CHECK(spy.log() == "peek() -> 65");
      }

      SUBCASE("doesn't call peek() when buffer is full") {
        stub.setup("AB");

        stream.read();
        int result = stream.peek();

        CHECK(result == 'B');
        CHECK(spy.log() == "readBytes(4) -> 2");
      }
    }

    SUBCASE("read()") {
      SUBCASE("reads 4 bytes at a time") {
        stub.setup("ABCDEFG");
        std::string result;

        for (int i = 0; i < 7; i++) {
          result += (char)stream.read();
        }

        CHECK(result == "ABCDEFG");
        CHECK(spy.log() ==
              "readBytes(4) -> 4"
              "readBytes(4) -> 3");
      }

      SUBCASE("returns -1 when empty") {
        stub.setup("");

        int result = stream.read();

        CHECK(result == -1);
        CHECK(spy.log() == "readBytes(4) -> 0");
      }
    }

    SUBCASE("readBytes()") {
      SUBCASE("empty input") {
        stub.setup("");

        char c;
        size_t result = stream.readBytes(&c, 1);

        CHECK(result == 0);
        CHECK(spy.log() == "readBytes(4) -> 0");
      }

      SUBCASE("reads 4 bytes when requested one") {
        stub.setup("ABCDEFG");

        char c;
        size_t result = stream.readBytes(&c, 1);

        CHECK(c == 'A');
        CHECK(result == 1);
        CHECK(spy.log() == "readBytes(4) -> 4");
      }

      SUBCASE("copy one byte from buffer") {
        stub.setup("ABCDEFGH");
        stream.read();  // load buffer

        char c;
        size_t result = stream.readBytes(&c, 1);

        CHECK(c == 'B');
        CHECK(result == 1);
        CHECK(spy.log() == "readBytes(4) -> 4");
      }

      SUBCASE("copy content from buffer then bypass buffer") {
        stub.setup("ABCDEFGH");
        stream.read();  // load buffer

        char c[8] = {0};
        size_t result = stream.readBytes(c, 7);

        CHECK(c == std::string("BCDEFGH"));
        CHECK(result == 7);
        CHECK(spy.log() ==
              "readBytes(4) -> 4"
              "readBytes(4) -> 4");
      }

      SUBCASE("copy content from buffer twice") {
        stub.setup("ABCDEFGH");
        stream.read();  // load buffer

        char c[8] = {0};
        size_t result = stream.readBytes(c, 4);

        CHECK(c == std::string("BCDE"));
        CHECK(result == 4);
        CHECK(spy.log() ==
              "readBytes(4) -> 4"
              "readBytes(4) -> 4");
      }

      SUBCASE("read past the end") {
        stub.setup("A");

        char c;
        stream.readBytes(&c, 1);
        size_t result = stream.readBytes(&c, 1);

        CHECK(result == 0);
        CHECK(spy.log() ==
              "readBytes(4) -> 1"
              "readBytes(4) -> 0");
      }
    }

    SUBCASE("flush()") {
      stream.flush();
      CHECK(spy.log() == "flush()");
    }

    SUBCASE("copy constructor") {
      stub.setup("ABCDEFGH");
      bufferedStream.read();

      auto dup = bufferedStream;

      int result = dup.read();

      CHECK(result == 'B');
      CHECK(spy.log() == "readBytes(4) -> 4");
    }
  }

  SUBCASE("No memory") {
    BasicStreamWithInputBuffer<FailingAllocator> stream(spy, 4);

    SUBCASE("available()") {
      stub.setup("ABC");

      CHECK(stream.available() == 3);
    }

    SUBCASE("capacity()") {
      CHECK(stream.capacity() == 0);
    }

    SUBCASE("peek()") {
      stub.setup("ABC");

      int c = stream.peek();

      CHECK(c == 'A');
      CHECK(spy.log() == "peek() -> 65");
    }

    SUBCASE("read()") {
      stub.setup("ABC");

      int c = stream.read();

      CHECK(c == 'A');
      CHECK(spy.log() == "read() -> 65");
    }

    SUBCASE("readBytes()") {
      stub.setup("ABC");

      char s[4] = {0};
      int n = stream.readBytes(s, 3);

      CHECK(n == 3);
      CHECK(s == std::string("ABC"));
      CHECK(spy.log() == "readBytes(3) -> 3");
    }
  }

  SUBCASE("Real example") {
    auto bufferedStream = bufferizeInput(spy, 64);
    Stream& stream = bufferedStream;

    stub.setup("{\"helloWorld\":\"Hello World\"}");

    char c[] = "ABCDEFGH";
    CHECK(stream.readBytes(&c[0], 1) == 1);
    CHECK(stream.readBytes(&c[1], 1) == 1);
    CHECK(stream.readBytes(&c[2], 1) == 1);
    CHECK(stream.readBytes(&c[3], 1) == 1);

    CHECK(c == std::string("{\"heEFGH"));
    CHECK(spy.log() == "readBytes(64) -> 28");
  }
}