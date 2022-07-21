catch2 = dependency('Catch2', method : 'cmake', modules : ['Catch2::Catch2WithMain'], required: false)

ai_edit_lib = declare_dependency(
  link_with : static_lib,
  include_directories : include_dirs
)

if not catch2.found()
  error('Tests require catch2 to be installed.')
endif

tests = run_command('find_tests.sh', check: true).stdout().strip().split()

foreach test_file : tests
    test_name = test_file.split('.cpp')[0].split('/')[-1]
    test_exec = executable(test_name, 
                           test_file,
                           dependencies: dependencies + [ai_edit_lib, catch2],
                           include_directories: include_dirs)
    test(test_name, test_exec)
endforeach
