{
  'targets': [
    {
      'target_name': 'addon',
      'sources': [ 
          'addon.cc',
          './hiredis/hiredis.c',
          './hiredis/net.c',
          './hiredis/sds.c',
          './hiredis/read.c',
          './hiredis/async.c'
      ],
      'include_dirs' : ["<!(node -e \"require('nan')\")", "."]
    }
  ]
}
