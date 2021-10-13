# Package

version       = "0.1.0"
author        = "geekrelief"
description   = "The Machinery bindings generator."
license       = "MIT"

bin = @["tm_gen"]
# Dependencies

requires "nim >= 1.4.0"

const dev = false

when not defined(dev):
  requires "https://github.com/geekrelief/nimterop >= 0.8.0"

import globals
import strformat
import os

task gen, "(dev) Generate the binding":
  if dev:
    exec &"nim c -d:dev tm_gen.nim"
  else:
    exec &"nim c tm_gen.nim"

proc commonFlags(): seq[string] =
  let cc = "vcc" # tcc doesn't like pragma once, getting undefined errors with gcc linker again
  var flags = @[&"--cc:{cc}"]

  flags.add case cc:
    of "vcc":
      #ignore 4311:pointer truncation, 4312: conversion of pointer to greater size, 4103: alignment changed after including header (windows header warnings)
      # 4133: incompatible types for proc callback, 4028: parameter different from declaration ({.header, importc.} warnings)
      "--passC:\"/wd4311 /wd4312 /wd4103 /wd4133 /wd4028\"" & 
      "--threads:on --tlsEmulation:off"
    #[of "gcc": "--threads:on --tlsEmulation:off" ]#
    else: 
      raise newException(Defect, cc & " is not supported.")

  flags &= @["--app:lib", "--gc:arc", "-d:danger", "--nomain:on", "--include:globals.nim", "--path:.", "--path:tm", "--path:samples"]
  flags

proc buildProject(nimFilePath, outDir: string): void =
  var (dir, name, ext) = splitFile(nimFilePath)
  let settings = commonFlags() & &"-o:tm_{name}.dll --outdir:\"{outDir}\""
  mkdir(outDir)
  exec &"nim c {settings.join(\" \")} {nimFilePath}"

task minimal, "Build the minimal sample":
  buildProject("samples/plugins/minimal.nim", "C:/tm/gr-tm/bin/Debug/plugins")

task simentry, "Build the simulation entry sample":
  buildProject(
    "samples/plugins/custom_simulation_entry.nim", 
    "C:/tm/tm-nim/build/samples/plugins/simulation")