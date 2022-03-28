import subprocess
import os


def runcmd(*args, verbose = True):
    if verbose: print('> ' + ' '.join([*args]))
    result = subprocess.run([*args], capture_output = True)

    def print_stdout(result_stdout):
        if verbose:
            for line in result_stdout: print(line)
            print('', flush = True)

    if result.returncode != 0:
        print_stdout(result.stdout.splitlines())
        raise RuntimeError(f'Subprocess {" ".join(args)} failed with error code {result.returncode}: {result.stderr}')

    result = result.stdout.splitlines()
    print_stdout(result)
        
    return result


def add_path(path):
    os.environ['PATH'] += os.pathsep + path


def log_env():
    print('Current Environment:')
    
    for k, v in os.environ.items():
        print(f'{k}={v}')
    
    print('', flush = True)


def find(root_dir, file, depth = 0):
    indent = ' ' * (depth * 4)
    print(indent + f'Searching {root_dir}...')

    for content in os.listdir(root_dir):
        content_path = os.path.join(root_dir, content)
        
        if content == file:
            print(indent + f'Found {content}', flush = True)
            return content_path
        
        if os.path.isdir(content_path):
            subresult = find(content_path, file, depth + 1)
            if subresult is not None: return subresult

    print(indent + f'Found nothing.', flush = True)
    return None


  
def main():
    runcmd(
        R'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat',
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