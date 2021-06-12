#!/usr/bin/env python3

import re
import sys
from pathlib import Path
from typing import List

repo_dir: Path = Path(__file__).resolve().parent.parent.resolve()


class Project(object):
    """Project class

    :ivar str commit_hash: Git commit hash
    :ivar list categories: Change categories
    :ivar str message: Change message
    :ivar str asana_url: Asana url if present
    :ivar str crashlytics_url: Crashlytics url if present
    :ivar str bugsnag_url: Bugsnag url if present
    """

    def __init__(self, modizer_dir: str, upstream_dir: str):
        """Change __init__

        :param str content: Change content
        """

        self.modizer_dir: Path = Path(modizer_dir)
        self.upstream_dir: Path = Path(upstream_dir)


def main() -> None:
    projects = [
        Project('Adplug', ''),
        Project('AHX', ''),
        Project('asap', ''),
        Project('ffmpeg-kit-audio-4.4.LTS-ios-framework', ''),
        Project('freeverb', ''),
        Project('highlyexperimental', ''),
        Project('HighlyQuixotic', ''),
        Project('highlytheoritical', ''),
        Project('LibAtrac9-master', ''),
        Project('libGME', ''),
        Project('libgsf', ''),
        Project('libkss', ''),
        Project('libLazyusf', ''),
        Project('libopenmpt', ''),
        Project('libpsflib', ''),
        Project('libtim', ''),
        Project('libvgmplay', ''),
        Project('libvgmstream', ''),
        Project('libxmp', ''),
        Project('libxsf', ''),
        Project('libxsf_snsf', ''),
        Project('mdxplay', ''),
        Project('node-lame-master', ''),
        Project('pmdmini', ''),
        Project('sc68', ''),
        Project('sid', ''),
        Project('snsf', ''),
        Project('STSound', ''),
        Project('uade-2.13', ''),
        Project('v2mplayer', '')
    ]

if __name__ == '__main__':
    main()
