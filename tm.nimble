# Package

version       = "0.1.0"
author        = "geekrelief"
description   = "The Machinery bindings generator."
license       = "MIT"

bin = @["tm_gen"]
# Dependencies

requires "nim >= 1.4.0"
requires "https://github.com/geekrelief/nimterop#define"

import globals
import strformat
import os

proc commonFlags(): seq[string] =
  var flags = @[&"--cc:{cc}"]

  flags.add case cc:
    of "tcc":
      "--threads:off --tlsEmulation:on"
    of "vcc":
      "passC:\"/wd4311 /wd4312 /wd4103\"" & #ignore 4311:pointer truncation, 4312: conversion of pointer to greater size, 4103: alignment changed after including header  
      "--threads:on --tlsEmulation:off"
    of "gcc":
      "--threads:on --tlsEmulation:off"
    else: 
      raise newException(Defect, cc & " is not supported.")

  flags &= @["--app:lib", "--gc:arc", "-d:danger", "--nomain:on", "--include:globals.nim", "--path:.", "--path:tm", "--path:samples"]
  flags

proc buildProject(nimFilePath, pluginDir: string): void =
  var (dir, name, ext) = splitFile(nimFilePath)
  let settings = commonFlags() & &"-o:tm_{name}.dll --outdir:\"{pluginDir}\""
  exec &"nim c {settings.join(\" \")} {nimFilePath}"

task minimal, "Build the minimal sample":
  buildProject("samples/plugins/minimal.nim", "C:/tm/gr-tm/bin/Debug/plugins")