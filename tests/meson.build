catch2 = dependency('Catch2', method : 'cmake', modules : ['Catch2::Catch2WithMain'], required: false)

if not catch2.found()
  error('Tests require catch2 to be installed.')
endif

tests = run_command('scripts/list_tests.sh', check: true).stdout().strip().split()

foreach test_file : tests
    test_name = test_file.split('.cpp')[0].split('/')[-1]
    test_exec = executable(test_name, 
                           sources + [test_file],
                           dependencies: dependencies + [catch2],
                           include_directories: include_dirs)
    test(test_name, test_exec)
endforeach