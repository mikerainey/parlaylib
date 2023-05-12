{ pkgs   ? import <nixpkgs> {},
  stdenv ? pkgs.stdenv
}:

stdenv.mkDerivation rec {
  name = "parlaylib";

  src = ./.;

  nativeBuildInputs = [ pkgs.cmake pkgs.extra-cmake-modules ];

  configurePhase = ''
    mkdir -p __build
    cd __build
    cmake .. -DCMAKE_INSTALL_PREFIX:PATH=$out
  '';

}
