{
  "targets": [
    {
      "target_name": "rawcan",
      "sources": [ "src/rawcan.cpp", "src/raw-can-socket.cpp" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "cflags": [ "-std=c++14" ]
    }
  ]
}
