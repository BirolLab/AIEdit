import argparse
import signal

import aiedit.polishing.args
import aiedit.training.generate_seeds
import aiedit.training.train
from aiedit import __version__

AIEDIT_LOGO = """
           _____ ______    _ _ _            
     /\   |_   _|  ____|  | /_\ |          
    /  \    | | | |__   __| | | |_         
   / /\ \   | | |  __| / _` | | __|       
  / ____ \ _| |_| |___| (_| | | |_         
 /_/    \_\_____|______\__,_|_|\__|
"""


def main():
    print(AIEDIT_LOGO[1:-1])
    print(f"Version {__version__}")
    print()

    parser = argparse.ArgumentParser("aiedit")
    subparsers = parser.add_subparsers(help="subcommand help", required=True)

    aiedit.polishing.args.add_subparser(subparsers)
    aiedit.training.train.add_subparser(subparsers)
    aiedit.training.generate_seeds.add_subparser(subparsers)

    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal.SIG_DFL)
    args.func(args)
