{ pkgs   ? import <nixpkgs> {},
  stdenv ? pkgs.clang14Stdenv,
  hwloc ? pkgs.hwloc
}:

stdenv.mkDerivation rec {
  name = "parlay-dev-shell";
  
  buildInputs = [pkgs.cmake pkgs.extra-cmake-modules hwloc];

  HWLOC_DEV_PATH="${hwloc.dev}";
  HWLOC_LIB_PATH="${hwloc.lib}";

}
