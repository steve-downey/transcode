// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// GENERATED — do not edit. Regenerate: uv run tools/generate_tables.py
// Source: WHATWG Encoding Standard index-gb18030-ranges.txt

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GB18030_RANGES_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GB18030_RANGES_HPP

#include <beman/transcode/config.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
#include <cstdint>

#endif
namespace beman::transcoding::detail::tables {

struct gb18030_range {
    std::uint32_t pointer;
    char32_t      codepoint;
};

inline constexpr gb18030_range gb18030_ranges[207] = {
    {0u, 0x0080u},     {36u, 0x00A5u},    {38u, 0x00A9u},     {45u, 0x00B2u},    {50u, 0x00B8u},    {81u, 0x00D8u},
    {89u, 0x00E2u},    {95u, 0x00EBu},    {96u, 0x00EEu},     {100u, 0x00F4u},   {103u, 0x00F8u},   {104u, 0x00FBu},
    {105u, 0x00FDu},   {109u, 0x0102u},   {126u, 0x0114u},    {133u, 0x011Cu},   {148u, 0x012Cu},   {172u, 0x0145u},
    {175u, 0x0149u},   {179u, 0x014Eu},   {208u, 0x016Cu},    {306u, 0x01CFu},   {307u, 0x01D1u},   {308u, 0x01D3u},
    {309u, 0x01D5u},   {310u, 0x01D7u},   {311u, 0x01D9u},    {312u, 0x01DBu},   {313u, 0x01DDu},   {341u, 0x01FAu},
    {428u, 0x0252u},   {443u, 0x0262u},   {544u, 0x02C8u},    {545u, 0x02CCu},   {558u, 0x02DAu},   {741u, 0x03A2u},
    {742u, 0x03AAu},   {749u, 0x03C2u},   {750u, 0x03CAu},    {805u, 0x0402u},   {819u, 0x0450u},   {820u, 0x0452u},
    {7922u, 0x2011u},  {7924u, 0x2017u},  {7925u, 0x201Au},   {7927u, 0x201Eu},  {7934u, 0x2027u},  {7943u, 0x2031u},
    {7944u, 0x2034u},  {7945u, 0x2036u},  {7950u, 0x203Cu},   {8062u, 0x20ADu},  {8148u, 0x2104u},  {8149u, 0x2106u},
    {8152u, 0x210Au},  {8164u, 0x2117u},  {8174u, 0x2122u},   {8236u, 0x216Cu},  {8240u, 0x217Au},  {8262u, 0x2194u},
    {8264u, 0x219Au},  {8374u, 0x2209u},  {8380u, 0x2210u},   {8381u, 0x2212u},  {8384u, 0x2216u},  {8388u, 0x221Bu},
    {8390u, 0x2221u},  {8392u, 0x2224u},  {8393u, 0x2226u},   {8394u, 0x222Cu},  {8396u, 0x222Fu},  {8401u, 0x2238u},
    {8406u, 0x223Eu},  {8416u, 0x2249u},  {8419u, 0x224Du},   {8424u, 0x2253u},  {8437u, 0x2262u},  {8439u, 0x2268u},
    {8445u, 0x2270u},  {8482u, 0x2296u},  {8485u, 0x229Au},   {8496u, 0x22A6u},  {8521u, 0x22C0u},  {8603u, 0x2313u},
    {8936u, 0x246Au},  {8946u, 0x249Cu},  {9046u, 0x254Cu},   {9050u, 0x2574u},  {9063u, 0x2590u},  {9066u, 0x2596u},
    {9076u, 0x25A2u},  {9092u, 0x25B4u},  {9100u, 0x25BEu},   {9108u, 0x25C8u},  {9111u, 0x25CCu},  {9113u, 0x25D0u},
    {9131u, 0x25E6u},  {9162u, 0x2607u},  {9164u, 0x260Au},   {9218u, 0x2641u},  {9219u, 0x2643u},  {11329u, 0x2E82u},
    {11331u, 0x2E85u}, {11334u, 0x2E89u}, {11336u, 0x2E8Du},  {11346u, 0x2E98u}, {11361u, 0x2EA8u}, {11363u, 0x2EABu},
    {11366u, 0x2EAFu}, {11370u, 0x2EB4u}, {11372u, 0x2EB8u},  {11375u, 0x2EBCu}, {11389u, 0x2ECBu}, {11682u, 0x2FFCu},
    {11686u, 0x3004u}, {11687u, 0x3018u}, {11692u, 0x301Fu},  {11694u, 0x302Au}, {11714u, 0x303Fu}, {11716u, 0x3094u},
    {11723u, 0x309Fu}, {11725u, 0x30F7u}, {11730u, 0x30FFu},  {11736u, 0x312Au}, {11982u, 0x322Au}, {11989u, 0x3232u},
    {12102u, 0x32A4u}, {12336u, 0x3390u}, {12348u, 0x339Fu},  {12350u, 0x33A2u}, {12384u, 0x33C5u}, {12393u, 0x33CFu},
    {12395u, 0x33D3u}, {12397u, 0x33D6u}, {12510u, 0x3448u},  {12553u, 0x3474u}, {12851u, 0x359Fu}, {12962u, 0x360Fu},
    {12973u, 0x361Bu}, {13738u, 0x3919u}, {13823u, 0x396Fu},  {13919u, 0x39D1u}, {13933u, 0x39E0u}, {14080u, 0x3A74u},
    {14298u, 0x3B4Fu}, {14585u, 0x3C6Fu}, {14698u, 0x3CE1u},  {15583u, 0x4057u}, {15847u, 0x4160u}, {16318u, 0x4338u},
    {16434u, 0x43ADu}, {16438u, 0x43B2u}, {16481u, 0x43DEu},  {16729u, 0x44D7u}, {17102u, 0x464Du}, {17122u, 0x4662u},
    {17315u, 0x4724u}, {17320u, 0x472Au}, {17402u, 0x477Du},  {17418u, 0x478Eu}, {17859u, 0x4948u}, {17909u, 0x497Bu},
    {17911u, 0x497Eu}, {17915u, 0x4984u}, {17916u, 0x4987u},  {17936u, 0x499Cu}, {17939u, 0x49A0u}, {17961u, 0x49B8u},
    {18664u, 0x4C78u}, {18703u, 0x4CA4u}, {18814u, 0x4D1Au},  {18962u, 0x4DAFu}, {19043u, 0x9FA6u}, {33469u, 0xE76Cu},
    {33470u, 0xE7C8u}, {33471u, 0xE7E7u}, {33484u, 0xE815u},  {33485u, 0xE819u}, {33490u, 0xE81Fu}, {33497u, 0xE827u},
    {33501u, 0xE82Du}, {33505u, 0xE833u}, {33513u, 0xE83Cu},  {33520u, 0xE844u}, {33536u, 0xE856u}, {33550u, 0xE865u},
    {37845u, 0xF92Du}, {37921u, 0xF97Au}, {37948u, 0xF996u},  {38029u, 0xF9E8u}, {38038u, 0xF9F2u}, {38064u, 0xFA10u},
    {38065u, 0xFA12u}, {38066u, 0xFA15u}, {38069u, 0xFA19u},  {38075u, 0xFA22u}, {38076u, 0xFA25u}, {38078u, 0xFA2Au},
    {39108u, 0xFE32u}, {39109u, 0xFE45u}, {39113u, 0xFE53u},  {39114u, 0xFE58u}, {39115u, 0xFE67u}, {39116u, 0xFE6Cu},
    {39265u, 0xFF5Fu}, {39394u, 0xFFE6u}, {189000u, 0x10000u}};

inline constexpr int gb18030_ranges_count = 207;

} // namespace beman::transcoding::detail::tables

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_TABLES_GB18030_RANGES_HPP
