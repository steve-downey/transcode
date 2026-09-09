::: wording

## Header `<transcode>` synopsis [transcode.syn] {-}

```cpp
// @[transcode.errors]@, error types

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

// @[transcode.reqs]@, range requirements

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

// @[transcode.codec]@, encodings

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

// @[transcode.codec.label]@, label lookup

constexpr optional<codec> get_encoding(string_view label) noexcept;

// @[transcode.custom.reqs]@, codec requirements

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

// @[transcode.custom.reqs]@, codec requirements

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

// @[transcode.whatwg.decode]@, decoding views

template<codec C, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires legacy_byte_range<R>
class whatwg_decode_view : public ranges::view_interface<whatwg_decode_view<C, R, E>> {
  R $base$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.whatwg.decode]@, construction and access
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

template<codec C, input_range R, transcode_error_kind E>
  requires legacy_byte_range<R>
inline constexpr bool enable_borrowed_range<whatwg_decode_view<C, R, E>> =
    borrowed_range<R>;

// @[transcode.whatwg.encode]@, encoding views

template<codec C, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires $whatwg-encode-input$<C, R>
class whatwg_encode_view : public ranges::view_interface<whatwg_encode_view<C, R, E>> {
  R $base$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.whatwg.encode]@, construction and access
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

template<codec C, input_range R, transcode_error_kind E>
  requires $whatwg-encode-input$<C, R>
inline constexpr bool enable_borrowed_range<whatwg_encode_view<C, R, E>> =
    borrowed_range<R>;

// @[transcode.custom.decode]@, class template decode_view

template<decode_codec Codec, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires legacy_byte_range<R>
class decode_view : public ranges::view_interface<decode_view<Codec, R, E>> {
  R $base$;      // exposition only
  Codec $codec$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.custom.decode]@, construction and access
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

// @[transcode.custom.encode]@, class template encode_view

template<encode_codec Codec, ranges::input_range R,
         transcode_error_kind E = transcode_error_kind::replacement>
  requires unicode_scalar_range<R>
class encode_view : public ranges::view_interface<encode_view<Codec, R, E>> {
  R $base$;      // exposition only
  Codec $codec$; // exposition only

  class $iterator$; // exposition only

public:
  // @[transcode.custom.encode]@, construction and access
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

// @[transcode.pipeline]@, transcoding pipelines

template<codec From, codec To> inline constexpr $unspecified$ transcode;

template<decode_codec From, encode_codec To>
constexpr pluggable_transcode_closure<From, To> pluggable_transcode(From from, To to);

// @[transcode.string]@, eager transcoding

string transcode_string(span<const char> src, codec from, codec to);

optional<string> transcode_string(span<const char> src, string_view from_label,
                                  string_view to_label);

template<codec C> u32string transcode_decode_all(span<const char> src);

template<codec C> string transcode_encode_all(u32string_view src);

// @[transcode.codec.sniff]@, byte order mark sniffing

template<legacy_byte_range R> constexpr optional<codec> sniff_encoding(R&& r) noexcept;

// @[transcode.iconv]@, iconv adaptors

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
  R $base$;            // exposition only
  IconvFns $fns$;      // exposition only
  const char* $from$;  // exposition only
  const char* $to$;    // exposition only
  span<char> $buffer$; // exposition only

public:
  class $iterator$; // exposition only

  // @[transcode.iconv]@, construction and access
  explicit iconv_transcode_view(R base, IconvFns fns, const char* from, const char* to,
                                span<char> buf);

  const R& base() const& noexcept;
  R base() &&;

  $iterator$ begin();
  default_sentinel_t end() const;
};

template<typename IconvFns, ranges::input_range R>
  requires legacy_byte_range<R>
class iconv_transcode_or_error_view
    : public ranges::view_interface<iconv_transcode_or_error_view<IconvFns, R>> {
  R $base$;            // exposition only
  IconvFns $fns$;      // exposition only
  const char* $from$;  // exposition only
  const char* $to$;    // exposition only
  span<char> $buffer$; // exposition only

public:
  class $iterator$; // exposition only

  // @[transcode.iconv]@, construction and access
  explicit iconv_transcode_or_error_view(R base, IconvFns fns, const char* from,
                                         const char* to, span<char> buf);

  const R& base() const& noexcept;
  R base() &&;

  $iterator$ begin();
  default_sentinel_t end() const;
};

inline iconv_functions make_real_iconv_fns() noexcept;

inline auto iconv_transcode(const char* from, const char* to, span<char> buf);

inline auto iconv_transcode_or_error(const char* from, const char* to, span<char> buf);

// @[transcode.iconv]@, eager conversion

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

[#]{.pnum} Each element of `R` is required to be a Unicode scalar value.  That is a precondition, not a constraint: `unicode_scalar_range` ([transcode.reqs]) is a requirement on the range's type, and a `char32_t` holding a surrogate or a value above U+10FFFF is not diagnosed.

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

[#]{.pnum} *Remarks*: `decode_view<Codec, R, E>` is `whatwg_decode_view` ([transcode.whatwg.decode]) with the codec supplied as a value rather than named by an enumerator: it presents the bytes of `R` as the Unicode scalar values `Codec` decodes them to, reports a decoding error as `E` says, and decodes lazily.  Everything that clause says about the value type, the error kind and the laziness holds here, of a codec the program wrote rather than one the Encoding Standard defines.

[#]{.pnum} The view models `random_access_range` when `Codec` models `random_access_decode_codec_type` and `R` models `random_access_range`.

```cpp
static constexpr $iterator$ $terminal$();
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

[#]{.pnum} *Remarks*: `encode_view<Codec, R, E>` is `whatwg_encode_view` ([transcode.whatwg.encode]) with the codec supplied as a value rather than named by an enumerator: it presents the Unicode scalar values of `R` as the bytes `Codec` encodes them to, reports an encoding error as `E` says, and encodes lazily.  Each element of `R` is required to be a Unicode scalar value, which is a precondition and not a constraint.

```cpp
static constexpr $iterator$ $terminal$();
```

[#]{.pnum} *Constraints*: `R` models `forward_range`.

[#]{.pnum} *Remarks*: The three POSIX `iconv` entry points a view calls, as a value the program supplies.  `open` opens a conversion descriptor, `convert` converts, and `close` closes it; each has the signature and the semantics POSIX gives the function of the same name.  The views take this as a template parameter rather than calling `::iconv` directly so that a program can supply a different implementation of the same interface -- another library's, or a test's -- and `make_real_iconv_fns` is the one bound to the platform's.

[#]{.pnum} *Remarks*: `iconv_transcode_view<IconvFns, R>` presents the bytes of `R` converted from the encoding named by `from` to the encoding named by `to`, one element per output byte, using the `iconv` implementation `IconvFns` names.  Conversion is lazy and proceeds in batches: the view converts into the caller's buffer, yields those bytes, and converts again.

[#]{.pnum} What the encoding names mean, which pairs convert, and what a conversion does with input the source encoding does not allow are the implementation's `iconv`'s, not this specification's.  That is the point of the adaptor: it gives an interface a program already has a ranges shape and a lifetime, and it does not restate a table it does not own.

[#]{.pnum} A conversion failure is reported as `iconv_error` ([transcode.errors]), which is the granularity POSIX reports at: `EILSEQ`, `EINVAL` and `E2BIG` say *that* a byte sequence is not valid, not why, so the WHATWG error vocabulary the other views use would be claiming knowledge the OS does not return.

[#]{.pnum} The output buffer is the caller's, and is not owned by the view. Its contents between two increments are unspecified, and the program must keep it alive for the lifetime of every iterator the view produces.

[#]{.pnum} *Remarks*: `iconv_transcode_or_error_view<IconvFns, R>` is `iconv_transcode_view` ([transcode.iconv]) with the errors reported rather than skipped: its value type is `expected<char, iconv_error>`, and a conversion failure is an element holding the `iconv_error` POSIX reported rather than input the range passes over.

:::
