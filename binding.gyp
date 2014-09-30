{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
          "src/binding.cc",
          'src/track.cc',
          'src/jukebox.cc'
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