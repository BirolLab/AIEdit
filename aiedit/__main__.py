import argparse

from aiedit import __version__, generate_seeds, polish, train

AIEDIT_LOGO = """
           _____ ______    _ _ _            
     /\   |_   _|  ____|  | /_\ |          
    /  \    | | | |__   __| | | |_         
   / /\ \   | | |  __| / _` | | __|       
  / ____ \ _| |_| |___| (_| | | |_         
 /_/    \_\_____|______\__,_|_|\__|
"""

print(AIEDIT_LOGO[1:-1])
print(f"Version {__version__}")
print()

parser = argparse.ArgumentParser("aiedit")
subparsers = parser.add_subparsers(help="subcommand help", required=True)

polish.add_subparser(subparsers)
train.add_subparser(subparsers)
generate_seeds.add_subparser(subparsers)

args = parser.parse_args()
args.func(args)
