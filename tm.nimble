# Package

version       = "0.1.0"
author        = "geekrelief"
description   = "The Machinery bindings generator."
license       = "MIT"

# Dependencies
requires "nim >= 1.7.1"
requires "https://github.com/geekrelief/ptr_math >= 0.6.0"
requires "https://github.com/geekrelief/genit >= 0.10.0"
requires "https://github.com/geekrelief/nillean >= 0.1.0"

const dev = false # bindings development flag

#> configuration variables
const mode = "--debugger:native --debuginfo:on" # -d:danger, -d:release
const cc = "tcc" # vcc or tcc work, but tcc needs to modify headers, gcc linker is having issues

const samples_dir = "samples/plugins/"
const build_dir = "C:/tm/tm-nim/build/samples/plugins/"
const tm_plugins_dir = "C:/tm/gr-tm/bin/Debug/plugins/"
#< configuration variables


when not defined(dev):
  requires "https://github.com/geekrelief/nimterop >= 0.8.10"

import globals
import strformat, strutils
import std/[os]

proc taskParams(): seq[string] = # nimble's paramCount / paramStr is broken in v0.13.1
  var params = commandLineParams()
  if params.len > 9:
    params[8 .. ^2]
  else:
    @[]
  
task gen, "Generate the binding":
  if cc == "tcc":
    exec "nim r tcc_mods.nim"
  exec "nim r -d:release deps.nim"
  if dev:
    exec &"nim c -d:dev --cc:{cc} tm_gen.nim"
  else:
    exec &"nim c --cc:{cc} tm_gen.nim"

proc commonFlags(): seq[string] =
  var flags = @[&"--cc:{cc}"]
  if dev:
    flags.add "--path:\"../gr-ptr_math/src\""
    flags.add "--path:\"../../nimdev/genit/src\""
    #flags.add "--path:\"../../nimdev/elvis/\""

  flags.add case cc:
    of "vcc":
      #ignore 4311:pointer truncation, 4312: conversion of pointer to greater size, 4103: alignment changed after including header (windows header warnings)
      # 4133: incompatible types for proc callback, 4028: parameter different from declaration ({.header, importc.} warnings)
      "--passC:\"/wd4311 /wd4312 /wd4103 /wd4133 /wd4028\" " & 
      "--threads:on --tlsEmulation:off "
    of "tcc":
      "--threads:off --tlsEmulation:on "
    else: 
      raise newException(Defect, cc & " is not supported.")

  flags &= @["--app:lib", "--mm:arc", &"{mode}", "--nomain:on", "--include:globals.nim", "--path:.", "--path:tm", "--path:samples"]
  flags


proc buildProject(name, targetDir: string = ""): void =
  let nimFilePath = samples_dir & name & ".nim"
  let dll = &"tm_{name}.dll"
  var outdir = build_dir & name & "/"
  mkdir(outdir)
  let settings = commonFlags() & &"-o:{dll} --outdir:\"{outdir}\""
  exec &"nim c {settings.join(\" \")} {nimFilePath}"
  var targetDir = if targetDir.len == 0: tm_plugins_dir else: targetDir
  mkdir(targetDir)
  if outdir.normalizedPath != targetDir.normalizedPath:
    cpFile(outdir & dll, targetDir & dll)


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


task minimal, "Build the minimal sample":
  buildProject("minimal")

task simentry, "Build the simulation entry sample":
  #buildProject("custom_simulation_entry", "C:/tm/tm-nim/build/samples/plugins/simulation/")
  buildProject("custom_simulation_entry")

task callbacks, "Build the plugin_callbacks sample":
  buildProject("plugin_callbacks")

task component, "Build the custom component sample":
  #buildProject("custom_component", "C:/tm/tm-nim/build/samples/plugins/custom_component/")
  buildProject("custom_component")

task first, "Build gameplay sample first person":
  buildProject("gameplay_sample_first_person", "C:/tm/tm-nim/build/samples/plugins/gameplay_sample_first_person/")

task third, "Build gameplay sample third person":
  buildProject("gameplay_sample_third_person", "C:/tm/tm-nim/build/samples/plugins/gameplay_sample_third_person/")