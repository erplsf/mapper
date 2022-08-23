{ pkgs ? import <nixpkgs> { } }:

with pkgs;

mkShell { buildInputs = [ conan gnumake cmake gcc11 ]; }
