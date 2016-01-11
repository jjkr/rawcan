{
  "targets": [
    {
      "target_name": "can_wrap",
      "sources": [ "src/can_wrap.cpp" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "cflags_cc": [ "-std=c++11" ]
    }
  ]
}
