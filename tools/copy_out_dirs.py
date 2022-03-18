import os
import shutil
import sys


# We can't simply use shutil.copytree since it will not overwrite files.
def copy_recursive(src_dir, dst_dir):
    print(f'{src_dir} => {dst_dir}', flush = True)

    for f in os.listdir(src_dir):
        f_real = os.path.realpath(os.path.join(src_dir, f))
    
        if os.path.isdir(f_real):
            subdst = os.path.realpath(os.path.join(dst_dir, f))
        
            os.makedirs(subdst, exist_ok = True)
            copy_recursive(f_real, subdst)
        else:
            subdst = os.path.realpath(os.path.join(dst_dir, f))
        
            print(f'Copying {f}', flush = True)
            shutil.copy(f_real, subdst)


def main():
    if len(sys.argv) < 2:
        print('Usage: python copy_out_dirs.py [debug|release|...]')
        return

    src = '../out_dirs'
    dst = '../out/$CFG/bin'
    cfg = sys.argv[1].lower()
    
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    copy_recursive(src, dst.replace('$CFG', cfg))


if __name__ == '__main__': main()