#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../library/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../library/stb_image/stb_image_write.h"

void saveImage(const char *filename, unsigned char *data, int width, int height, int channels)
{
    stbi_write_png(filename, width, height, channels, data, width * channels);
}

unsigned char *convertToGrayscale(unsigned char *data, int width, int height, int channels)
{
    unsigned char *grayscaleData = (unsigned char *)malloc(width * height);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Calcular el valor promedio de los canales de color
            int sum = 0;
            for (int c = 0; c < channels; ++c)
            {
                sum += data[(y * width + x) * channels + c];
            }
            int average = sum / channels;
            // Asignar el mismo valor a cada canal en la imagen en escala de grises
            grayscaleData[y * width + x] = average;
        }
    }

    return grayscaleData;
}

unsigned char *resizeImage(unsigned char *data, int width, int height, int channels, int newWidth, int newHeight)
{
    unsigned char *resizedData = (unsigned char *)malloc(newWidth * newHeight * channels);

    for (int y = 0; y < newHeight; ++y)
    {
        for (int x = 0; x < newWidth; ++x)
        {
            for (int c = 0; c < channels; ++c)
            {
                // Interpolación simple: promedio de los píxeles vecinos
                resizedData[(y * newWidth + x) * channels + c] =
                    (data[(2 * y * width + 2 * x) * channels + c] +
                     data[(2 * y * width + 2 * x + 1) * channels + c] +
                     data[((2 * y + 1) * width + 2 * x) * channels + c] +
                     data[((2 * y + 1) * width + 2 * x + 1) * channels + c]) /
                    4;
            }
        }
    }

    return resizedData;
}

int main()
{
    int width, height, channels;
    unsigned char *data = stbi_load("test.jpg", &width, &height, &channels, STBI_rgb);

    if (data == NULL)
    {
        printf("Error: %s\n", stbi_failure_reason());
        return 1;
    }

    printf("Original - Width: %d, Height: %d, Channels: %d\n", width, height, channels);

    // Reducir el tamaño de la imagen
    int newWidth = width / 2;
    int newHeight = height / 2;
    unsigned char *resizedData = resizeImage(data, width, height, channels, newWidth, newHeight);
    printf("Resized - Width: %d, Height: %d, Channels: %d\n", newWidth, newHeight, channels);

    // Convertir la imagen a escala de grises
    unsigned char *grayscaleData = convertToGrayscale(data, width, height, channels);
    printf("Converted to Grayscale\n");

    // Guardar la imagen en escala de grises
    saveImage("grayscale_image.png", grayscaleData, width, height, 1); // 1 canal para la escala de grises

    // Guardar la imagen redimensionada
    saveImage("resized_image.png", resizedData, newWidth, newHeight, channels);

    // Liberar la memoria de la imagen original
    stbi_image_free(data);

    // Liberar la memoria de la imagen redimensionada
    free(resizedData);

    return 0;
}
