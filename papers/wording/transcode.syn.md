::: wording

## Header `<transcode>` synopsis [transcode.syn]{- .sref} {-}

```cpp
// @[transcode.errors]{- .sref}@, error types

enum class whatwg_error {
  invalid_byte,
  truncated_sequence,
  overlong_encoding,
  surrogate_code_point,
  out_of_range,
  unmapped_codepoint,
};

enum class iconv_error {
  invalid_sequence,
  incomplete_sequence,
  output_full,
};

enum class transcode_error_kind {
  replacement,
  expected,
};

// @[transcode.reqs]{- .sref}@, range requirements

template<typename T>
concept $legacy-byte-type$ =
    same_as<T, char> || same_as<T, signed char> || same_as<T, unsigned char> ||
    same_as<T, byte>; // exposition only

template<typename R>
concept legacy_byte_range = ranges::range<R> && !is_array_v<remove_cvref_t<R>> &&
                            $legacy-byte-type$<remove_cv_t<ranges::range_value_t<R>>>;

template<typename R>
concept unicode_scalar_range =
    ranges::input_range<R> && !is_array_v<remove_cvref_t<R>> &&
    same_as<remove_cv_t<ranges::range_value_t<R>>, char32_t>;

// @[transcode.codec]{- .sref}@, encodings

enum class codec {
  utf_8,
  replacement,
  x_user_defined,
  ibm866,
  iso_8859_2,
  iso_8859_3,
  iso_8859_4,
  iso_8859_5,
  iso_8859_6,
  iso_8859_7,
  iso_8859_8,
  iso_8859_8_i,
  iso_8859_10,
  iso_8859_13,
  iso_8859_14,
  iso_8859_15,
  iso_8859_16,
  koi8_r,
  koi8_u,
  macintosh,
  windows_874,
  windows_1250,
  windows_1251,
  windows_1252,
  windows_1253,
  windows_1254,
  windows_1255,
  windows_1256,
  windows_1257,
  windows_1258,
  x_mac_cyrillic,
  utf_16be,
  utf_16le,
  gbk,
  gb18030,
  big5,
  shift_jis,
  euc_jp,
  iso_2022_jp,
  euc_kr,
};

// @[transcode.codec.label]{- .sref}@, label lookup

constexpr optional<codec> get_encoding(string_view label) noexcept;

// @[transcode.custom.reqs]{- .sref}@, codec requirements

struct decode_result {
  char32_t code_point{};
  whatwg_error error{};
  bool is_error{false};
};

struct encode_result {
  unsigned char bytes[8]{};
  int count{0};
  bool is_error{false};
};

// @[transcode.custom.reqs]{- .sref}@, codec requirements

template<typename C>
concept decode_codec = semiregular<C> && requires(C& c, const unsigned char*& iter,
                                                  const unsigned char* sentinel) {
  { c.decode_one(iter, sentinel) } -> same_as<decode_result>;
};

template<typename C>
concept random_access_decode_codec_type =
    decode_codec<C> && requires(const C& c, unsigned char byte) {
      { c.decode_byte(byte) } -> same_as<char32_t>;
    };

template<typename C>
concept encode_codec = semiregular<C> && requires(C& c, char32_t cp) {
  { c.encode_one(cp) } -> same_as<encode_result>;
};

template<typename C>
concept flushable_decode_codec = decode_codec<C> && requires(C& c) {
  { c.flush() } -> same_as<optional<decode_result>>;
};

// @[transcode.whatwg.decode]{- .sref}@, decoding views

template<codec C, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires legacy_byte_range<R>
class whatwg_decode_view : public ranges::view_interface<whatwg_decode_view<C, R, E>> {
  R $base$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.whatwg.decode]{- .sref}@, construction and access
  constexpr explicit whatwg_decode_view(R base);

  constexpr const R& base() const& noexcept;
  constexpr R base() &&;

  constexpr $iterator$ begin();
  constexpr $iterator$ begin() const
    requires $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr $iterator$ end()
    requires ranges::forward_range<R> && ranges::common_range<R>;
  constexpr $iterator$ end() const
    requires ranges::forward_range<const R> && ranges::common_range<const R> &&
             $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr default_sentinel_t end() const;
};

template<codec C> inline constexpr $unspecified$ whatwg_decode;

template<codec C> inline constexpr $unspecified$ whatwg_decode_or_error;

template<codec C, random_access_range R, transcode_error_kind E>
  requires legacy_byte_range<R> && detail::$random-access-decode-codec$<C>
inline constexpr bool enable_borrowed_range<random_access_whatwg_decode_view<C, R, E>> =
    borrowed_range<R>;

template<codec C, input_range R, transcode_error_kind E>
  requires legacy_byte_range<R>
inline constexpr bool enable_borrowed_range<whatwg_decode_view<C, R, E>> =
    borrowed_range<R>;

// @[transcode.whatwg.encode]{- .sref}@, encoding views

template<codec C, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires $whatwg-encode-input$<C, R>
class whatwg_encode_view : public ranges::view_interface<whatwg_encode_view<C, R, E>> {
  R $base$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.whatwg.encode]{- .sref}@, construction and access
  constexpr explicit whatwg_encode_view(R base);

  constexpr const R& base() const& noexcept;
  constexpr R base() &&;

  constexpr $iterator$ begin();
  constexpr $iterator$ begin() const
    requires $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr $iterator$ end()
    requires ranges::forward_range<R> && ranges::common_range<R>;
  constexpr $iterator$ end() const
    requires ranges::forward_range<const R> && ranges::common_range<const R> &&
             $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr default_sentinel_t end() const;
};

template<codec C> inline constexpr $unspecified$ whatwg_encode;

template<codec C> inline constexpr $unspecified$ whatwg_encode_or_error;

template<codec C, random_access_range R, transcode_error_kind E>
  requires unicode_scalar_range<R> && detail::$random-access-encode-codec$<C>
inline constexpr bool enable_borrowed_range<random_access_whatwg_encode_view<C, R, E>> =
    borrowed_range<R>;

template<codec C, input_range R, transcode_error_kind E>
  requires detail::$whatwg-encode-input$<C, R>
inline constexpr bool enable_borrowed_range<whatwg_encode_view<C, R, E>> =
    borrowed_range<R>;

// @[transcode.custom.decode]{- .sref}@, class template decode_view

template<decode_codec Codec, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires legacy_byte_range<R>
class decode_view : public ranges::view_interface<decode_view<Codec, R, E>> {
  R $base$;      // exposition only
  Codec $codec$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.custom.decode]{- .sref}@, construction and access
  constexpr explicit decode_view(R base, Codec codec = {});

  constexpr const R& base() const& noexcept;
  constexpr R base() &&;

  constexpr $iterator$ begin();
  constexpr $iterator$ begin() const
    requires $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr $iterator$ end()
    requires ranges::forward_range<R> && ranges::common_range<R>;
  constexpr $iterator$ end() const
    requires ranges::forward_range<const R> && ranges::common_range<const R> &&
             $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr default_sentinel_t end() const;
};

template<decode_codec Codec> constexpr decode_closure<Codec> decode(Codec codec = {});

template<decode_codec Codec>
constexpr decode_closure<Codec, transcode_error_kind::expected> decode_or_error(
    Codec codec = {});

template<decode_codec Codec, ranges::input_range R>
decode_or_error_view decode_or_error_view =
    decode_view<Codec, R, transcode_error_kind::expected>;

template<random_access_decode_codec_type Codec, ranges::random_access_range R>
random_access_decode_or_error_view random_access_decode_or_error_view =
    random_access_decode_view<Codec, R, transcode_error_kind::expected>;

template<decode_codec Codec>
decode_or_error_closure decode_or_error_closure =
    decode_closure<Codec, transcode_error_kind::expected>;

// @[transcode.custom.encode]{- .sref}@, class template encode_view

template<encode_codec Codec, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires unicode_scalar_range<R>
class encode_view : public ranges::view_interface<encode_view<Codec, R, E>> {
  R $base$;      // exposition only
  Codec $codec$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.custom.encode]{- .sref}@, construction and access
  constexpr explicit encode_view(R base, Codec codec = {});

  constexpr const R& base() const& noexcept;
  constexpr R base() &&;

  constexpr $iterator$ begin();
  constexpr $iterator$ begin() const
    requires $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr $iterator$ end()
    requires ranges::forward_range<R> && ranges::common_range<R>;
  constexpr $iterator$ end() const
    requires ranges::forward_range<const R> && ranges::common_range<const R> &&
             $const-iterator-compatible-range$<R> && $const-sentinel-compatible-range$<R>;
  constexpr default_sentinel_t end() const;
};

template<encode_codec Codec> constexpr encode_closure<Codec> encode(Codec codec = {});

template<encode_codec Codec>
constexpr encode_closure<Codec, transcode_error_kind::expected> encode_or_error(
    Codec codec = {});

template<encode_codec Codec, ranges::input_range R>
encode_or_error_view encode_or_error_view =
    encode_view<Codec, R, transcode_error_kind::expected>;

template<encode_codec Codec>
encode_or_error_closure encode_or_error_closure =
    encode_closure<Codec, transcode_error_kind::expected>;

template<codec From, codec To>
struct transcode_closure {
  template<legacy_byte_range R> constexpr auto operator()(R&& r) const;

  template<legacy_byte_range R>
  constexpr friend auto operator|(R&& r, const transcode_closure& self);

  template<typename R>
    requires is_array_v<remove_cvref_t<R>>
  friend auto operator|(R&&, const transcode_closure&);
};

template<codec From, codec To>
inline constexpr auto transcode = transcode_closure<From, To>{};

template<decode_codec From, encode_codec To>
struct pluggable_transcode_closure {
  From from_;
  To to_;

  template<legacy_byte_range R> constexpr auto operator()(R&& r) const;

  template<legacy_byte_range R>
  constexpr friend auto operator|(R&& r, const pluggable_transcode_closure& self);

  template<typename R>
    requires is_array_v<remove_cvref_t<R>>
  friend auto operator|(R&&, const pluggable_transcode_closure&);
};

template<decode_codec From, encode_codec To>
constexpr pluggable_transcode_closure<From, To> pluggable_transcode(From from, To to);

string transcode_string(span<const char> src, codec from, codec to);

optional<string> transcode_string(span<const char> src, string_view from_label,
                                  string_view to_label);

template<codec C> u32string transcode_decode_all(span<const char> src);

template<codec C> string transcode_encode_all(u32string_view src);

// @[transcode.codec.sniff]{- .sref}@, byte order mark sniffing

template<legacy_byte_range R> constexpr optional<codec> sniff_encoding(R&& r) noexcept;

inline constexpr size_t iconv_error_rc = static_cast<size_t>(-1);

struct iconv_functions {
  iconv_t (*open)(const char* tocode, const char* fromcode);
  size_t (*convert)(iconv_t cd, char** inbuf, size_t* inbytesleft, char** outbuf,
                    size_t* outbytesleft);
  int (*close)(iconv_t cd);
};

template<typename IconvFns, ranges::input_range R>
  requires legacy_byte_range<R>
class iconv_transcode_view
    : public ranges::view_interface<iconv_transcode_view<IconvFns, R>> {
public:
  class iterator {
    using base_iter = ranges::iterator_t<R>;
    using base_sent = ranges::sentinel_t<R>;

    iconv_t handle_;
    IconvFns fns_;
    span<char> buffer_;
    char* output_pos_;
    char* output_end_;
    // Accumulates unconsumed input bytes across load() calls so that
    // multi-byte sequences can be assembled before passing to iconv.
    char staging_[64];
    size_t staging_len_{0};
    base_iter current_;
    base_sent end_;
    bool done_;
    bool flushed_{false};

    // Fills output_pos_/output_end_ with the next batch of converted bytes.
    // Handles EINVAL (incomplete sequence) by accumulating more input, and
    // E2BIG/EILSEQ by yielding partial output or skipping one staging byte.
    // Sets done_ = true when all input and staging bytes are exhausted.
    void load();

    // Only iconv_transcode_view::begin() may construct an iterator.
    friend class iconv_transcode_view;
    iterator(iconv_t handle, IconvFns fns, span<char> buffer, base_iter current,
             base_sent end);

  public:
    using iterator_concept = input_iterator_tag;
    using value_type = char;
    using difference_type = ptrdiff_t;
    using reference = char;

    iterator(const iterator&) = delete;
    iterator& operator=(const iterator&) = delete;

    iterator(iterator&&) noexcept;
    iterator& operator=(iterator&&) noexcept;
    ~iterator();

    const base_iter& base() const noexcept { return current_; }

    char operator*() const;
    iterator& operator++();
    void operator++(int);

    friend bool operator==(const iterator& it, default_sentinel_t) { return it.done_; }
  };

  explicit iconv_transcode_view(R base, IconvFns fns, const char* from, const char* to,
                                span<char> buf);

  const R& base() const& noexcept;
  R base() &&;

  iterator begin();
  default_sentinel_t end() const;
};

template<typename IconvFns>
struct iconv_transcode_closure {
  IconvFns fns_;
  const char* from_;
  const char* to_;
  span<char> buffer_;

  template<legacy_byte_range R> auto operator()(R&& r) const;

  template<legacy_byte_range R>
  friend auto operator|(R&& r, const iconv_transcode_closure& self);
};

template<typename IconvFns, ranges::input_range R>
  requires legacy_byte_range<R>
class iconv_transcode_or_error_view
    : public ranges::view_interface<iconv_transcode_or_error_view<IconvFns, R>> {
public:
  class iterator {
    using base_iter = ranges::iterator_t<R>;
    using base_sent = ranges::sentinel_t<R>;
    using result_t = expected<char, iconv_error>;

    iconv_t handle_;
    IconvFns fns_;
    span<char> buffer_;
    char* output_pos_;
    char* output_end_;
    char staging_[64];
    size_t staging_len_{0};
    base_iter current_;
    base_sent end_;
    bool done_;
    bool flushed_{false};
    bool has_error_{false};
    iconv_error error_value_{};

    void load();

    friend class iconv_transcode_or_error_view;
    iterator(iconv_t handle, IconvFns fns, span<char> buffer, base_iter current,
             base_sent end);

  public:
    using iterator_concept = input_iterator_tag;
    using value_type = result_t;
    using difference_type = ptrdiff_t;
    using reference = result_t;

    iterator(const iterator&) = delete;
    iterator& operator=(const iterator&) = delete;

    iterator(iterator&&) noexcept;
    iterator& operator=(iterator&&) noexcept;
    ~iterator();

    const base_iter& base() const noexcept { return current_; }

    result_t operator*() const;
    iterator& operator++();
    void operator++(int);

    friend bool operator==(const iterator& it, default_sentinel_t) { return it.done_; }
  };

  explicit iconv_transcode_or_error_view(R base, IconvFns fns, const char* from,
                                         const char* to, span<char> buf);

  const R& base() const& noexcept;
  R base() &&;

  iterator begin();
  default_sentinel_t end() const;
};

template<typename IconvFns>
struct iconv_transcode_or_error_closure {
  IconvFns fns_;
  const char* from_;
  const char* to_;
  span<char> buffer_;

  template<legacy_byte_range R> auto operator()(R&& r) const;

  template<legacy_byte_range R>
  friend auto operator|(R&& r, const iconv_transcode_or_error_closure& self);
};

inline iconv_functions make_real_iconv_fns() noexcept;

inline auto iconv_transcode(const char* from, const char* to, span<char> buf);

inline auto iconv_transcode_or_error(const char* from, const char* to, span<char> buf);

template<typename IconvFns>
struct iconv_guard {
  iconv_t handle;
  IconvFns fns;
  ~iconv_guard();
};

struct iconv_input_buf {
  vector<char> storage;
  char* data;
  size_t size;
};

template<legacy_byte_range R> iconv_input_buf materialize_iconv_input(R&& source);

template<typename Container = string, typename IconvFns, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to,
                             IconvFns fns);

template<typename Container = string, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to);

template<typename IconvFns, legacy_byte_range R, output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to, Output output,
                            IconvFns fns);

template<legacy_byte_range R, output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to,
                            Output output);

template<typename Container = string, typename IconvFns, legacy_byte_range R>
expected<Container, iconv_error> iconv_transcode_to_or_error(R&& source,
                                                             const char* from,
                                                             const char* to,
                                                             IconvFns fns);

template<typename Container = string, legacy_byte_range R>
expected<Container, iconv_error> iconv_transcode_to_or_error(R&& source,
                                                             const char* from,
                                                             const char* to);
```

[#]{.pnum} *Remarks*: What a codec's `decode_one` returns.  `is_error` says whether the decode failed.  When it did, `error` says why and `code_point` is U+FFFD REPLACEMENT CHARACTER; when it did not, `code_point` is the decoded Unicode scalar value and `error` is unspecified.

[#]{.pnum} *Remarks*: What a codec's `encode_one` returns.  `is_error` says whether the encode failed -- the encoding has no representation for that scalar value. When it did not, `count` is the number of bytes written and `bytes[0]` through `bytes[count - 1]` are the encoded form; when it did, `count` is zero and the view substitutes or reports the error as its error kind says.

[#]{.pnum} *Remarks*: `whatwg_decode_view<C, R, E>` presents the bytes of `R` as the Unicode scalar values the encoding `C` decodes them to, one element per decoded scalar value.  A byte sequence the encoding does not allow is a decoding error, reported as `E` says: as U+FFFD REPLACEMENT CHARACTER when `E` is `transcode_error_kind::replacement`, and as an `unexpected` holding a `whatwg_error` when it is `transcode_error_kind::expected`.  Decoding is lazy: an element is decoded when the iterator reaches it.

[#]{.pnum} The view models `random_access_range` when `C` names an encoding that decodes one byte to one scalar value and `R` models `random_access_range`.  How an implementation achieves that is not specified; it is a property of the view rather than a second view.

[#]{.pnum} *Remarks*: `whatwg_encode_view<C, R, E>` presents the Unicode scalar values of `R` as the bytes the encoding `C` encodes them to, one element per byte. A scalar value the encoding cannot represent is an encoding error, reported as `E` says: as `'?'` when `E` is `transcode_error_kind::replacement`, and as an `unexpected` holding `whatwg_error::unmapped_codepoint` when it is `transcode_error_kind::expected`.  Encoding is lazy, and one input element can produce several output elements.

[#]{.pnum} Each element of `R` is required to be a Unicode scalar value.  That is a precondition, not a constraint: `unicode_scalar_range` ([transcode.reqs]{- .sref}) is a requirement on the range's type, and a `char32_t` holding a surrogate or a value above U+10FFFF is not diagnosed.

[#]{.pnum} `C` is required to be an encoding the WHATWG Encoding Standard defines an encoder for.  It defines none for `utf_16be`, `utf_16le`, `replacement` or `x_user_defined`, and the view does not accept them.

```cpp
constexpr iterator begin() const;
```

[#]{.pnum} *Constraints*: `R` models `$const-iterator-compatible-range$`.

```cpp
constexpr iterator end() const;
```

[#]{.pnum} *Constraints*: `R` models `$const-iterator-compatible-range$` and `R` models `range`.

```cpp
constexpr auto size() const;
```

[#]{.pnum} *Constraints*: `R` models `sized_range`.

[#]{.pnum} *Remarks*: `decode_view<Codec, R, E>` is `whatwg_decode_view` ([transcode.whatwg.decode]{- .sref}) with the codec supplied as a value rather than named by an enumerator: it presents the bytes of `R` as the Unicode scalar values `Codec` decodes them to, reports a decoding error as `E` says, and decodes lazily.  Everything that clause says about the value type, the error kind and the laziness holds here, of a codec the program wrote rather than one the Encoding Standard defines.

[#]{.pnum} The view models `random_access_range` when `Codec` models `random_access_decode_codec_type` and `R` models `random_access_range`.

```cpp
static constexpr $iterator$ $terminal$();
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

[#]{.pnum} *Remarks*: `encode_view<Codec, R, E>` is `whatwg_encode_view` ([transcode.whatwg.encode]{- .sref}) with the codec supplied as a value rather than named by an enumerator: it presents the Unicode scalar values of `R` as the bytes `Codec` encodes them to, reports an encoding error as `E` says, and encodes lazily.  Each element of `R` is required to be a Unicode scalar value, which is a precondition and not a constraint.

```cpp
static constexpr $iterator$ $terminal$();
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

:::
