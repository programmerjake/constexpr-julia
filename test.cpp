/**
 * Copyright (c) 2017 Jacob Lifshay
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
namespace std
{
    typedef decltype(sizeof(0)) size_t;
    typedef __UINT8_TYPE__ uint8_t;
    typedef __UINT16_TYPE__ uint16_t;
    typedef __UINT32_TYPE__ uint32_t;
    typedef __UINT64_TYPE__ uint64_t;
    typedef __INT8_TYPE__ int8_t;
    typedef __INT16_TYPE__ int16_t;
    typedef __INT32_TYPE__ int32_t;
    typedef __INT64_TYPE__ int64_t;
}

struct Color final
{
    unsigned char r, g, b;
};

constexpr unsigned maxCount = 0xFF;

constexpr Color rgbF(float r, float g, float b) noexcept
{
    if(r < 0)
        r = 0;
    else if(!(r < 1))
        r = 1;
    if(g < 0)
        g = 0;
    else if(!(g < 1))
        g = 1;
    if(b < 0)
        b = 0;
    else if(!(b < 1))
        b = 1;
    r *= 0x100;
    g *= 0x100;
    b *= 0x100;
    if(r > 0xFF)
        r = 0xFF;
    if(g > 0xFF)
        g = 0xFF;
    if(b > 0xFF)
        b = 0xFF;
    int rI = r, gI = g, bI = b;
    return Color{static_cast<unsigned char>(rI), static_cast<unsigned char>(gI), static_cast<unsigned char>(bI)};
}

constexpr Color mapColor(unsigned count) noexcept
{
    float v = count;
    v /= maxCount;
    return rgbF(v, v * 6, v > 0.5 ? 2 - 2 * v : 2 * v);
}

constexpr unsigned char juliaCore(float zx, float zy, float cx, float cy) noexcept
{
#if 1
    for(std::size_t i = 0; i < maxCount; i++)
    {
        if(zx * zx + zy * zy > 4)
            return i;
        float x = zx * zx - zy * zy + cx;
        float y = zx * zy * 2 + cy;
        zx = x;
        zy = y;
    }
    return maxCount;
#else
    return static_cast<long long>(zx * 0xFF + 0x80);
#endif
}

template <std::size_t xSize, std::size_t ySize>
constexpr unsigned char juliaCoreI(int x, int y, float cx, float cy) noexcept
{
    return juliaCore((float)x / (xSize - 1) * 3.0f - 1.5f, 1.5f - (float)y / (ySize - 1) * 3.0f, cx, cy);
}

template <std::size_t XSize, std::size_t YSize>
struct Image final
{
    Color data[XSize * YSize];
};

template <std::size_t XSize, std::size_t YSize>
constexpr Image<XSize, YSize> julia(float cx, float cy) noexcept
{
    Image<XSize, YSize> retval{};
    for(std::size_t y = 0; y < YSize; y++)
    {
        for(std::size_t x = 0; x < XSize; x++)
        {
            retval.data[x + XSize * y] = mapColor(juliaCoreI<XSize, YSize>(x, y, cx, cy));
        }
    }
    return retval;
}

template <std::size_t N>
struct constexpr_string final
{
    char data[N + 1] = {};
    constexpr std::size_t size() const noexcept
    {
        return N;
    }
    constexpr std::size_t length() const noexcept
    {
        return N;
    }
    constexpr const char *begin() const noexcept
    {
        return data;
    }
    constexpr const char *end() const noexcept
    {
        return data + N;
    }
    constexpr char *begin() noexcept
    {
        return data;
    }
    constexpr char *end() noexcept
    {
        return data + N;
    }
    constexpr char &operator[](std::size_t index) noexcept
    {
        return data[index];
    }
    constexpr const char &operator[](std::size_t index) const noexcept
    {
        return data[index];
    }
    constexpr operator const char *() const noexcept
    {
        return data;
    }
};

template <char...chars>
constexpr constexpr_string<sizeof...(chars)> operator ""_cs() noexcept
{
    return {{chars...}};
}

template <std::size_t N1, std::size_t N2>
constexpr constexpr_string<N1 + N2> operator+(const constexpr_string<N1> &a, const constexpr_string<N2> &b) noexcept
{
    constexpr_string<N1 + N2> retval;
    char *p = retval.data;
    for(char v : a)
        *p++ = v;
    for(char v : b)
        *p++ = v;
    return retval;
}

constexpr char base64Digit(unsigned v) noexcept
{
    v &= 0x3F;
    if(v < 26)
        return 'A' + v;
    if(v < 52)
        return 'a' + v - 26;
    if(v < 62)
        return '0' + v - 52;
    if(v < 63)
        return '+';
    return '/';
}

template <std::size_t N>
constexpr auto toBase64Base(const constexpr_string<N> &input) noexcept
{
    static_assert(N % 3 == 0);
    constexpr_string<N * 4 / 3> retval;
    for(std::size_t i = 0, j = 0; i < input.size(); i += 3, j += 4)
    {
        unsigned char ch1 = input[i];
        unsigned char ch2 = input[i + 1];
        unsigned char ch3 = input[i + 2];
        unsigned v = ch1;
        v <<= 8;
        v |= ch2;
        v <<= 8;
        v |= ch3;
        retval[j + 3] = base64Digit(v);
        v >>= 6;
        retval[j + 2] = base64Digit(v);
        v >>= 6;
        retval[j + 1] = base64Digit(v);
        v >>= 6;
        retval[j] = base64Digit(v);
    }
    return retval;
}

template <std::size_t N>
constexpr auto toBase64(const constexpr_string<N> &input) noexcept
{
    if constexpr(N % 3 == 1)
    {
        auto retval = toBase64Base(input + constexpr_string<2>{});
        retval.data[retval.size() - 1] = '=';
        retval.data[retval.size() - 2] = '=';
        return retval;
    }
    else if constexpr(N % 3 == 2)
    {
        auto retval = toBase64Base(input + constexpr_string<1>{});
        retval.data[retval.size() - 1] = '=';
        return retval;
    }
    else
    {
        return toBase64Base(input);
    }
}

template <std::size_t Value>
constexpr auto toDecimal() noexcept
{
    if constexpr(Value >= 10)
        return toDecimal<Value / 10>() + constexpr_string<1>{{(char)('0' + Value % 10)}};
    else
        return constexpr_string<1>{{(char)('0' + Value % 10)}};
}

constexpr constexpr_string<1> u8String(unsigned char v) noexcept
{
    return {{static_cast<char>(v)}};
}

constexpr constexpr_string<2> u16String(std::uint16_t v) noexcept
{
    return u8String(v) + u8String(v >> 8);
}

constexpr constexpr_string<4> u32String(std::uint32_t v) noexcept
{
    return u16String(v) + u16String(v >> 16);
}

constexpr constexpr_string<8> u64String(std::uint64_t v) noexcept
{
    return u32String(v) + u32String(v >> 32);
}

template <std::size_t XSize, std::size_t YSize>
constexpr auto imageToBMPString(const Image<XSize, YSize> &input) noexcept
{
    constexpr auto bmMagic = constexpr_string<2>{{'B', 'M'}};
    constexpr std::size_t headerSize = 0x36U;
    constexpr std::size_t bytesPerPixel = 3;
    constexpr std::size_t unroundedLineSize = bytesPerPixel * XSize;
    constexpr std::size_t lineSize = (unroundedLineSize + 3) & -4;
    constexpr std::size_t imageDataSize = lineSize * YSize;
    constexpr std::size_t imageSize = headerSize + imageDataSize;
    constexpr int reserved = 0;
    constexpr std::uint16_t planeCount = 1;
    constexpr std::uint16_t bitsPerPixel = 24;
    constexpr std::uint32_t dpi96 = 3622;
    constexpr auto bmpBaseHeader = bmMagic
                                 + u32String(imageSize)
                                 + u32String(reserved)
                                 + u32String(headerSize);
    constexpr std::size_t bmpInfoHeaderSize = 0x28;
    constexpr auto bmpInfoHeader = u32String(0x28)
                                 + u32String(XSize)
                                 + u32String(YSize)
                                 + u16String(planeCount)
                                 + u16String(bitsPerPixel)
                                 + u32String(0 /*BI_RGB*/)
                                 + u32String(imageDataSize)
                                 + u32String(dpi96)
                                 + u32String(dpi96)
                                 + u32String(0)
                                 + u32String(0);
    static_assert(bmpInfoHeader.size() == bmpInfoHeaderSize);
    constexpr auto bmpHeader = bmpBaseHeader + bmpInfoHeader;
    static_assert(bmpHeader.size() == headerSize);
    constexpr_string<imageSize> retval{};
    for(std::size_t i = 0; i < bmpHeader.size(); i++)
        retval[i] = bmpHeader[i];
    char *imageData = &retval[bmpHeader.size()];
    for(std::size_t i = 0, y = YSize - 1; i < YSize; i++, y--)
    {
        char *rowData = &imageData[i * lineSize];
        for(std::size_t x = 0; x < XSize; x++)
        {
            auto pixel = input.data[x + XSize * y];
            *rowData++ = pixel.b;
            *rowData++ = pixel.g;
            *rowData++ = pixel.r;
        }
    }
    return retval;
}

const char *fn()
{
    constexpr auto image = julia<256, 256>(-0.8, 0.156);
    static constexpr auto retval = toBase64(imageToBMPString(image));
    return retval;
}
