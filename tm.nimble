# Package

version       = "0.1.0"
author        = "geekrelief"
description   = "The Machinery bindings generator."
license       = "MIT"

bin = @["tm_gen"]
# Dependencies

requires "nim >= 1.6.0"

const dev = false

when not defined(dev):
  requires "https://github.com/geekrelief/nimterop >= 0.8.2"

import globals
import strformat, strutils
import os

proc taskParams(): seq[string] = # nimble's paramCount / paramStr is broken in v0.13.1
  var params = commandLineParams()
  if params.len > 9:
    params[8 .. ^2]
  else:
    @[]
  

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

task new, "Creates scaffolding for new plugin":
  var params = taskParams()
  if params.len != 1:
    echo "Usage: nimble new -- [path]"
    return

  var filename = params[0]
  if not filename.endsWith(".nim"): filename &= ".nim"
  filename = filename.absolutePath
  let (path, name, ext) = filename.splitFile
  if not dirExists(path):
    echo &"{path} does not exist.  Would you like to create it (y/n)?"
    let answer = readLineFromStdin()
    if answer == "y":
      mkDir(path)
    else:
      return

  if fileExists(filename):
    echo &"Error: {filename} exists."
    return

  echo &"Writing new plugin to: {filename}"
  include "plugin.template.nimf"
  writeFile(filename, plugin())

### TM Plugins

let samples_dir = "samples/plugins/"

task minimal, "Build the minimal sample":
  buildProject(samples_dir & "minimal.nim", "C:/tm/gr-tm/bin/Debug/plugins")

task simentry, "Build the simulation entry sample":
  buildProject(samples_dir & "custom_simulation_entry.nim", 
    "C:/tm/tm-nim/build/samples/plugins/simulation")

task callbacks, "Build the plugin_callbacks sample":
  buildProject(samples_dir & "plugin_callbacks.nim", "C:/tm/gr-tm/bin/Debug/plugins")

task component, "Build the custom component sample":
  buildProject(samples_dir & "custom_component.nim", "C:/tm/tm-nim/build-samples/plugins/custom_component")