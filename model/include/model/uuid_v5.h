/**************************************************************************
 *   Copyright (C) 2026 by Pavel Kalian                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Minimal UUID version 5 (SHA-1 namespace-based) implementation.
 */

#ifndef UUID_V5_H_
#define UUID_V5_H_

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cctype>

using uuid_bytes = std::array<std::uint8_t, 16>;
using sha1_bytes = std::array<std::uint8_t, 20>;

namespace UUIDv5 {
// ---------------------------
// Minimal SHA-1 (big-endian)
// ---------------------------
namespace hash {

static inline std::uint32_t rol(std::uint32_t x, std::uint32_t n) {
  return (x << n) | (x >> (32U - n));
}

class sha1 {
public:
  sha1() { reset(); }

  void reset() {
    h0_ = 0x67452301U;
    h1_ = 0xEFCDAB89U;
    h2_ = 0x98BADCFEU;
    h3_ = 0x10325476U;
    h4_ = 0xC3D2E1F0U;
    length_bits_ = 0;
    buffer_.clear();
  }

  void update(const std::uint8_t* data, std::size_t len) {
    length_bits_ += static_cast<std::uint64_t>(len) * 8ULL;
    buffer_.insert(buffer_.end(), data, data + len);
    while (buffer_.size() >= 64) {
      process_block(&buffer_[0]);
      buffer_.erase(buffer_.begin(), buffer_.begin() + 64);
    }
  }

  sha1_bytes finalize() {
    // Padding: 0x80, then zeros until length ≡ 56 (mod 64), then 64-bit length
    // (big-endian)
    std::vector<std::uint8_t> tail = buffer_;
    tail.push_back(0x80U);
    while ((tail.size() % 64) != 56) {
      tail.push_back(0x00U);
    }
    std::uint64_t L = length_bits_;
    for (int i = 7; i >= 0; --i) {
      tail.push_back(static_cast<std::uint8_t>((L >> (i * 8)) & 0xFFU));
    }
    // Process final blocks
    for (std::size_t i = 0; i + 64 <= tail.size(); i += 64) {
      process_block(&tail[i]);
    }

    sha1_bytes out{};
    write_be32(out, 0, h0_);
    write_be32(out, 4, h1_);
    write_be32(out, 8, h2_);
    write_be32(out, 12, h3_);
    write_be32(out, 16, h4_);
    return out;
  }

private:
  std::uint32_t h0_, h1_, h2_, h3_, h4_;
  std::uint64_t length_bits_;
  std::vector<std::uint8_t> buffer_;

  static inline std::uint32_t read_be32(const std::uint8_t* p) {
    return (static_cast<std::uint32_t>(p[0]) << 24) |
           (static_cast<std::uint32_t>(p[1]) << 16) |
           (static_cast<std::uint32_t>(p[2]) << 8) |
           (static_cast<std::uint32_t>(p[3]));
  }

  static inline void write_be32(sha1_bytes& out, std::size_t pos,
                                std::uint32_t v) {
    out[pos + 0] = static_cast<std::uint8_t>((v >> 24) & 0xFFU);
    out[pos + 1] = static_cast<std::uint8_t>((v >> 16) & 0xFFU);
    out[pos + 2] = static_cast<std::uint8_t>((v >> 8) & 0xFFU);
    out[pos + 3] = static_cast<std::uint8_t>((v) & 0xFFU);
  }

  void process_block(const std::uint8_t* block) {
    std::uint32_t w[80];
    for (int t = 0; t < 16; ++t) {
      w[t] = read_be32(block + 4 * t);
    }
    for (int t = 16; t < 80; ++t) {
      w[t] = rol(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16], 1);
    }

    std::uint32_t a = h0_, b = h1_, c = h2_, d = h3_, e = h4_;
    for (int t = 0; t < 80; ++t) {
      std::uint32_t f, k;
      if (t < 20) {
        f = (b & c) | ((~b) & d);
        k = 0x5A827999U;
      } else if (t < 40) {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1U;
      } else if (t < 60) {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDCU;
      } else {
        f = b ^ c ^ d;
        k = 0xCA62C1D6U;
      }
      std::uint32_t temp = rol(a, 5) + f + e + k + w[t];
      e = d;
      d = c;
      c = rol(b, 30);
      b = a;
      a = temp;
    }

    h0_ += a;
    h1_ += b;
    h2_ += c;
    h3_ += d;
    h4_ += e;
  }
};

}  // namespace hash

// -----------------------------------------
// UUID parsing / formatting (canonical form)
// -----------------------------------------
inline bool is_hex(char c) {
  return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

inline std::uint8_t hex2byte(char hi, char lo) {
  auto h =
      std::uint8_t(std::isdigit(hi) ? hi - '0' : (std::toupper(hi) - 'A' + 10));
  auto l =
      std::uint8_t(std::isdigit(lo) ? lo - '0' : (std::toupper(lo) - 'A' + 10));
  return static_cast<std::uint8_t>((h << 4) | l);
}

// Parses canonical "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
inline uuid_bytes parse_uuid(const std::string& s) {
  std::string u = s;
  // Optional braces
  if (!u.empty() && u.front() == '{' && u.back() == '}') {
    u = u.substr(1, u.size() - 2);
  }
  // Validate length and hyphen positions
  if (u.size() != 36 || u[8] != '-' || u[13] != '-' || u[18] != '-' ||
      u[23] != '-') {
    throw std::invalid_argument("UUID string not in canonical 8-4-4-4-12 form");
  }
  // Validate hex characters
  for (std::size_t i = 0; i < u.size(); ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) continue;
    if (!is_hex(u[i])) {
      throw std::invalid_argument("UUID contains non-hex characters");
    }
  }

  uuid_bytes out{};
  auto take_byte = [&](std::size_t idx) -> std::uint8_t {
    return hex2byte(u[idx], u[idx + 1]);
  };

  std::size_t i = 0, o = 0;
  for (; i < u.size() && o < 16;) {
    if (u[i] == '-') {
      ++i;
      continue;
    }
    out[o++] = take_byte(i);
    i += 2;
  }
  return out;
}

inline std::string format_uuid(const uuid_bytes& u) {
  std::ostringstream oss;
  oss << std::hex << std::nouppercase << std::setfill('0');
  auto emit = [&](int start, int count, bool dashAfter) {
    for (int i = 0; i < count; ++i) {
      oss << std::setw(2) << static_cast<int>(u[start + i]);
    }
    if (dashAfter) oss << '-';
  };
  emit(0, 4, true);    // time_low
  emit(4, 2, true);    // time_mid
  emit(6, 2, true);    // time_hi_and_version
  emit(8, 2, true);    // clock_seq
  emit(10, 6, false);  // node
  return oss.str();
}

// -------------------------------------------
// Well-known namespace UUIDs (RFC 4122, §4.3)
// -------------------------------------------
inline uuid_bytes NS_DNS() {
  return parse_uuid("6ba7b810-9dad-11d1-80b4-00c04fd430c8");
}
inline uuid_bytes NS_URL() {
  return parse_uuid("6ba7b811-9dad-11d1-80b4-00c04fd430c8");
}
inline uuid_bytes NS_OID() {
  return parse_uuid("6ba7b812-9dad-11d1-80b4-00c04fd430c8");
}
inline uuid_bytes NS_X500() {
  return parse_uuid("6ba7b814-9dad-11d1-80b4-00c04fd430c8");
}

// ----------------------------------------------------------
// UUID v5 (SHA-1) generation from namespace + name (UTF-8)
// ----------------------------------------------------------
inline uuid_bytes uuid_v5_bytes(const uuid_bytes& ns,
                                const std::vector<std::uint8_t>& name_octets) {
  // Per RFC 4122: hash over namespace bytes (in network order) followed by name
  // octets.
  hash::sha1 sha;
  sha.update(ns.data(), ns.size());
  if (!name_octets.empty()) {
    sha.update(name_octets.data(), name_octets.size());
  }
  sha1_bytes h = sha.finalize();

  uuid_bytes out{};
  // First 16 bytes of SHA-1
  for (int i = 0; i < 16; ++i) out[i] = h[i];

  // Set version (5) in time_hi_and_version (byte 6 high nibble)
  out[6] = static_cast<std::uint8_t>((out[6] & 0x0F) | 0x50);

  // Set RFC 4122 variant in clock_seq_hi_and_reserved (byte 8 top two bits =
  // 10)
  out[8] = static_cast<std::uint8_t>((out[8] & 0x3F) | 0x80);

  return out;
}

inline std::string uuid_v5(const uuid_bytes& ns, const std::string& name_utf8) {
  std::vector<std::uint8_t> octets(name_utf8.begin(),
                                   name_utf8.end());  // UTF-8 bytes
  return format_uuid(uuid_v5_bytes(ns, octets));
}

// Convenience overload: namespace as string
inline std::string uuid_v5(const std::string& namespace_uuid,
                           const std::string& name_utf8) {
  return uuid_v5(parse_uuid(namespace_uuid), name_utf8);
}

// Parsed constant for f8de1cfa-f30a-5484-9456-4cf3e15699c0
inline uuid_bytes NS_OPENCPN() {
  return parse_uuid("6ba7b814-9dad-11d1-80b4-00c04fd430c8");
};

// Convenience overload: namespace as string
inline std::string uuid_v5_opencpn(const std::string& name_utf8) {
  return uuid_v5(NS_OPENCPN(), name_utf8);
}

}  // namespace UUIDv5

#endif  // UUID_V5_H_
