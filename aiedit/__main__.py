import argparse

from aiedit import train

AIEDIT_LOGO = """
           _____ ______    _ _ _            
     /\   |_   _|  ____|  | /_\ |          
    /  \    | | | |__   __| | | |_         
   / /\ \   | | |  __| / _` | | __|       
  / ____ \ _| |_| |___| (_| | | |_         
 /_/    \_\_____|______\__,_|_|\__|
"""

print(AIEDIT_LOGO[1:-1])
print("Version 1.0.0")
print()

parser = argparse.ArgumentParser("aiedit")
subparsers = parser.add_subparsers(help="subcommand help", required=True)

train.add_subparser(subparsers)

args = parser.parse_args()
args.func(args)
