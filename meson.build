project(
    'AIEdit',
    'cpp', 
    version: '0.1', 
    license: 'GPL-3', 
    default_options: ['cpp_std=c++17', 'werror=true']
)

compiler = meson.get_compiler('cpp')

dependencies = [
    dependency('openmp'),
    compiler.find_library('btllib')
]

include_dirs = [
    include_directories('include'),
    include_directories('vendor/argparse/include'),
    include_directories('vendor/json/single_include')
]

sources = run_command('scripts/list_sources.sh', check: true).stdout().strip().split()

executable(
    'ai-edit',
    sources,
    dependencies: dependencies,
    include_directories: include_dirs,
    install: true
)

# subdir('tests')