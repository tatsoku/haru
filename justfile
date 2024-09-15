default:
  just --list

@fix_perms:
  chmod +x build/build.sh

@init-clangd: fix_perms
  build/build.sh -f

@compile extra_args="": fix_perms
  build/build.sh -c {{extra_args}}

@link name="haru": fix_perms
  build/build.sh -l {{name}}

@test extra_arg1="" extra_arg2 ="": fix_perms
  build/build.sh -t {{extra_arg1}} {{extra_arg2}}

@setup-testing: fix_perms
  build/build.sh -st

@build name="haru": fix_perms
  just compile {{name}}
  just link {{name}}

@run args="" name="haru": fix_perms
  just compile {{name}}
  just link {{name}}
  ./build/bin/{{name}} {{args}}

@debug-linux args="" name="haru": fix_perms
  just compile
  just link {{name}}
  valgrind ./build/bin/{{name}} {{args}}

@debug-windows args="" name="haru": fix_perms
  just compile --debug
  just link {{name}}
  lldb ./build/bin/{{name}}.exe {{args}}

@debug-no-debugger args="" name="haru": fix_perms
  just compile --debug
  just link {{name}}
  ./build/bin/{{name}} {{args}}

@clear_cores: fix_perms
  build/build.sh -vg

@install name="haru": fix_perms
  just build {{name}}
  sudo mv bin/{{name}} /usr/bin/
