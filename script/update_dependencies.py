#!/usr/bin/env python3

import re
import sys
from pathlib import Path
from typing import List, Union

modizer_repo_dir: Path = Path(__file__).resolve().parent.parent.resolve()
external_repo_dir: Path = modizer_repo_dir.parent / 'chiptunes' / 'external'

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

        self.modizer_dir: Path = modizer_repo_dir / modizer_dependency_dir
        if isinstance(upstream_dependency_dirs, str):
            self.upstream_dirs: List[Path] = [external_repo_dir / upstream_dependency_dirs]
        else:
            self.upstream_dirs: List[Path] = [external_repo_dir / d for d in upstream_dependency_dirs]


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

if __name__ == '__main__':
    main()
