import subprocess
import os


def runcmd(*args, verbose = True):
    if verbose: print('> ' + ' '.join([*args]))
    result = subprocess.run([*args], capture_output = True)

    if result.returncode != 0:
        raise RuntimeError(f'Subprocess {" ".join(args)} failed with error code {result.returncode}: {result.stderr}')

    result = result.stdout.splitlines()
    if verbose: print('\n'.join(result), flush = True)
    
    return result


def log_env():
    print('Current Environment:')
    
    for k, v in os.environ.items():
        print(f'{k}={v}')
    
    print('', flush = True)

  
def main():
    log_env()
    
    runcmd(
        'cmake',
        '-G', 'Ninja',
        '-DVE_GRAPHICS_API=opengl',
        '-DENABLE_TESTING=ON',
        '-DCMAKE_BUILD_TYPE=DEBUG',
        '-DCMAKE_C_COMPILER_WORKS=ON',
        '-DCMAKE_CXX_COMPILER_WORKS=ON',
        '-DCMAKE_C_COMPILER=clang-cl',
        '-DCMAKE_CXX_COMPILER=clang-cl',
        '-DCMAKE_VERBOSE_MAKEFILE=ON',
        '../'
    )


if __name__ == '__main__': main()