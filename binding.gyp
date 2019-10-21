{
  "targets": [{
    "target_name": "ClipboardData",
    "include_dirs" : [
      "src",
      "modules/clip",
      "<!(node -e \"require('nan')\")"
    ],
    "sources": [
      "src/index.cc",
      "modules/clip/clip.cpp",
      "modules/clip/image.cpp"
    ],
    "conditions": [
        ["OS=='linux'", {
            "sources": [ "modules/clip/clip_x11.cpp"]
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