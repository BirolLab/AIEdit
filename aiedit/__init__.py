import importlib.metadata

import aiedit.core as core

try:
    __version__ = importlib.metadata.version("aiedit")
except importlib.metadata.PackageNotFoundError:
    __version__ = "dev"
