{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
          "src/binding.cc",
          'src/track.cc',
          'src/jukebox.cc',
          'src/worker.cc'
      ],
      "conditions": [
          [ 'OS=="mac"', {
              "xcode_settings": {
                  'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
                  'OTHER_LDFLAGS': ['-stdlib=libc++'],
                  'MACOSX_DEPLOYMENT_TARGET': '10.7'
              }
            }
          ]
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "libraries": [
          '-lmusly'
      ]
    }
  ]
}