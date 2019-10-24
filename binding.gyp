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
              "<!(pkg-config --cflags libx11-devel)",
              "-Wno-missing-field-initializers",
              "-Wno-deprecated-declarations",
            ],
            "link_settings": {
              "ldflags": [
                "<!(pkg-config --libs-only-L --libs-only-other libx11-devel)",
              ],
              "libraries": [
                "<!(pkg-config --libs-only-l libx11-devel)",
              ],
            }
        }],

        ["OS=='mac'", {
            "sources": [ "modules/clip/clip_osx.mm"]
        }],

        ["OS=='win'", {
            "sources": [ "modules/clip/clip_win.cpp"]
        }]
    ]
  }]
}