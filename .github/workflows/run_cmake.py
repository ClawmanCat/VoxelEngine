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


def add_path(path):
    os.environ['PATH'] += os.pathsep + path


def log_env():
    print('Current Environment:')
    
    for k, v in os.environ.items():
        print(f'{k}={v}')
    
    print('', flush = True)


def find(root_dir, file):
    for content in os.listdir(root_dir):
        content_path = os.path.join(root_dir, content)
        
        if content == file:
            return content_path
        
        if os.isdir(content_path):
            subresult = find(content_path, file)
            if subresult is not None: return subresult

    return None


  
def main():
    where = find(R'C:\Program Files (x86)\Microsoft Visual Studio', 'vcvars64.bat')
    print(f'vcvars64: {where}', flush = True)

    # runcmd(R'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat')
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