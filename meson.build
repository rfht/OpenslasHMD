project(
	'OpenslasHMD', 'c',
	default_options : 'c_std=c11',
	version : '0.1.0',
	meson_version : '>= 0.49'
)

sources = [
	'gl.c',
	'maploader.c',
	'main.c',
]

cc = meson.get_compiler('c')
libm = cc.find_library('m', required : false)

deps = [
	libm,
	dependency('sdl2'),
	dependency('SDL2_mixer'),
	dependency('glew'),
	dependency('gl'),
	dependency('openhmd'),
	dependency('json-c'),
]

openhmd_lib = executable('OpenslasHMD', sources, dependencies : deps, install : true)
