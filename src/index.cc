#include "nan.h"
#include "clip.h"
#include "png.h"
#include "zlib.h"

namespace ClipboardData
{

NAN_METHOD(GetText)
{
    std::string value;
    clip::get_text(value);
    info.GetReturnValue().Set(Nan::New(value).ToLocalChecked());
}

NAN_METHOD(SetText)
{
    if (info.Length() < 1 || !info[0]->IsString())
    {
        Nan::ThrowTypeError("Invalid argument provided. Must be of type string.");
        return;
    }

    Nan::Utf8String str(info[0]);
    std::string cppStr(*str);
    clip::set_text(cppStr);
}

NAN_METHOD(GetImage)
{
    if (!clip::has(clip::image_format()))
    {
        Nan::ThrowTypeError("Clipboard doesn't contain an image.");
    }

    clip::image img;
    if (!clip::get_image(img))
    {
        Nan::ThrowError("Error getting image from clipboard.");
    }

    auto errorHandler = [](png_structp pngPtr, png_const_charp message) {
        Nan::ThrowError(message);
    };
    auto warningHandler = [](png_structp pngPtr, png_const_charp message) {};

    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, errorHandler, warningHandler);
    if (!pngPtr)
    {
        Nan::ThrowError("Unable to initialize libpng for writing.");
        return;
    }

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        Nan::ThrowError("Unable to initialize libpng info struct.");
        return;
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        Nan::ThrowTypeError("Error encoding PNG.");
        return;
    }

    std::vector<uint8_t> encoded;

    png_set_write_fn(pngPtr, &encoded, [](png_structp pngPtr, png_bytep data, png_size_t length) {
        auto encoded = reinterpret_cast<std::vector<uint8_t> *>(png_get_io_ptr(pngPtr));
        encoded->insert(encoded->end(), data, data + length); }, nullptr);

    png_set_compression_level(pngPtr, Z_BEST_COMPRESSION);

    auto spec = img.spec();
    auto width = spec.width;
    auto height = spec.height;
    auto colorType = (spec.alpha_mask ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB);

    png_set_IHDR(pngPtr, infoPtr, width, height, 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(pngPtr, infoPtr);

    png_bytep row =
        (png_bytep)png_malloc(pngPtr, png_get_rowbytes(pngPtr, infoPtr));

    for (png_uint_32 y = 0; y < spec.height; ++y)
    {
        const uint32_t *src =
            (const uint32_t *)(((const uint8_t *)img.data()) + y * spec.bytes_per_row);
        uint8_t *dst = row;
        unsigned int x, c;

        for (x = 0; x < spec.width; x++)
        {
            c = *(src++);
            *(dst++) = (c & spec.red_mask) >> spec.red_shift;
            *(dst++) = (c & spec.green_mask) >> spec.green_shift;
            *(dst++) = (c & spec.blue_mask) >> spec.blue_shift;
            if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
                *(dst++) = (c & spec.alpha_mask) >> spec.alpha_shift;
        }

        png_write_rows(pngPtr, &row, 1);
    }
    png_free(pngPtr, row);
    png_write_end(pngPtr, nullptr);
    png_destroy_write_struct(&pngPtr, &infoPtr);
    info.GetReturnValue().Set(Nan::CopyBuffer(reinterpret_cast<char *>(&encoded[0]), encoded.size()).ToLocalChecked());
}

NAN_MODULE_INIT(Init)
{
    Nan::SetMethod(target, "getText", GetText);
    Nan::SetMethod(target, "setText", SetText);
    Nan::SetMethod(target, "getImage", GetImage);
}

#if NODE_MAJOR_VERSION >= 10
NAN_MODULE_WORKER_ENABLED(NODE_GYP_MODULE_NAME, Init)
#else
NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
#endif

} // namespace ClipboardData
