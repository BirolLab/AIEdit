project(
    'AIEdit',
    'cpp',
    version: '0.1',
    license: 'GPL-3',
    default_options: [
        'cpp_std=c++17',
        'werror=true',
        'warning_level=3',
        'optimization=3',
        'b_lto=true',
    ],
)

conf = configuration_data()
conf.set('PROJECT_VERSION', meson.project_version())
configure_file(
    input: 'src/version.hpp.in',
    output: 'version.hpp',
    configuration: conf,
)

compiler = meson.get_compiler('cpp')

dependencies = [
    dependency('argparse'),
    dependency('frugally-deep', modules: ['frugally-deep::fdeep']),
    dependency('nlohmann_json'),
    dependency('openmp'),
    compiler.find_library('btllib'),
]

sources = run_command(
    'scripts/list_sources.sh',
    check: true,
).stdout().strip().split()

executable(
    'aiedit',
    ['src/main.cpp'] + sources,
    dependencies: dependencies,
    install: true,
)

# --------------------------------- TESTS ---------------------------------
catch2 = dependency(
    'Catch2',
    method: 'cmake',
    modules: ['Catch2::Catch2WithMain'],
    required: false,
)

if catch2.found()

    tests = run_command(
        'scripts/list_tests.sh',
        check: true,
    ).stdout().strip().split()

    foreach test_file : tests
        test_name = test_file.split('.cpp')[0].split('/')[-1]
        test_exec = executable(
            test_name,
            sources + [test_file],
            dependencies: [catch2] + dependencies,
            include_directories: ['src'],
        )
        test(test_name, test_exec)
    endforeach

else
    warning('Tests require catch2 to be installed.')

endif