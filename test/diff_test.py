#!/usr/bin/env python3

import logging
import os
import subprocess
import sys


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='[%(asctime)s] %(message)s')
    
    passed = True
    for filename in os.listdir():
        if filename.endswith('.ac'):
            prefix = filename[:-3]
            logging.info(f'testing {prefix}')
            try:
                splc_return_code = subprocess.run(['../bin/splc', prefix + '.spl']).returncode
                assert splc_return_code in (0, 4)
                subprocess.run(['diff', filename, prefix + '.out'], check=True)
            except AssertionError:
                logging.error(f'  failed: unexpected exit status (code={splc_return_code})')
                passed = False
            except subprocess.CalledProcessError:
                logging.error(f'  failed: wrong output')
                passed = False
    if not passed:
        sys.exit('tests failed')
