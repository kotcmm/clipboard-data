{
  "targets": [{
    "target_name": "ClipboardData",
    "dependencies" : [
      "gyp/libpng.gyp:libpng"
    ],
    "include_dirs" : [
      "src",
      "modules/clip",
      "modules/lpng",
      "modules/zlib",
      "configs",
      "<!(node -e \"require('nan')\")"
    ],
    "sources": [
      "src/index.cc",
      "modules/clip/clip.cpp",
      "modules/clip/image.cpp"
    ],
    "conditions": [
        ["OS not in ['mac', 'win']", {
            "sources": [ "modules/clip/clip_x11.cpp"],
          "cflags": [
            " -fexceptions"
          ]
        }],

        ["OS=='mac'", {
            "sources": [ "modules/clip/clip_osx.mm"],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
            },
            "cflags": [
              " -fexceptions"
            ]
        }],

        ["OS=='win'", {
            "sources": [ "modules/clip/clip_win.cpp"],
            'msvs_disabled_warnings': [
              4244,  # conversion from 'unsigned long' to 'uint8_t', possible loss of data
            ]
        }]
    ]
  }]
}