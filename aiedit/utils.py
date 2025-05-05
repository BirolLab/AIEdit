import glob


def load_seeds(path: str) -> list[str]:
    with open(path) as fp:
        seeds = list(map(str.strip, fp.readlines()))
    return seeds


def glob_seed_paths(pattern: str) -> list[list[str]]:
    seed_paths = [p for w in pattern for p in glob.glob(w, recursive=True)]
    return [load_seeds(path) for path in seed_paths]
