#include <memory>
#include <string>

namespace glShow
{
class glShow2d final
{
  public:
    glShow2d(unsigned const width, unsigned const height,
             std::string const& windowName);

    glShow2d(unsigned const width, unsigned const height,
             std::string const& windowName, std::string const& pathToFont);

    void Draw(unsigned char const* const data, int const width,
              int const height, int const nChannels);

    struct TextColor
    {
        float r, g, b;
    };
    void DrawText(std::string const& text, float const x, float const y,
                  float const scale, TextColor const& color);

    void EnableOrReInitTextRenderer(std::string const& pathToFont);

    ~glShow2d() noexcept;

  private:
    class glShow2dImpl;
    std::unique_ptr<glShow2dImpl> pImpl_;
    glShow2dImpl& pImpl() { return *pImpl_; }
    glShow2dImpl const& pImpl() const { return *pImpl_; }
};
} // namespace glShow
