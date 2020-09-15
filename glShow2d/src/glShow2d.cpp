#include "glShow2d.h"
#include "glShow2dImpl.h"

class glShow::glShow2d::glShow2dImpl
{
  public:
    glShow2dImpl(unsigned const width, unsigned const height,
                 std::string const& windowName)
        : impl(width, height, windowName)
    {
    }

    glShow2dImpl(unsigned const width, unsigned const height,
                 std::string const& windowName, std::string const& pathToFont)
        : impl(width, height, windowName, pathToFont)
    {
    }

    glShow::impl::glShow2d impl;
};

glShow::glShow2d::glShow2d(unsigned const width, unsigned const height,
                           std::string const& windowName)
    : pImpl_(std::make_unique<glShow2dImpl>(width, height, windowName))
{
}

glShow::glShow2d::glShow2d(unsigned const width, unsigned const height,
                           std::string const& windowName,
                           std::string const& pathToFont)
    : pImpl_(
          std::make_unique<glShow2dImpl>(width, height, windowName, pathToFont))
{
}

glShow::glShow2d::~glShow2d() = default;

void glShow::glShow2d::Draw(unsigned char const* const data, int const width,
                            int const height, int const nChannels)
{
    pImpl().impl.Draw(data, width, height, nChannels);
}

void glShow::glShow2d::DrawText(std::string const& text, float const x,
                                float const y, float const scale,
                                TextColor const& color)
{
    pImpl().impl.DrawText(text, x, y, scale, {color.r, color.g, color.b});
}

void glShow::glShow2d::EnableOrReInitTextRenderer(std::string const& pathToFont)
{
    pImpl().impl.EnableOrReInitTextRenderer(pathToFont);
}
