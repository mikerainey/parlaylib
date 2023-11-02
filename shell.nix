{ pkgs   ? import <nixpkgs> {},
  stdenv ? pkgs.clang14Stdenv,
  hwloc ? pkgs.hwloc,
  taskparts ? import ../nix-packages/pkgs/taskparts/hdronly.nix {stdenv = stdenv; cmake = pkgs.cmake; taskpartsSrc = ./../successor;}
}:

#cmake . -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DPARLAY_EXAMPLES=On -DPARLAY_TASKPARTSHDRONLY=On -DFEW_EXAMPLES=ON -DTASKPARTS_LOGGING=On  && cmake --build .

stdenv.mkDerivation rec {
  name = "parlay-dev-shell";
  
  buildInputs = [pkgs.cmake pkgs.extra-cmake-modules hwloc taskparts];

  HWLOC_DEV_PATH="${hwloc.dev}";
  HWLOC_LIB_PATH="${hwloc.lib}";

  TASKPARTS_DEFAULT_SEARCH_DIR="${taskparts}";

}
