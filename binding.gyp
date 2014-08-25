{
  "targets": [
    {
      "target_name": "hipack",
      "sources": [
      	"./pack.cc"
      ]
    }
  ],
  'include_dirs': ["<!(node -e \"require('nan')\")"],
  'cflags': [
	  '-Wall',
	  '-O3'
  ]
}