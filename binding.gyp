{
  "targets": [
    {
      "target_name": "CustomAddon",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ 
        "addon.cc", 
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")",
      ],
      "conditions": [
        ["OS=='mac'", {
          "cflags+": [
            "-fvisibility=hidden"
          ],
          "xcode_settings": {
            "OTHER_CFLAGS": [
              "-fvisibility=hidden",
            ],
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "12"
          },
        }],
        ['OS=="win"', {
          "defines": [
            "_HAS_EXCEPTIONS=1"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            },
          },
        }]
      ]
    }
  ]
}
