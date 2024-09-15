{pkgs ? import <nixpkgs> {}}:
with pkgs;
  mkShell {
    allowUnfree = true;
    name = "haru!";
    packages = [
      zsh
      glfw
      freetype
      pre-commit
      bash-language-server
      valgrind
      yamlfix
      yamlfmt
      alejandra
      cbfmt
      mdformat
      beautysh
      shfmt
      shellcheck
      vulkan-tools
      renderdoc
      tracy
      vulkan-tools-lunarg
      clang-tools
      clang
      mold
      cmake
      glslang
      vulkan-headers
      vulkan-loader
      vulkan-validation-layers
      shaderc
      pkg-config
    ];

    buildInputs = [
      glfw
      freetype
    ];

    LD_LIBRARY_PATH = "${glfw}/lib:${freetype}/lib:${vulkan-loader}/lib:${vulkan-validation-layers}/lib";
    VULKAN_SDK = "${vulkan-headers}";
    VK_LAYER_PATH = "${vulkan-validation-layers}/share/vulkan/explicit_layer.d";

    shellHook = ''
      pre-commit install
    '';
  }
