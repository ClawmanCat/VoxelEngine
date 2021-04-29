import sys
import re
import os

from typing import List


def parse_args(required_args: List[str] = []):
    args = dict()

    for arg in sys.argv:
        # Match mykey=myval and mykey="myval"
        match = re.search(r'^([^=]+)=([\"\']?)(.+)\2$', arg)

        if match is not None:
            args[match.group(1)] = match.group(3)
        else:
            args[arg] = None

    for key in required_args:
        if key not in args or args[key] is None:
            raise KeyError(f'Missing required parameter {key}')

    return args


def get_sdk_root():
    for key in ['VK_SDK_DIR', 'VULKAN_SDK']:
        if key in os.environ:
            return os.environ[key].replace('\\', '/')

    raise FileNotFoundError('Failed to find Vulkan SDK.')


def get_sdk_subdir(dirname: str, use_x86_folder: bool = False):
    root = get_sdk_root()

    if use_x86_folder:
        return os.path.join(root, dirname + '32')
    else:
        return os.path.join(root, dirname)


def get_sdk_version():
    sdk_dir = get_sdk_root()
    return sdk_dir.split('/')[-1]


def find_file_without_extension(path: str, file: str):
    for possible_file in os.listdir(path):
        if re.match(rf'^{file}(\..+)?$', possible_file):
            return file + os.path.splitext(possible_file)[1]

    raise FileNotFoundError(f'No such file {file} in {path}.')
