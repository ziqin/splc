#!/usr/bin/env python3

import os
import subprocess

for filename in os.listdir():
    if filename.endswith('.ac'):
        prefix = filename[:-3]
        assert subprocess.run(['../bin/splc', prefix + '.spl']).returncode in (0, 4)
        subprocess.run(['diff', filename, prefix + '.out'], check=True)
