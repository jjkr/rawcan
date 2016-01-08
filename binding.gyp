{
  "targets": [
    {
      "target_name": "rawcan",
      "sources": [ "src/rawcan.cpp" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "cflags": [ "-std=c++14" ]
    }
  ]
}
