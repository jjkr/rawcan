{
  "targets": [
    {
      "target_name": "rawcan",
      "sources": [ "rawcan.cpp" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "cflags": [ "-std=c++14" ]
    }
  ]
}
