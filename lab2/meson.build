project(
  'lab2',
  'c',
  default_options : [
    'buildtype=plain',
    'debug=false',
    'optimization=2',
    'c_std=gnu2x',
    'b_pie=true',
  ]
)

lab2_sources = files(
  'lab2.c',
  'clock.c', 
  'fcyc.c',
  'implementation.c',
  'implementation_reference.c',
  'utilities.c',
)

executable('lab2', lab2_sources)
