import datetime
import json
import re
import os
import requests
import csv
import common


class LicenseInfo:
    def __init__(self, path, file, package, licenses, copyright):
        self.path      = path
        self.file      = file
        self.package   = package
        self.licenses  = licenses.split(' & ')
        self.copyright = copyright


class LicenseFetcherInfo:
    def __init__(self, id, code, name):
        self.id   = id
        self.code = code
        self.name = name


class LicenseFile:
    def __init__(self, **kwargs):
        for key in ['id', 'name', 'base_license', 'notice', 'code', 'create_date', 'mod_date']:
            setattr(self, key, kwargs[key])

        # API uses inconsistent naming.
        if 'body' in kwargs: self.notice = kwargs['body']


def fetch(url):
    return requests.get(url).content.decode('utf-8')


def fetch_license_overview(os: str, version: str):
    if os not in ['macos', 'windows', 'linux']:
        raise KeyError(f'Unknown operating system: {os}')

    csv_file = fetch(rf'https://vulkan.lunarg.com/software/license/vulkan-{version}-{os}-license-detail.csv').split('\n')

    reader = csv.reader(csv_file, delimiter = ',')
    return { row[1]: LicenseInfo(*row[0:5]) for row in reader }


def fetch_license_file_list():
    json_data = json.loads(fetch(fr'https://vulkan.lunarg.com/software_license/list.json'))

    result = dict()
    for obj in json_data:
        subresult = LicenseFetcherInfo(**obj)
        result[subresult.code] = subresult

    return result


def fetch_license_file(info: LicenseFetcherInfo):
    json_data = json.loads(fetch(fr'https://vulkan.lunarg.com/software_license/record/{info.id}.json'))
    return LicenseFile(**json_data)


def fix_typos(info: LicenseInfo):
    # Thank you LunarG, very nice.
    typo_dict = {
        'glslangPreprocesssor': 'glslangPreprocessor'
    }

    info.licenses = list(map(lambda l: typo_dict[l] if l in typo_dict else l, info.licenses))
    return info


def gen_combined_license_file(info: LicenseInfo, version: str):
    current_date = datetime.date.today().strftime('%Y/%m/%d')
    current_year = datetime.date.today().strftime('%Y')

    license_list = fetch_license_file_list()

    combined_license_file = "\n".join([
        f"This is an automatically generated license file, created by parsing the various license files at https://vulkan.lunarg.com/license/",
        f"and the license summary at https://vulkan.lunarg.com/license/#/release/",
        f"For the most up-to-date license information, you should consult these URLs.",
        f"This file was generated on {current_date} for Vulkan SDK version {version}.",
        f"",
        f""
    ])

    for license in info.licenses:
        license = fetch_license_file(license_list[license])
        license_text = ""

        def add_row(text):
            nonlocal license_text
            license_text += text + '\n'

        add_row(license.name)
        if len(license.base_license) > 0: add_row(f'Based on: {license.base_license}')
        add_row('\nNotice:')

        # Replace copyright placeholder with actual copyright holder.
        license_body = re.sub(
            r'(^|\n)(\/\/ )?COPYRIGHT.+(COPYRIGHT (HOLDER|OWNER)|<various other dates and companies>)',
            info.copyright,
            license.notice,
            flags = re.IGNORECASE
        )

        # Replace copyright year placeholder with current year.
        license_body = re.sub(
            r'(YYYY|YEAR)',
            current_year,
            license_body
        )


        add_row(license_body)
        add_row('\n\n')

        combined_license_file += license_text

    return combined_license_file


def main():
    args = common.parse_args(required_args = ['os', 'files', 'arch', 'dest'])

    target_os = args['os'].lower()
    if target_os == 'darwin': target_os = 'macos'

    license_overview = fetch_license_overview(target_os, common.get_sdk_version())


    for file in args['files'].split(';'):
        file_with_ext = common.find_file_without_extension(common.get_sdk_subdir('Bin', args['arch'] == 'x86'), file)
        license_info  = fix_typos(license_overview[file_with_ext])

        dest_path = os.path.join(args['dest'], f'LICENSE_{file.upper()}.txt')
        if os.path.exists(dest_path) and 'regenerate_all' not in args:
            print(f'Skipping license for {file}: license file is already present.')
        else:
            print(f'Generating license file for {file}...')

            license_text = gen_combined_license_file(license_info, common.get_sdk_version())
            with open(dest_path, 'w') as handle: handle.write(license_text)

if __name__ == '__main__': main()
