#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char type[2];
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t off_bits;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t size;           // Размер этой структуры в байтах
    int32_t width;           // Ширина изображения в пикселях
    int32_t height;          // Высота изображения в пикселях
    uint16_t planes;         // Количество плоскостей (обычно 1)
    uint16_t bit_count;      // Количество бит на пиксель (например, 24 или 32)
    uint32_t compression;    // Метод сжатия (обычно 0 для отсутствия сжатия)
    uint32_t size_image;     // Размер данных изображения в байтах
    int32_t x_pels_per_meter; // Горизонтальное разрешение в пикселях на метр
    int32_t y_pels_per_meter; // Вертикальное разрешение в пикселях на метр
    uint32_t clr_used;       // Количество используемых цветов
    uint32_t clr_important;  // Количество важных цветов
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
   if (!out.is_open()) {
       return false;
   }

    int indent = GetBMPStride(image.GetWidth());

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    file_header.type[0] = 'B';
    file_header.type[1] = 'M';
    file_header.size = sizeof(file_header) + sizeof(info_header) + (indent * image.GetHeight());
    file_header.reserved1 = 0;
    file_header.reserved2 = 0;
    file_header.off_bits = sizeof(file_header) + sizeof(info_header);

    info_header.size = sizeof(info_header);           
    info_header.width = image.GetWidth();          
    info_header.height = image.GetHeight();          
    info_header.planes = 1;         
    info_header.bit_count = 24;
    info_header.compression = 0;
    info_header.size_image = indent * image.GetHeight();
    info_header.x_pels_per_meter = 11811; 
    info_header.y_pels_per_meter = 11811; 
    info_header.clr_used = 0;
    info_header.clr_important = 0x1000000;  

    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    std::vector<char> buff(indent, 0);

    for (int y = info_header.height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < info_header.width; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), indent);
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);

    BitmapFileHeader file_header;
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    if (!ifs || file_header.type[0] != 'B' || file_header.type[1] != 'M') {
        return {};
    }

    BitmapInfoHeader info_header;
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    int indent = GetBMPStride(info_header.width);
    Image result(info_header.width, info_header.height, Color::Black());
    std::vector<char> buff(indent);

    for (int y = info_header.height - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), indent);

        for (int x = 0; x < info_header.width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib