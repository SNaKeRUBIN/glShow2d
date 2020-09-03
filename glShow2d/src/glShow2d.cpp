#include "glShow2d.h"

namespace
{
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

constexpr char* GetTextureVertexShader()
{
    return R"(
        #version 330 core
        layout(location = 0) in vec2 vertex;
        layout(location = 1) in vec2 texCoords;

        out vec2 oTexCoords;

        void main()
        {
            gl_Position = vec4(vertex.xy, 0.0, 1.0);
            oTexCoords = texCoords;
        }
        )";
}

constexpr char* GetTextureFragShader()
{
    return R"(
        #version 330 core
        in vec2 oTexCoords;
        out vec4 FragColor;

        uniform sampler2D tex;

        void main()
        {
            FragColor = texture(tex, oTexCoords);
        }
        )";
}

constexpr char* GetTextVertexShader()
{
    return R"(
        #version 330 core
        layout (location = 0) in vec2 vertex;
        layout (location = 1) in vec2 texCoords;

        out vec2 oTexCoords;

        uniform mat4 projection;

        void main()
        {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
            oTexCoords = texCoords;
        }
        )";
}

constexpr char* GetTextFragShader()
{
    return R"(
        #version 330 core
        in vec2 oTexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec3 textColor;

        void main()
        {
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, oTexCoords).r);
            
            color = vec4(textColor, 1.0) * sampled;
        }
        )";
}
} // namespace

struct MyDisplay::Display::uivec2
{
    unsigned x, y;
};

struct MyDisplay::Display::ivec2
{
    int x, y;
};

struct MyDisplay::Display::vec3
{
    float r, g, b;
};

struct MyDisplay::Display::Character
{
    uivec2 Size;   // Size of glyph
    ivec2 Bearing; // Offset from baseline to left/top of glyph
    ivec2 Advance; // Offset to advance to next glyph
    uivec2 Offset; // Offset to char in tex atlas in range [0.0, 1.0]
};

struct MyDisplay::Display::TextToRender
{
    std::string text;
    float x, y;
    float scale;
    vec3 color;
};

MyDisplay::Display::Display(unsigned const width, unsigned const height,
                            std::string const& windowName)
    : mWidth{width}, mHeight{height}, mWindowName{windowName},
      mTextRendererInitialized{false}
{
    InitGLFWAndGlad();

    LoadTextureVertexArray();
    CreateTextureShaderProgram();
}

MyDisplay::Display::Display(unsigned const width, unsigned const height,
                            std::string const& windowName,
                            std::string const& pathToFont)
    : mWidth{width}, mHeight{height}, mWindowName{windowName},
      mTextRendererInitialized{true}
{
    InitGLFWAndGlad();

    LoadTextureVertexArray();
    CreateTextureShaderProgram();

    InitTextRenderer(pathToFont);
    CreateTextShaderProgram();
}

void MyDisplay::Display::EnableOrReInitTextRenderer(
    std::string const& pathToFont)
{
    InitTextRenderer(pathToFont);
}

MyDisplay::Display::~Display()
{
    glDeleteVertexArrays(1, &mTextureVAO);
    glDeleteBuffers(1, &mTextureVBO);
    glDeleteBuffers(1, &mTextureEBO);

    glDeleteVertexArrays(1, &mTextVAO);
    glDeleteBuffers(1, &mTextVBO);

    glDeleteTextures(1, &mImageTexture);

    glfwTerminate();
}

void MyDisplay::Display::Draw(unsigned char const* const data, int const width,
                              int const height, int const nChannels)
{
    if (!glfwWindowShouldClose(mWindow.get()))
    {
        LoadTexture(data, width, height, nChannels);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mTextureShaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mImageTexture);

        glBindVertexArray(mTextureVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        for (auto const& text : mTextsToRender)
        {
            RenderText(text);
        }
        mTextsToRender.clear();

        glfwSwapBuffers(mWindow.get());
        glfwPollEvents();
    }
    else
    {
        throw WindowClosedError();
    }
}

void MyDisplay::Display::DrawText(std::string const& text, float const x,
                                  float const y, float const scale,
                                  TextColor const& color)
{
    if (!mTextRendererInitialized)
    {
        std::cout << "Text renderer not initialized\n";
    }
    else
    {
        mTextsToRender.push_back(
            {text, x, y, scale, vec3{color.r, color.g, color.b}});
    }
}

void MyDisplay::Display::LoadTexture(unsigned char const* const data,
                                     int const width, int const height,
                                     int const nChannels)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mImageTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum const format = [nChannels]() {
        if (nChannels == 1)
            return GL_RED;
        else if (nChannels == 3)
            return GL_RGB;
        else if (nChannels == 4)
            return GL_RGBA;
        else
            return GL_FALSE;
    }();

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint MyDisplay::Display::CompileShader(char const* const shaderCode,
                                         GLenum const shaderType)
{
    GLuint const shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        const std::string shaderTypeStr = [shaderType]() {
            if (shaderType == GL_VERTEX_SHADER)
            {
                return "VERTEX";
            }
            else if (shaderType == GL_FRAGMENT_SHADER)
            {
                return "FRAGMENT";
            }
            else
            {
                return "UNKNOWN";
            }
        }();

        std::basic_string<GLchar> infoLog;
        constexpr GLsizei logLength{512};
        infoLog.reserve(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, infoLog.data());
        std::cout << "ERROR::SHADER::" << shaderTypeStr.c_str()
                  << "::COMPILATION_FAILED\n"
                  << infoLog.c_str() << '\n';
    }

    return shader;
}

GLuint MyDisplay::Display::LinkProgram(GLuint const vertexShader,
                                       GLuint const fragShader)
{
    GLuint const shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        std::basic_string<GLchar> infoLog;
        constexpr GLsizei logLength{512};
        infoLog.reserve(logLength);
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog.data());
        std::cout << "ERROR::SHADER::LINKAGE_FAILED\n"
                  << infoLog.c_str() << '\n';
    }

    return shaderProgram;
}

void MyDisplay::Display::InitGLFWAndGlad()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    mWindow.reset(
        glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), NULL, NULL));
    if (mWindow == nullptr)
    {
        std::cout << "failed to create a window\n";
        glfwTerminate();
    }
    glfwMakeContextCurrent(mWindow.get());
    glfwSetFramebufferSizeCallback(mWindow.get(), framebuffer_size_callback);
    glfwSetInputMode(mWindow.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize glad\n";
    }
    glfwSwapInterval(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyDisplay::Display::LoadTextureVertexArray()
{
    std::array<float, 4 * 4> const vertices = {
        // clang-format off
            // positions  // tex-coords
            1.0f, 1.0f,   1.0f, 1.0f,   // top right
            1.0f, -1.0f,  1.0f, 0.0f,   // bottom right
            -1.0f, -1.0f, 0.0f, 0.0f,   // bottom left
            -1.0f, 1.0f,  0.0f, 1.0f,   // top left
        // clang-format on
    };
    std::array<unsigned, 3 * 2> const indices = {
        // clang-format off
            0, 1, 2,    // first triangle
            0, 2, 3     // second triangle
        // clang-format on
    };
    glGenVertexArrays(1, &mTextureVAO);
    glGenBuffers(1, &mTextureVBO);
    glGenBuffers(1, &mTextureEBO);

    using VertexType = decltype(vertices)::value_type;

    glBindVertexArray(mTextureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mTextureVBO);
    glBufferData(GL_ARRAY_BUFFER, std::size(vertices) * sizeof(VertexType),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTextureEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 std::size(indices) * sizeof(decltype(indices)::value_type),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(VertexType),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(VertexType),
                          (void*)(2 * sizeof(VertexType)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &mImageTexture);
}

void MyDisplay::Display::CreateTextureShaderProgram()
{
    constexpr char* vShaderCode = GetTextureVertexShader();
    constexpr char* fShaderCode = GetTextureFragShader();

    GLuint const vertexShader = CompileShader(vShaderCode, GL_VERTEX_SHADER);
    GLuint const fragShader = CompileShader(fShaderCode, GL_FRAGMENT_SHADER);

    mTextureShaderProgram = LinkProgram(vertexShader, fragShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    glUseProgram(mTextureShaderProgram);
    glUniform1i(glGetUniformLocation(mTextureShaderProgram, "tex"), 0);
}

void MyDisplay::Display::CreateTextShaderProgram()
{
    constexpr char* vShaderCode = GetTextVertexShader();
    constexpr char* fShaderCode = GetTextFragShader();

    GLuint const vertexShader = CompileShader(vShaderCode, GL_VERTEX_SHADER);
    GLuint const fragShader = CompileShader(fShaderCode, GL_FRAGMENT_SHADER);

    mTextShaderProgram = LinkProgram(vertexShader, fragShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    glUseProgram(mTextShaderProgram);
    std::array<float, 4 * 4> const projectionText2 = {
        // clang-format off
        0.002500f, 0.000000f, 0.000000f, 0.000000f,
        0.000000f, 0.003333f, 0.000000f, 0.000000f,
        0.000000f, 0.000000f, -1.000000f, 0.000000f,
        -1.000000f, -1.000000f, 0.000000f, 1.000000f
        // clang-format on
    };
    glUniformMatrix4fv(glGetUniformLocation(mTextShaderProgram, "projection"),
                       1, GL_FALSE, projectionText2.data());
}

void MyDisplay::Display::InitTextRenderer(std::string const& pathToFont)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE::COULD_NOT_INIT_FREETYPE_LIBRARY\n";
    }

    FT_Face face;
    if (FT_New_Face(ft, pathToFont.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE::FAILED_TO_LOAD_FONT\n";
    }

    // set size to load glyphs
    FT_Set_Pixel_Sizes(face, 0, 40);

    unsigned int totalWidth = 0;
    unsigned int maxHeight = 0;

    for (unsigned int c = 32; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE::FAILED_TO_LOAD_GLYPH\n";
            continue;
        }
        totalWidth += face->glyph->bitmap.width;
        maxHeight = std::max(face->glyph->bitmap.rows, maxHeight);
    }

    mAtlasHeight = maxHeight;
    mAtlasWidth = totalWidth;

    glActiveTexture(GL_TEXTURE0);
    glDeleteTextures(1, &mAtlasTexture);
    glGenTextures(1, &mAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, mAtlasTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, totalWidth, maxHeight, 0, GL_RED,
                 GL_UNSIGNED_BYTE, 0);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Clamping to edges is important to prevent artifacts when scaling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Linear filtering usually looks best for text */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned xoffset = 0;
    for (unsigned int c = 32; c < 128; ++c)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE::FAILED_TO_LOAD_GLYPH\n";
            continue;
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, 0, face->glyph->bitmap.width,
                        face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer);
        Character temp = {{face->glyph->bitmap.width, face->glyph->bitmap.rows},
                          {face->glyph->bitmap_left, face->glyph->bitmap_top},
                          {face->glyph->advance.x, face->glyph->advance.y},
                          {xoffset, face->glyph->bitmap.rows}};
        mCharacters.insert(std::pair<char, Character>(
            c, {{face->glyph->bitmap.width, face->glyph->bitmap.rows},
                {face->glyph->bitmap_left, face->glyph->bitmap_top},
                {face->glyph->advance.x, face->glyph->advance.y},
                {xoffset, face->glyph->bitmap.rows}}));
        xoffset += face->glyph->bitmap.width + 1;
    }

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenBuffers(1, &mTextVBO);
    glGenVertexArrays(1, &mTextVAO);
}

void MyDisplay::Display::RenderText(TextToRender const& textStuct)
{
    glUseProgram(mTextShaderProgram);
    glUniform3f(glGetUniformLocation(mTextShaderProgram, "textColor"),
                textStuct.color.r, textStuct.color.g, textStuct.color.b);

    float cur_x = textStuct.x;
    std::vector<float> allVertices(textStuct.text.size() * 6 * 4);
    for (size_t i = 0; i < textStuct.text.size(); ++i)
    {
        const Character& ch = mCharacters.at(textStuct.text[i]);

        float xpos = cur_x + ch.Bearing.x * textStuct.scale;
        float ypos = textStuct.y - (ch.Size.y - ch.Bearing.y) * textStuct.scale;

        float w = ch.Size.x * textStuct.scale;
        float h = ch.Size.y * textStuct.scale;
        // update VBO for each character

        std::vector<float> temp = {
            // clang-format off
                // position         // texture coords
                xpos, ypos + h,     (float)ch.Offset.x / mAtlasWidth, 0.0f,
                xpos, ypos,         (float)ch.Offset.x / mAtlasWidth, (float)ch.Offset.y / mAtlasHeight,
                xpos + w, ypos,     (float)(ch.Offset.x + ch.Size.x) / mAtlasWidth, (float)ch.Offset.y / mAtlasHeight,
                xpos, ypos + h,     (float)ch.Offset.x / mAtlasWidth, 0.0f,
                xpos + w, ypos,     (float)(ch.Offset.x + ch.Size.x) / mAtlasWidth, (float)ch.Offset.y / mAtlasHeight,
                xpos + w, ypos + h, (float)(ch.Offset.x + ch.Size.x) / mAtlasWidth, 0.0f
            // clang-format on
        };

        std::copy(temp.cbegin(), temp.cend(), allVertices.begin() + 6 * 4 * i);

        // now advance cursors for next glyph (note that advance is
        // number of 1/64 pixels)
        cur_x += (ch.Advance.x >> 6) * textStuct.scale;
    }

    // load glyph atlas
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mAtlasTexture);

    // update content of VBO memory
    glBindVertexArray(mTextVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mTextVBO);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
                 allVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // render quads
    glDrawArrays(GL_TRIANGLES, 0, 6 * (GLsizei)textStuct.text.size());

    // unbind all buffers
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
