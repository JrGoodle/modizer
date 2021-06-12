#!/usr/bin/env python3

import re
import sys
from pathlib import Path
from typing import List

modizer_repo_dir: Path = Path(__file__).resolve().parent.parent.resolve()
external_repo_dir: Path = modizer_repo_dir.parent / 'chiptunes' / 'external'

class Project(object):
    """Project class

    :ivar Path modizer_dir: Path to dependency directory in modizer
    :ivar Path upstream_dir: Path to dependency directory in external
    """

    def __init__(self, modizer_dependency_dir: str, upstream_dependency_dir: str):
        """Change __init__

        :param str content: Change content
        """

        self.modizer_dir: Path = modizer_repo_dir / modizer_dir
        self.upstream_dir: Path = external_repo_dir / upstream_dependency_dir


def main() -> None:
    projects = [
        Project('Adplug', 'adplug'),
        Project('AHX', 'hivelytracker'),
        Project('asap', 'asap'),
        Project('ffmpeg-kit-audio-4.4.LTS-ios-framework', 'ffmpeg-kit'),
        Project('freeverb', 'Freeverb/freeverb'),
        Project('highlyexperimental', 'webpsx'),
        Project('HighlyQuixotic', 'Highly_Quixotic'),
        Project('highlytheoritical', 'Highly_Theoretical'),
        Project('LibAtrac9-master', 'LibAtrac9'),
        Project('libGME', 'libgme'),
        Project('libgsf', 'libgsf'),
        Project('libkss', 'libkss'),
        Project('libLazyusf', 'lazyusf2'),
        Project('libopenmpt', 'openmpt'),
        Project('libpsflib', 'psflib'),
        Project('libtim', 'libtim'),
        Project('libvgmplay', 'vgmplay'),
        Project('libvgmstream', 'vgmstream'),
        Project('libxmp', 'libxmp'),
        Project('libxsf', 'in_xsf'),
        Project('libxsf_snsf', 'in_xsf'),
        Project('mdxplay', 'mdxmini'),
        Project('node-lame-master', 'node-lame'),
        Project('pmdmini', 'pmdmini'),
        Project('sc68', 'sc68-2.2.1'),
        Project('sid', 'sidplayfp'),
        Project('snsf', 'in_xsf'),
        Project('STSound', 'StSound'),
        Project('uade-2.13', 'uade-2.13'),
        Project('v2mplayer', 'v2m-player')
    ]

if __name__ == '__main__':
    main()
