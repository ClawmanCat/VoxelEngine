import common
import os

from shutil import copyfile


def main():
    args = common.parse_args(required_args = ['arch', 'dest', 'files'])

    src_dir = common.get_sdk_subdir('Bin', args['arch'] == 'x86')
    dst_dir = args['dest']

    for file in args['files'].split(';'):
        os.makedirs(dst_dir, exist_ok = True)

        file_path = os.path.join(src_dir, common.find_file_without_extension(src_dir, file))
        copyfile(file_path, os.path.join(dst_dir, file + os.path.splitext(file_path)[1]))

if __name__ == '__main__': main()
