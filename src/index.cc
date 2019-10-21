#include <node_buffer.h>
#include <nan.h>
#include "clip.h"

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

void buffer_delete_callback(char *data, void *the_vector)
{
    delete reinterpret_cast<std::vector<unsigned char> *>(the_vector);
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

    std::vector<unsigned char> *image = new std::vector<unsigned char>();

    clip::image_spec spec = img.spec();

    for (unsigned long y = 0; y < spec.height; ++y)
    {
        char *p = img.data() + spec.bytes_per_row * y;
        for (unsigned long x = 0; x < spec.width; ++x)
        {
            for (unsigned long byte = 0; byte < spec.bits_per_pixel / 8; ++byte, ++p)
                image->push_back(int((*p) & 0xff));
        }
    }

    args.GetReturnValue().Set(
        Nan::NewBuffer((char *)image->data(),
                       image->size(),
                       buffer_delete_callback,
                       image)
            .ToLocalChecked());
}

void SetImage(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Local<Value> arg0 = args[0];
    if (!node::Buffer::HasInstance(arg0))
    {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(
                isolate, "Invalid argument provided. Must be of type Buffer.", NewStringType::kNormal)
                .ToLocalChecked()));
    }
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "getText", GetText);
    NODE_SET_METHOD(exports, "setText", SetText);
    NODE_SET_METHOD(exports, "getImage", GetImage);
    NODE_SET_METHOD(exports, "setImage", SetImage);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize);

} // namespace ClipboardData