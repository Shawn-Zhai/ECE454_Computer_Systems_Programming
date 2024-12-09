project(
  'lab4',
  'cpp',
  default_options : [
    'buildtype=plain',
    'debug=true',
    'optimization=2',
    'cpp_std=gnu++23',
    'b_pie=true',
  ]
)

cpp_args = ['-fgnu-tm']
link_args = ['-litm']  # Link the transactional memory runtime library

executable('randtrack',
  'randtrack.cpp',
  cpp_args : cpp_args,
  link_args : link_args
)
