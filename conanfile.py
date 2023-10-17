from conan import ConanFile
from conan.tools.meson import Meson


class AIEditRecipe(ConanFile):
    requires = "argparse/2.9", "frugally-deep/0.15.24-p0", "nlohmann_json/3.11.2"
    build_requires = "meson/1.2.2"
    settings = "os", "compiler", "build_type", "arch"
    generators = "PkgConfigDeps", "MesonToolchain"

    def build(self):
        meson = Meson(self)
        meson.configure()
        meson.build()
