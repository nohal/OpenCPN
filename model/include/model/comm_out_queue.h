#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>


class PerfCounter {
  public:
    PerfCounter() : msgs_in(0), msgs_out(0), bytes_in(0), bytes_out(0), bps_in(0), mps_in(0), bps_out(0), mps_out(0), in_out_delay_us(0), overflow_msgs(0) {}
    void in(const size_t bytes, bool ok) {
      auto t1 = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::micro> us_time = t1 - last_in;
      bps_in = 0.95 * bps_in + 0.05 * bytes * 1000000 / us_time.count();
      mps_in = 0.95 * bps_in + 0.05 * 1000000 / us_time.count();
      msgs_in++;
      bytes_in += bytes;
      last_in = t1;
      if(!ok) {
        overflow_msgs++;
      }
    }

    void out(const size_t bytes, std::chrono::time_point<std::chrono::steady_clock> in_ts) {
      auto t1 = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::micro> us_time = t1 - last_in;
      bps_out = 0.95 * bps_out + 0.05 * bytes * 1000000 / us_time.count();
      mps_out = 0.95 * bps_out + 0.05 * 1000000 / us_time.count();
      us_time = t1 - in_ts;
      in_out_delay_us = 0.95 * in_out_delay_us + 0.05 * us_time.count();
      msgs_out++;
      bytes_out += bytes;
      last_out = t1;
    }

    size_t msgs_in;
    size_t msgs_out;
    size_t bytes_in;
    size_t bytes_out;
    uint32_t bps_in;
    double mps_in;
    uint32_t bps_out;
    double mps_out;
    size_t in_out_delay_us;
    size_t overflow_msgs;
    std::chrono::time_point<std::chrono::steady_clock> last_in;
    std::chrono::time_point<std::chrono::steady_clock> last_out;
};

/**
 *  Queue of NMEA0183 messages which only holds a limited amount
 *  of each message type.
 */
class CommOutQueue {
public:
  /**
   * Insert valid line of NMEA0183 data in buffer.
   * @return false on errors including invalid input, else true.
   */
  virtual bool push_back(const std::string& line);

  /**
   * Return  next line to send and remove it from buffer,
   * throws exception if empty.
   */
  virtual std::string pop();

  /** Return number of lines in queue. */
  int size() const;

  /**
   * Create a buffer which stores at most message_count items of each
   * message.
   */
  CommOutQueue(int message_count);

  CommOutQueue() : CommOutQueue(1) {}

  // Disable copying and assignment
  CommOutQueue(const CommOutQueue& other) = delete;
  CommOutQueue& operator=(const CommOutQueue&) = delete;

protected:
  struct BufferItem {
    uint64_t type;
    std::string line;
    BufferItem(const std::string& line);
    BufferItem(const BufferItem& other);
    std::chrono::time_point<std::chrono::steady_clock> ts;
  };

  std::vector<BufferItem> m_buffer;
  mutable std::mutex m_mutex;
  int m_size;
};

/** A  CommOutQueue limited to one message of each kind. */
class CommOutQueueSingle : public CommOutQueue {
public:
  CommOutQueueSingle() : CommOutQueue(1) {}

  /** Insert line of NMEA0183 data in buffer. */
  bool push_back(const std::string& line) override;
};

/** Add unit test measurements to CommOutQueue. */
#include <unordered_map>

class MeasuredCommOutQueue : public CommOutQueue {
public:
  MeasuredCommOutQueue(int size) : CommOutQueue(size), push_time(0), pop_time(0) {}

  bool push_back(const std::string& line) override;
  std::string pop() override;

  std::unordered_map<unsigned long, PerfCounter> msg_perf;
  PerfCounter perf;
  double push_time;
  double pop_time;
};
