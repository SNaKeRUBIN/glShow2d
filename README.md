# glShow2d
This static library allows you to display a 2d image in a window. Also, text can be rendered using 2d.

## Usage
Create a object specifying the render size.
```cpp
glShow::glShow2d display(800, 600, "window_name", fontPath);
```

Draw an 2d image.
```cpp
display.Draw(imageData, width, height, nChannels);
```

Draw text on the image.
```cpp
display.DrawText("some_text", 10.0f, 540.0f, 1.0f, {1.0f, 1.0f, 1.0f});
```

## Build
Run CMake with install target.
Other dependencies will be downloaded automatically.

```bash
cmake -Bbuild -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=install
cmake --build build --config Release --target install
```

## Linking with CMake
The project will output CMake config that can be found using CMake's find_package.
```cmake
find_package(glShow2d 
    REQUIRED
    PATHS ${path_to_glShow2d_install}
    NO_DEFAULT_PATH
)
```

Just link the target with your target using target_link_libraries.
```cmake
target_link_libraries(some_target glShow2d)
```

## Notice
Tested on Visual Studio 15 2017.
