#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <map>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace bio {

// Header-only binary IO with RAII and extensible save/load.
class BinaryFile {
public:
  // File open mode.
  enum class Mode { Read, Write };

  // Construct empty; call open() later.
  BinaryFile() = default;
  // Construct and open a file.
  BinaryFile(const std::string &path, Mode mode) { open(path, mode); }

  BinaryFile(const BinaryFile &) = delete; // copy construction not allowed
  BinaryFile &operator=(const BinaryFile &) = delete; // copy assignment not allowed

  // Move support transfers ownership.
  BinaryFile(BinaryFile &&other) noexcept { swap(other); }
  // Move-assign transfers ownership; closes current file if open.
  BinaryFile &operator=(BinaryFile &&other) noexcept {
    if (this != &other) {
      close();
      swap(other);
    }
    return *this;
  }

  // Destructor closes file and flushes pending writes.
  ~BinaryFile() { close(); }

  // Internal buffer size; 0 disables internal buffering.
  void set_buffer_size(std::size_t bytes) {
    if (buffer_size_ == bytes) return;
    if (file_ && mode_ == Mode::Write && buffer_pos_ > 0) {
      flush_write_buffer();
    }
    buffer_size_ = bytes;
    if (file_) {
      reset_buffer();
      apply_stdio_buffering();
    }
  }

  // Open file in binary mode and reset buffers.
  void open(const std::string &path, Mode mode) {
    close();
    mode_ = mode;
    const char *m = (mode == Mode::Read) ? "rb" : "wb";
    file_ = std::fopen(path.c_str(), m);
    if (!file_) {
      throw std::runtime_error("BinaryFile: failed to open file");
    }
    reset_buffer();
    apply_stdio_buffering();
  }

  // Close file; flushes internal write buffer if needed.
  void close() noexcept {
    if (file_) {
      if (mode_ == Mode::Write) {
        flush_write_buffer_noexcept();
      }
      std::fclose(file_);
      file_ = nullptr;
    }
    if (buffer_) {
      std::free(buffer_);
      buffer_ = nullptr;
    }
  }

  // Check if a file is open.
  bool is_open() const noexcept { return file_ != nullptr; }
  // Current open mode.
  Mode mode() const noexcept { return mode_; }

  // Serialize value to file.
  template <class T>
  void save(const T &value) {
    ensure_mode(Mode::Write);
    write(value);
  }

  // Deserialize value from file.
  template <class T>
  void load(T &value) {
    ensure_mode(Mode::Read);
    read(value);
  }

private:
  // Type-dispatched serialization entry points.
  template <class T>
  void write(const T &value);

  template <class T>
  void read(T &value);

  // Raw byte write with optional internal buffering.
  void write_raw(const void *data, std::size_t size) {
    if (size == 0) {
      return;
    }
    if (buffer_size_ == 0) {
      const std::size_t written = std::fwrite(data, 1, size, file_);
      if (written != size) {
        throw std::runtime_error("BinaryFile: fwrite failed");
      }
      return;
    }
    const char *src = static_cast<const char *>(data);
    std::size_t remaining = size;
    while (remaining > 0) {
      const std::size_t space = buffer_size_ - buffer_pos_;
      if (space == 0) {
        flush_write_buffer();
        continue;
      }
      const std::size_t chunk = std::min(remaining, space);
      std::memcpy(buffer_ + buffer_pos_, src, chunk);
      buffer_pos_ += chunk;
      src += chunk;
      remaining -= chunk;
      if (buffer_pos_ == buffer_size_) {
        flush_write_buffer();
      }
    }
  }

  // Raw byte read with optional internal buffering.
  void read_raw(void *data, std::size_t size) {
    if (size == 0) {
      return;
    }
    if (buffer_size_ == 0) {
      const std::size_t read = std::fread(data, 1, size, file_);
      if (read != size) {
        throw std::runtime_error("BinaryFile: fread failed");
      }
      return;
    }
    char *dst = static_cast<char *>(data);
    std::size_t remaining = size;
    while (remaining > 0) {
      if (buffer_pos_ == buffer_filled_) {
        buffer_filled_ = std::fread(buffer_, 1, buffer_size_, file_);
        buffer_pos_ = 0;
        if (buffer_filled_ == 0) {
          throw std::runtime_error("BinaryFile: fread failed");
        }
      }
      const std::size_t avail = buffer_filled_ - buffer_pos_;
      const std::size_t chunk = std::min(remaining, avail);
      std::memcpy(dst, buffer_ + buffer_pos_, chunk);
      buffer_pos_ += chunk;
      dst += chunk;
      remaining -= chunk;
    }
  }

  // Verify file is open and mode matches the operation.
  void ensure_mode(Mode mode) const {
    if (!file_) {
      throw std::runtime_error("BinaryFile: file not open");
    }
    if (mode_ != mode) {
      throw std::runtime_error("BinaryFile: wrong mode");
    }
  }

  // Swap internal state with another instance.
  void swap(BinaryFile &other) noexcept {
    std::swap(file_, other.file_);
    std::swap(mode_, other.mode_);
    std::swap(buffer_, other.buffer_);
    std::swap(buffer_size_, other.buffer_size_);
    std::swap(buffer_pos_, other.buffer_pos_);
    std::swap(buffer_filled_, other.buffer_filled_);
  }

  void reset_buffer() {
    if (buffer_size_ == 0) {
      if (buffer_) {
        std::free(buffer_);
        buffer_ = nullptr;
      }
      buffer_pos_ = 0;
      buffer_filled_ = 0;
      return;
    }
    void *ptr = buffer_ ? std::realloc(buffer_, buffer_size_)
                        : std::malloc(buffer_size_);
    if (!ptr) {
      buffer_ = nullptr;
      throw std::bad_alloc();
    }
    buffer_ = static_cast<char *>(ptr);
    buffer_pos_ = 0;
    buffer_filled_ = 0;
  }

  // Flush internal write buffer (throws on error).
  void flush_write_buffer() {
    if (buffer_pos_ == 0) {
      return;
    }
    const std::size_t written =
        std::fwrite(buffer_, 1, buffer_pos_, file_);
    if (written != buffer_pos_) {
      throw std::runtime_error("BinaryFile: fwrite failed");
    }
    buffer_pos_ = 0;
  }

  // Flush internal write buffer without throwing (for cleanup).
  void flush_write_buffer_noexcept() noexcept {
    if (buffer_pos_ == 0 || !file_) {
      return;
    }
    std::fwrite(buffer_, 1, buffer_pos_, file_);
    buffer_pos_ = 0;
  }

  // Avoid double buffering: internal buffer or stdio buffer, not both.
  void apply_stdio_buffering() {
    if (!file_) return;
    if (buffer_size_ == 0)  std::setvbuf(file_, nullptr, _IOFBF, 0);
     else std::setvbuf(file_, nullptr, _IONBF, 0);
  }

  std::FILE *file_ = nullptr;       // Owned FILE* handle.
  Mode mode_ = Mode::Read;           // Current mode.
  char *buffer_ = nullptr;           // Internal buffer storage (uninitialized).
  std::size_t buffer_size_ = 1u << 20; // Default 1MB buffer.
  std::size_t buffer_pos_ = 0;       // Current position in buffer.
  std::size_t buffer_filled_ = 0;    // Bytes available when reading.
};

namespace detail {

// Remove const/reference qualifiers.
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

// Type trait helpers for common STL containers.
// std::string detection.
template <class T>
struct is_string : std::false_type {};

template <class CharT, class Traits, class Alloc>
struct is_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type {};

template <class T>
inline constexpr bool is_string_v = is_string<T>::value;

// std::vector detection.
template <class T>
struct is_vector : std::false_type {};

template <class U, class Alloc>
struct is_vector<std::vector<U, Alloc>> : std::true_type {};

template <class T>
inline constexpr bool is_vector_v = is_vector<T>::value;

// Extract std::vector element type.
template <class T>
struct vector_value {
  using type = void;
};

template <class U, class Alloc>
struct vector_value<std::vector<U, Alloc>> {
  using type = U;
};

template <class T>
using vector_value_t = typename vector_value<T>::type;

// std::array detection.
template <class T>
struct is_array : std::false_type {};

template <class U, std::size_t N>
struct is_array<std::array<U, N>> : std::true_type {};

template <class T>
inline constexpr bool is_array_v = is_array<T>::value;

// std::map detection.
template <class T>
struct is_map : std::false_type {};

template <class K, class V, class Cmp, class Alloc>
struct is_map<std::map<K, V, Cmp, Alloc>> : std::true_type {};

template <class T>
inline constexpr bool is_map_v = is_map<T>::value;

// std::unordered_map detection.
template <class T>
struct is_unordered_map : std::false_type {};

template <class K, class V, class Hash, class Eq, class Alloc>
struct is_unordered_map<std::unordered_map<K, V, Hash, Eq, Alloc>>
    : std::true_type {};

template <class T>
inline constexpr bool is_unordered_map_v = is_unordered_map<T>::value;

// std::optional detection.
template <class T>
struct is_optional : std::false_type {};

template <class U>
struct is_optional<std::optional<U>> : std::true_type {};

template <class T>
inline constexpr bool is_optional_v = is_optional<T>::value;

// std::variant detection.
template <class T>
struct is_variant : std::false_type {};

template <class... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

template <class T>
inline constexpr bool is_variant_v = is_variant<T>::value;

// tuple-like detection.
template <class T, class = void>
struct is_tuple : std::false_type {};

template <class T>
struct is_tuple<T, std::void_t<decltype(std::tuple_size<T>::value)>>
    : std::true_type {};

template <class T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;

// std::pair detection.
template <class T>
struct is_pair : std::false_type {};

template <class U, class V>
struct is_pair<std::pair<U, V>> : std::true_type {};

template <class T>
inline constexpr bool is_pair_v = is_pair<T>::value;

// Detect user-defined save/load via ADL.
template <class T>
struct has_adl_save {
  template <class U>
  static auto test(int) -> decltype(save(std::declval<BinaryFile &>(),
                                         std::declval<const U &>()),
                                    std::true_type{});
  template <class>
  static std::false_type test(...);
  static constexpr bool value = decltype(test<T>(0))::value;
};

template <class T>
struct has_adl_load {
  template <class U>
  static auto test(int) -> decltype(load(std::declval<BinaryFile &>(),
                                         std::declval<U &>()),
                                    std::true_type{});
  template <class>
  static std::false_type test(...);
  static constexpr bool value = decltype(test<T>(0))::value;
};

template <class T>
inline constexpr bool has_adl_save_v = has_adl_save<T>::value;

template <class T>
inline constexpr bool has_adl_load_v = has_adl_load<T>::value;

// Invoke save/load via ADL (argument-dependent lookup).
template <class T>
inline void adl_save(BinaryFile &file, const T &value) {
  save(file, value);
}

template <class T>
inline void adl_load(BinaryFile &file, T &value) {
  load(file, value);
}

// Store sizes as fixed-width integers on disk.
inline std::uint64_t to_size(std::size_t value) {
  return static_cast<std::uint64_t>(value);
}

// Convert fixed-width size back to size_t.
inline std::size_t from_size(std::uint64_t value) {
  return static_cast<std::size_t>(value);
}

// Variant index dispatch for C++17.
template <class Variant, std::size_t... Is>
inline void read_variant_by_index_impl(BinaryFile &file, Variant &value,
                                       std::uint64_t idx,
                                       std::index_sequence<Is...>) {
  bool handled = false;
  (void)std::initializer_list<int>{
      (idx == Is
           ? (static_cast<void>([&] {
                using Alt = std::variant_alternative_t<Is, Variant>;
                Alt tmp{};
                file.load(tmp);
                value = std::move(tmp);
              }()),
              handled = true,
              0)
           : 0)...};
  if (!handled) {
    throw std::runtime_error("BinaryFile: variant index out of range");
  }
}

template <class Variant>
inline void read_variant_by_index(BinaryFile &file, Variant &value,
                                  std::uint64_t idx) {
  read_variant_by_index_impl(file, value, idx,
                             std::make_index_sequence<
                                 std::variant_size_v<Variant>>{});
}

} // namespace detail

// ADL customization points: users can implement these in their own namespace.
//   void save(bio::BinaryFile &, const T &);
//   void load(bio::BinaryFile &, T &);

template <class T>
void BinaryFile::write(const T &value) {
  using U = detail::remove_cvref_t<T>;
  // Encoding: size prefix for variable-length containers, then elements.
  if constexpr (detail::has_adl_save_v<U>) {
    detail::adl_save(*this, value);
  } else if constexpr (std::is_trivially_copyable_v<U>) {
    write_raw(&value, sizeof(U));
  } else if constexpr (detail::is_string_v<U>) {
    const std::uint64_t size = detail::to_size(value.size());
    write_raw(&size, sizeof(size));
    write_raw(value.data(), value.size());
  } else if constexpr (detail::is_vector_v<U>) {
    const std::uint64_t size = detail::to_size(value.size());
    write_raw(&size, sizeof(size));
    if constexpr (std::is_trivially_copyable_v<detail::vector_value_t<U>>) {
      write_raw(value.data(),
                value.size() * sizeof(detail::vector_value_t<U>));
    } else {
      for (const auto &elem : value) {
        write(elem);
      }
    }
  } else if constexpr (detail::is_array_v<U>) {
    for (const auto &elem : value) {
      write(elem);
    }
  } else if constexpr (detail::is_map_v<U>) {
    const std::uint64_t size = detail::to_size(value.size());
    write_raw(&size, sizeof(size));
    for (const auto &kv : value) {
      write(kv.first);
      write(kv.second);
    }
  } else if constexpr (detail::is_unordered_map_v<U>) {
    const std::uint64_t size = detail::to_size(value.size());
    write_raw(&size, sizeof(size));
    for (const auto &kv : value) {
      write(kv.first);
      write(kv.second);
    }
  } else if constexpr (detail::is_optional_v<U>) {
    const bool has_value = value.has_value();
    write_raw(&has_value, sizeof(has_value));
    if (has_value) {
      write(*value);
    }
  } else if constexpr (detail::is_variant_v<U>) {
    // Store active index followed by payload.
    const std::uint64_t idx = static_cast<std::uint64_t>(value.index());
    write_raw(&idx, sizeof(idx));
    std::visit([this](const auto &elem) { write(elem); }, value);
  } else if constexpr (detail::is_pair_v<U>) {
    write(value.first);
    write(value.second);
  } else if constexpr (detail::is_tuple_v<U>) {
    std::apply([this](const auto &...elems) { (write(elems), ...); }, value);
  } else {
    static_assert(std::is_trivially_copyable_v<U>,
                  "bio::BinaryFile::save: unsupported type; provide save/load"
                  " overloads");
  }
}

template <class T>
void BinaryFile::read(T &value) {
  using U = detail::remove_cvref_t<T>;
  // Decoding must mirror write() order.
  if constexpr (detail::has_adl_load_v<U>) {
    detail::adl_load(*this, value);
  } else if constexpr (std::is_trivially_copyable_v<U>) {
    read_raw(&value, sizeof(U));
  } else if constexpr (detail::is_string_v<U>) {
    std::uint64_t size = 0;
    read_raw(&size, sizeof(size));
    value.resize(detail::from_size(size));
    if (!value.empty()) {
      read_raw(value.data(), value.size());
    }
  } else if constexpr (detail::is_vector_v<U>) {
    std::uint64_t size = 0;
    read_raw(&size, sizeof(size));
    value.resize(detail::from_size(size));
    if constexpr (std::is_trivially_copyable_v<detail::vector_value_t<U>>) {
      read_raw(value.data(),
               value.size() * sizeof(detail::vector_value_t<U>));
    } else {
      for (auto &elem : value) {
        read(elem);
      }
    }
  } else if constexpr (detail::is_array_v<U>) {
    for (auto &elem : value) {
      read(elem);
    }
  } else if constexpr (detail::is_map_v<U>) {
    std::uint64_t size = 0;
    read_raw(&size, sizeof(size));
    value.clear();
    for (std::uint64_t i = 0; i < size; ++i) {
      typename U::key_type key{};
      typename U::mapped_type mapped{};
      read(key);
      read(mapped);
      value.emplace(std::move(key), std::move(mapped));
    }
  } else if constexpr (detail::is_unordered_map_v<U>) {
    std::uint64_t size = 0;
    read_raw(&size, sizeof(size));
    value.clear();
    value.reserve(detail::from_size(size));
    for (std::uint64_t i = 0; i < size; ++i) {
      typename U::key_type key{};
      typename U::mapped_type mapped{};
      read(key);
      read(mapped);
      value.emplace(std::move(key), std::move(mapped));
    }
  } else if constexpr (detail::is_optional_v<U>) {
    bool has_value = false;
    read_raw(&has_value, sizeof(has_value));
    if (has_value) {
      typename U::value_type tmp{};
      read(tmp);
      value = std::move(tmp);
    } else {
      value.reset();
    }
  } else if constexpr (detail::is_variant_v<U>) {
    // Read index then construct the active alternative.
    std::uint64_t idx = 0;
    read_raw(&idx, sizeof(idx));
    if (idx >= std::variant_size_v<U>) {
      throw std::runtime_error("BinaryFile: variant index out of range");
    }
    detail::read_variant_by_index(*this, value, idx);
  } else if constexpr (detail::is_pair_v<U>) {
    read(value.first);
    read(value.second);
  } else if constexpr (detail::is_tuple_v<U>) {
    std::apply([this](auto &...elems) { (read(elems), ...); }, value);
  } else {
    static_assert(std::is_trivially_copyable_v<U>,
                  "bio::BinaryFile::load: unsupported type; provide save/load"
                  " overloads");
  }
}

} // namespace bio
