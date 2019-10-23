#include "nan.h"
#include "clip.h"
#include "png.h"
#include "zlib.h"

namespace ClipboardData
{

using v8::Context;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

void GetText(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    std::string value;
    clip::get_text(value);
    args.GetReturnValue().Set(String::NewFromUtf8(
                                  isolate, value.c_str(), NewStringType::kNormal)
                                  .ToLocalChecked());
}

void SetText(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    if (args.Length() < 1 || !args[0]->IsString())
    {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(
                isolate, "Invalid argument provided. Must be of type string.", NewStringType::kNormal)
                .ToLocalChecked()));
        return;
    }

    Local<Context> context = args.GetIsolate()->GetCurrentContext();
    String::Utf8Value str(isolate, args[0]);
    std::string cppStr(*str);
    clip::set_text(cppStr);
}

void GetImage(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    if (!clip::has(clip::image_format()))
    {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(
                isolate, "Clipboard doesn't contain an image.", NewStringType::kNormal)
                .ToLocalChecked()));
    }

    clip::image img;
    if (!clip::get_image(img))
    {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(
                isolate, "Error getting image from clipboard.", NewStringType::kNormal)
                .ToLocalChecked()));
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
    args.GetReturnValue().Set(Nan::CopyBuffer(reinterpret_cast<char *>(&encoded[0]), encoded.size()).ToLocalChecked());
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "getText", GetText);
    NODE_SET_METHOD(exports, "setText", SetText);
    NODE_SET_METHOD(exports, "getImage", GetImage);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize);

} // namespace ClipboardData