::: wording

## Header `<transcode>` synopsis [transcode.syn] {-}

```cpp
namespace std {
  // @[transcode.errors]@, error types

  enum class iconv_error {
    invalid_sequence,
    incomplete_sequence,
    output_full,
    open_failed,
    system_error,
  };

  enum class error_mode {
    replacement,
    report,
  };

  // @[transcode.errors]@, error types

  enum class error {
    invalid_byte,
    truncated_sequence,
    overlong_encoding,
    surrogate_code_point,
    out_of_range,
    unmapped_codepoint,
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

  // @[transcode.whatwg.encoding]@, encodings

  enum class encoding {
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

  // @[transcode.whatwg.label]@, label lookup

  constexpr optional<encoding> get_encoding(string_view label) noexcept;

  // @[transcode.whatwg.label]@, label lookup

  constexpr optional<encoding> get_encoding(text_encoding::id id) noexcept;

  constexpr optional<text_encoding::id> to_text_encoding_id(encoding e) noexcept;

  // @[transcode.codec.reqs]@, codec requirements

  struct decode_result {
    char32_t code_point{};
    whatwg::error error{};
    bool is_error{false};
    bool is_empty{false};
  };

  struct encode_result {
    unsigned char bytes[8]{};
    int count{0};
    bool is_error{false};
  };

  // @[transcode.codec.reqs]@, codec requirements

  template<typename C>
  concept text_decoder = semiregular<C> && requires(C& c, const unsigned char*& iter,
                                                    const unsigned char* sentinel) {
    { c.decode_one(iter, sentinel) } -> same_as<decode_result>;
  };

  template<typename C>
  concept single_byte_text_decoder =
      text_decoder<C> && requires(const C& c, unsigned char byte) {
        { c.decode_byte(byte) } -> same_as<char32_t>;
      };

  template<typename C>
  concept text_encoder = semiregular<C> && requires(C& c, char32_t cp) {
    { c.encode_one(cp) } -> same_as<encode_result>;
  };

  template<typename C>
  concept single_byte_text_encoder =
      text_encoder<C> && requires(const C& c, char32_t scalar) {
        { c.encode_byte(scalar) } -> same_as<optional<unsigned char>>;
      };

  template<typename C>
  concept flushable_text_decoder = text_decoder<C> && requires(C& c) {
    { c.flush() } -> same_as<optional<decode_result>>;
  };

  template<typename C>
  concept bom_aware_text_decoder =
      text_decoder<C> &&
      requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
        { c.decode_first(iter, sentinel) } -> same_as<decode_result>;
      };

  template<typename C>
  concept flushable_text_encoder = text_encoder<C> && requires(C& c) {
    { c.flush() } -> same_as<optional<encode_result>>;
  };

  // @[transcode.whatwg.decoders]@, decoders

  template<encoding C>
  class decoder {
    [[no_unique_address]] detail::decode_state_t<C> $state${}; // exposition only

  public:
    // @[transcode.whatwg.decoders]@, decoding
    template<input_iterator I, sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end);

    template<input_iterator I, sentinel_for<I> S>
    constexpr decode_result decode_first(I& current, S end)
      requires $strips-leading-bom$<C>;

    constexpr char32_t decode_byte(unsigned char byte) const
      requires $random-access-decode-codec$<C>;

    constexpr optional<decode_result> flush()
      requires(!same_as<detail::decode_state_t<C>, detail::no_decode_state>);

    friend constexpr bool operator==(const decoder&, const decoder&) = default;
  };

  // @[transcode.whatwg.encoders]@, encoders

  template<encoding C>
    requires $whatwg-encode-codec$<C>
  class encoder {
    [[no_unique_address]] detail::encode_state_t<C> $state${}; // exposition only

  public:
    // @[transcode.whatwg.encoders]@, encoding
    constexpr encode_result encode_one(char32_t scalar);

    constexpr optional<unsigned char> encode_byte(char32_t scalar) const
      requires $random-access-encode-codec$<C>;

    constexpr optional<encode_result> flush()
      requires(C == encoding::iso_2022_jp);

    friend constexpr bool operator==(const encoder&, const encoder&) = default;
  };

  // @[transcode.codec.decode]@, class template decode_view

  template<text_decoder Codec, ranges::input_range R,
           error_mode E = error_mode::replacement>
    requires legacy_byte_range<R>
  class decode_view : public ranges::view_interface<decode_view<Codec, R, E>> {
    R $base$;                            // exposition only
    [[no_unique_address]] Codec $codec$; // exposition only

    class $iterator$; // exposition only

  public:
    // @[transcode.codec.decode]@, construction and access
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

  template<text_decoder Codec> constexpr auto decode(Codec codec = {});

  template<text_decoder Codec> constexpr auto decode_or_error(Codec codec = {});

  template<text_decoder Codec, input_range R, error_mode E>
    requires legacy_byte_range<R>
  inline constexpr bool enable_borrowed_range<decode_view<Codec, R, E>> =
      borrowed_range<R>;

  // @[transcode.codec.encode]@, class template encode_view

  template<text_encoder Codec, ranges::input_range R,
           error_mode E = error_mode::replacement>
    requires unicode_scalar_range<R>
  class encode_view : public ranges::view_interface<encode_view<Codec, R, E>> {
    R $base$;                            // exposition only
    [[no_unique_address]] Codec $codec$; // exposition only

    class $iterator$; // exposition only

  public:
    // @[transcode.codec.encode]@, construction and access
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

  template<text_encoder Codec> constexpr auto encode(Codec codec = {});

  template<text_encoder Codec> constexpr auto encode_or_error(Codec codec = {});

  template<text_encoder Codec, input_range R, error_mode E>
    requires unicode_scalar_range<R>
  inline constexpr bool enable_borrowed_range<encode_view<Codec, R, E>> =
      borrowed_range<R>;

  // @[transcode.whatwg.decode]@, decoding adaptors

  template<whatwg::encoding C> inline constexpr $unspecified$ decode;

  template<whatwg::encoding C> inline constexpr $unspecified$ decode_or_error;

  // @[transcode.whatwg.encode]@, encoding adaptors

  template<whatwg::encoding C> inline constexpr $unspecified$ encode;

  template<whatwg::encoding C> inline constexpr $unspecified$ encode_or_error;

  // @[transcode.pipeline]@, transcoding pipelines

  template<text_decoder From, text_encoder To>
  constexpr auto transcode(From from, To to);

  // @[transcode.pipeline]@, transcoding pipelines

  template<encoding From, encoding To> inline constexpr $unspecified$ transcode;

  // @[transcode.string]@, eager transcoding

  constexpr bool has_encoder(whatwg::encoding encoding) noexcept;

  optional<string> transcode_string(span<const char> src, whatwg::encoding from,
                                    whatwg::encoding to);

  optional<string> transcode_string(span<const char> src, string_view from_label,
                                    string_view to_label);

  // @[transcode.bulk]@, eager bulk conversion

  template<encoding C, legacy_byte_range R>
  constexpr vector<char32_t> decode_to(R&& source);

  template<encoding C, typename Container = string, unicode_scalar_range R>
  constexpr Container encode_to(R&& source);

  template<encoding C, legacy_byte_range R, output_iterator<char32_t> Output>
  constexpr void decode_into(R&& source, Output output);

  template<encoding C, unicode_scalar_range R, output_iterator<char> Output>
  constexpr void encode_into(R&& source, Output output);

  // @[transcode.bulk]@, eager bulk conversion

  template<text_decoder Codec, legacy_byte_range R>
  constexpr vector<char32_t> decode_to(Codec codec, R&& source);

  template<text_encoder Codec, typename Container = string, unicode_scalar_range R>
  constexpr Container encode_to(Codec codec, R&& source);

  template<text_decoder Codec, legacy_byte_range R, output_iterator<char32_t> Output>
  constexpr void decode_into(Codec codec, R&& source, Output output);

  template<text_encoder Codec, unicode_scalar_range R, output_iterator<char> Output>
  constexpr void encode_into(Codec codec, R&& source, Output output);

  // @[transcode.whatwg.sniff]@, byte order mark sniffing

  template<legacy_byte_range R>
    requires ranges::forward_range<R>
  constexpr optional<encoding> bom_sniff(R&& r) noexcept;

  // @[transcode.iconv]@, iconv adaptors

  inline constexpr size_t iconv_min_buffer_size = 4;

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
    explicit iconv_transcode_view(R base, IconvFns fns, const char* from,
                                  const char* to, span<char> buf);

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

  inline auto iconv_transcode_or_error(const char* from, const char* to,
                                       span<char> buf);

  // @[transcode.iconv]@, eager conversion

  template<typename Container = string, typename IconvFns, legacy_byte_range R>
  Container iconv_transcode_to(R&& source, const char* from, const char* to,
                               IconvFns fns);

  template<typename Container = string, legacy_byte_range R>
  Container iconv_transcode_to(R&& source, const char* from, const char* to);

  template<typename IconvFns, legacy_byte_range R, output_iterator<char> Output>
  Output iconv_transcode_into(R&& source, const char* from, const char* to,
                              Output output, IconvFns fns);

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
}
```

:::
