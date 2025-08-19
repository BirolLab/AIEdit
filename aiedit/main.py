import argparse
import importlib
import signal

import aiedit.generate_seeds.args
import aiedit.list_models.main
import aiedit.polish.args
import aiedit.train.args
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
    subparsers = parser.add_subparsers(dest="command", required=True)

    aiedit.polish.args.add_subparser(subparsers)
    aiedit.train.args.add_subparser(subparsers)
    aiedit.generate_seeds.args.add_subparser(subparsers)
    aiedit.list_models.main.add_subparser(subparsers)

    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal.SIG_DFL)
    subcommand = importlib.import_module(f"aiedit.{args.command}.main")
    subcommand.main(args)
