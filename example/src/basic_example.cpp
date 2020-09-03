#include <chrono>
#include <iostream>
#include <string>

#include "glShow2d.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main()
{
    std::string const fontPath{"D:\\Programming\\OpenGL\\fonts\\bitter.otf"};
    MyDisplay::Display display(800, 600, "new window", fontPath);
    std::string const filename{"C:\\Users\\SNaKeRUBIN\\Desktop\\blue.png"};

    // flip images for stb_image
    stbi_set_flip_vertically_on_load(true);
    // load and generate the texture
    int width, height, nChannels;
    unsigned char* data =
        stbi_load(filename.c_str(), &width, &height, &nChannels, 0);
    if (!data)
    {
        std::cout << "could not load image\n";
        return -1;
    }

    float deltaTime = 0.0f;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop;
    std::chrono::duration<float, std::milli> elapsed;
    while (true)
    {
        start = std::chrono::high_resolution_clock::now();
        try
        {
            std::string fps =
                "FPS: " + std::to_string(static_cast<int>(1000.0f / deltaTime));
            display.DrawText(fps, 10.0f, 540.0f, 1.0f, {1.0f, 1.0f, 1.0f});
            display.Draw(data, width, height, nChannels);
            stop = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration<float, std::milli>(stop - start);
            deltaTime = elapsed.count();
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << '\n';
            return -1;
        }
    }

    return 0;
}
