#include <iostream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image/stb_image_write.h"

void loadImage(const std::string &imagePath, int &width, int &height, int &channels, unsigned char *&data)
{
    data = stbi_load(imagePath.c_str(), &width, &height, &channels, STBI_rgb);
    if (data == NULL)
    {
        std::cerr << "Error: " << stbi_failure_reason() << std::endl;
        exit(1);
    }

    std::cout << "Image loaded - Width: " << width << ", Height: " << height << ", Channels: " << channels << std::endl;
}

void saveImage(const char *filename, unsigned char *data, int width, int height, int channels)
{
    stbi_write_png(filename, width, height, channels, data, width * channels);
}