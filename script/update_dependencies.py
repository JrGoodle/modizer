#!/usr/bin/env python3

import os
import shutil
from pathlib import Path
from typing import List, Union

modizer_repo_dir: Path = Path(__file__).resolve().parent.parent.resolve()
modizer_libs_dir: Path = modizer_repo_dir / 'libs'
external_dir: Path = modizer_repo_dir.parent / 'chiptunes' / 'external'

class Project(object):
    """Project class

    :ivar Path modizer_dir: Path to dependency directory in modizer
    :ivar List[Path] upstream_dirs: Path to dependency directories in external
    """

    def __init__(self, modizer_dependency_dir: str, upstream_dependency_dirs: Union[List[str], str]):
        """Change __init__

        :param str modizer_dependency_dir: Modizer dependency dir containing source code
        :param Union[List[str], str] upstream_dependency_dirs: Upstream dependency dirs containing code
        """

        self.modizer_dir: Path = modizer_libs_dir / modizer_dependency_dir
        if isinstance(upstream_dependency_dirs, str):
            self.upstream_dirs: List[Path] = [external_dir / upstream_dependency_dirs]
        else:
            self.upstream_dirs: List[Path] = [external_dir / d for d in upstream_dependency_dirs]


def remove_dir(dir_path: Path, ignore_errors: bool = False) -> None:
    shutil.rmtree(str(dir_path), ignore_errors=ignore_errors)


def listdir(directory: Path) -> List[Path]:
    files = os.listdir(directory)
    return [directory / f for f in files]


def copy_directory(from_dir: Path, to_path: Path):
    # TODO: Replace rmdir() with copytree(dirs_exist_ok=True) when support for Python 3.7 is dropped
    remove_dir(to_path, ignore_errors=True)
    shutil.copytree(from_dir, to_path, symlinks=True)


def copy_file(from_path: Path, to_path: Path):
    shutil.copyfile(from_path, to_path)


def copy(from_path: Path, to_path: Path):
    if from_path.is_dir():
        copy_directory(from_path, to_path)
    else:
        copy_file(from_path, to_path)


def make_dir(dir_path: Path, exist_ok: bool = False) -> Path:
    os.makedirs(dir_path, exist_ok=exist_ok)
    return dir_path


def main() -> None:
    third_party_projects = [
        Project('Adplug', 'adplug/src'),
        Project('AHX', 'hivelytracker/hvl2wav'),
        Project('asap', 'asap'),
        # Project('ffmpeg-kit-audio-4.4.LTS-ios-framework', 'ffmpeg-kit'),
        Project('freeverb', ['Freeverb/freeverb/FreeverbVST', 'Freeverb/freeverb/Components']),
        Project('highlyexperimental/highlyexperimental', 'webpsx'),
        Project('HighlyQuixotic/HighlyQuixotic', 'Highly_Quixotic/Core'),
        Project('highlytheoritical/highlytheoritical', 'Highly_Theoretical/Core'),
        Project('LibAtrac9-master', 'LibAtrac9'),
        Project('libGME/gme', 'libgme/gme'),
        Project('libgsf', 'playgsf'),
        Project('libkss/libkss', 'libkss'),
        Project('libLazyusf/libLazyusf', 'lazyusf2'),
        Project('libopenmpt/openmpt-trunk', 'openmpt'),
        Project('libpsflib/psflib', 'psflib'),
        Project('libtim', 'timidity'),
        Project('libvgmplay/vgm', 'vgmplay/VGMPlay'),
        Project('libvgmstream', 'vgmstream/src'),
        Project('libxmp/libxmp-master', 'libxmp'),
        Project('libxsf', 'in_xsf'),
        Project('libxsf_snsf', 'in_xsf'),
        Project('mdxplay', 'mdxmini/src'),
        Project('node-lame-master', 'node-lame'),
        Project('pmdmini/pmdmini', 'pmdmini/src'),
        Project('sc68', 'sc68-2.2.1'),
        Project('sid/libsidplayfp', 'sidplayfp'),
        Project('snsf/snsf9x', 'snsf9x/snsf9x'),
        Project('STSound', 'StSound'),
        Project('uade-2.13', 'uade-2.13'),
        Project('v2mplayer/v2mplayer', 'v2m-player')
    ]

    for project in third_party_projects:
        remove_dir(project.modizer_dir, ignore_errors=True)
        make_dir(project.modizer_dir)
        for path in project.upstream_dirs:
            files = listdir(path)
            for file in files:
                from_path = file
                to_path = project.modizer_dir / file.name
                print(f'Copy file from {from_path} to {to_path}')
                copy(from_path, to_path)


if __name__ == '__main__':
    main()
