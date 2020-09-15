// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <ft2build.h>
#include FT_FREETYPE_H

#include <array>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace glShow
{
namespace impl
{
namespace detail
{
struct DestroyGLFWWindow
{
    void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); }
};
} // namespace detail

class WindowClosedError final : public std::runtime_error
{
  public:
    WindowClosedError() : runtime_error("Window closed.\n") {}
};

class glShow2d
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

    ~glShow2d();

  private:
    struct uivec2;
    struct ivec2;
    struct vec3;
    struct Character;
    struct TextToRender;

    void LoadTexture(unsigned char const* const data, int const width,
                     int const height, int const nChannels);

    GLuint CompileShader(char const* const shaderCode, GLenum const shaderType);

    GLuint LinkProgram(GLuint const vertexShader, GLuint const fragShader);

    void InitGLFWAndGlad();

    void LoadTextureVertexArray();

    void CreateTextureShaderProgram();

    void CreateTextShaderProgram();

    void InitTextRenderer(std::string const& pathToFont);

    void RenderText(TextToRender const& textStuct);

    unsigned mWidth;
    unsigned mHeight;
    std::string mWindowName;
    std::unique_ptr<GLFWwindow, detail::DestroyGLFWWindow> mWindow;
    bool mTextRendererInitialized;
    GLuint mTextureVAO, mTextureVBO, mTextureEBO;
    GLuint mTextureShaderProgram;
    GLuint mImageTexture;
    std::map<char, Character> mCharacters;
    GLuint mTextVAO, mTextVBO;
    GLuint mTextShaderProgram;
    GLuint mAtlasTexture;
    GLuint mAtlasWidth, mAtlasHeight;
    std::vector<TextToRender> mTextsToRender;
};
} // namespace impl
} // namespace glShow
