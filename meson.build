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

dependencies = [
    dependency('argparse'),
    dependency('frugally-deep', modules: ['frugally-deep::fdeep']),
    dependency('nlohmann_json'),
    dependency('openmp'),
    meson.get_compiler('cpp').find_library('btllib'),
]

executable(
    'aiedit',
    configure_file(
        input: 'src/main.cpp',
        output: 'main.cpp',
        configuration: {'PROJECT_VERSION': meson.project_version()},
    ),
    include_directories: include_directories('src'),
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