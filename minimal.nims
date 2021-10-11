import globals
import strformat

switch("cc", cc)

case cc:
  of "tcc":
    switch("threads", "off")
    switch("tlsEmulation", "on")
  of "vcc":
    switch("passC", "/wd4311 /wd4312 /wd4103") #ignore 4311:pointer truncation, 4312: conversion of pointer to greater size, 4103: alignment changed after including header  
    switch("threads", "on")
    switch("tlsEmulation", "off")
  of "gcc":
    switch("threads", "on")
    switch("tlsEmulation", "off")
  else: 
    #cc = gcc errors duplicate and undefined references on windows
    raise newException(Defect, cc & " is not supported.")

switch("app", "lib")
switch("gc", "arc")
switch("d", "danger")
switch("nomain", "on")
switch("include", "./globals.nim")
switch("path", "api")
switch("o", "tm_minimal.dll")