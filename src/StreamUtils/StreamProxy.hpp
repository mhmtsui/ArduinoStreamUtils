// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019
// MIT License

#pragma once

namespace StreamUtils {

template <typename ReadPolicy, typename WritePolicy>
class StreamProxy : public Stream {
 public:
  explicit StreamProxy(Stream &upstream, ReadPolicy reader, WritePolicy writer)
      : _upstream(upstream), _reader(reader), _writer(writer) {}

  StreamProxy(const StreamProxy &other)
      : _upstream(other._upstream),
        _reader(other._reader),
        _writer(other._writer) {}

  ~StreamProxy() {
    _writer.detach(_upstream);
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    return _writer.write(_upstream, buffer, size);
  }

  size_t write(uint8_t data) override {
    return _writer.write(_upstream, data);
  }

  using Stream::write;

  int available() override {
    return _reader.available(_upstream);
  }

  int read() override {
    return _reader.read(_upstream);
  }

  int peek() override {
    return _reader.peek(_upstream);
  }

  void flush() override {
    _writer.flush(_upstream);
  }

  // WARNING: we cannot use "override" because most cores don't define this
  // function as virtual
  virtual size_t readBytes(char *buffer, size_t size) {
    return _reader.readBytes(_upstream, buffer, size);
  }

 private:
  Stream &_upstream;
  ReadPolicy _reader;
  WritePolicy _writer;
};

}  // namespace StreamUtils