# This script does a topological sort of the header dependencies and produces a file with the order
# for nimterop. This helps to produce cleaner output for debugging.

import globals, std / [os, sugar, strformat, strutils, sequtils, streams, re, algorithm, sets]

proc getHeaders(dir: string, exclude: seq[string] = @[]): seq[string] =
  result = collect:
    for path in walkDirRec(&"{dir}"):
      #echo repr(path)
      var (dir, name, ext) = splitFile(path)
      if path in exclude: 
        echo &"excluding {name & ext}"
        continue
      if path.endsWith(".h") or path.endsWith(".inl"):
        path

proc getIncludes(path: string): seq[string] =
  var strm = newFileStream(path, fmRead)
  if not isNil(strm):
    var (dir, file, ext) = splitFile(path)
    var line = ""
    while strm.readLine(line):
      if line.strip.startsWith("#include"):
        var matches:array[2, string]
        if line.match(re"#include\s+([<""])([^>""]+)[>""]", matches):
          if matches[0] == "<":
            var path = tm_headers_dir & matches[1]
            let (idir, ifile, iext) = path.splitFile()
            result &= idir & "/" & ifile & iext
          elif matches[0] == "\"":
            let (idir, ifile, iext) = (dir & "/" & matches[1]).splitFile()
            result &= idir & "/" & ifile & iext

    strm.close()
  result

proc process(inputs: seq[string]): seq[string] =
  var nextInputs: seq[string]
  for i in inputs:
    nextInputs &= getIncludes(i)

  if nextInputs.len > 0:
    result &= nextInputs & process(nextInputs)

proc getOrder(inputs: seq[string]): seq[string] =
  var deps = inputs & process(inputs)
  deps.reverse()
  var included = initHashSet[string]()
  
  for i in deps:
    var path = i.normalizedPath()
    if not included.contains(path):
      included.incl(path)
      if fileExists(path):
        result.add(path)
  result.sort(proc(x, y:string): int =
    if x.contains("foundation"): -1
    elif y.contains("foundation"): 1
    else: 0
  )

var headers = getHeaders(tm_headers_dir)
var excluded:seq[string] = @[
    #"C:\\tm\\tm-nim\\headers\\plugins\\ui\\ui_custom.h", 
    #"C:\\tm\\tm-nim\\headers\\plugins\\ui\\ui.h", 
    #"C:\\tm\\tm-nim\\headers\\plugins\\ui\\draw2d.h", 
    "C:\\tm\\tm-nim\\headers\\plugins\\the_machinery_shared\\viewer.h", 
    #"C:\\tm\\tm-nim\\headers\\plugins\\the_machinery_shared\\component_interfaces\\editor_ui_interface.h", 
    #"C:\\tm\\tm-nim\\headers\\plugins\\simulation\\simulation_entry.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\simulation\\simulation.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\shader_system\\shader_system.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\shader_system\\shader_system_api_types.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\render_utilities\\gpu_picking.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\render_utilities\\render_component.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\render_graph_toolbox\\render_pipeline.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\render_graph\\render_graph.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\renderer\\commands.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\renderer\\renderer.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\renderer\\renderer_api_types.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\renderer\\render_backend.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\renderer\\render_command_buffer.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\physx\\physx_scene.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_body_component.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_collision.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_joint_component.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_material.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_mover_component.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_scene_settings.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\physics\\physics_shape_component.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\physics\\velocity_component.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\gamestate\\gamestate.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\entity\\entity.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\entity\\entity_api_types.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\entity\\entity_properties.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\entity\\tag_component.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\entity\\transform_component.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\asset_browser.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\asset_label.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\editor_views_loader.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\graph.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\profiler_view.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\properties.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\tree_view.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\editor_views\\ui_popup_item_picker.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\creation_graph.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\creation_graph_api_types.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\creation_graph_interpreter.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\creation_graph_loader.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\creation_graph_node_type.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\geometry_nodes.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\image_nodes.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\render_nodes.h",
    "C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\simulation_nodes.h",
    #"C:\\tm\\tm-nim\\headers\\plugins\\creation_graph\\creation_graph_output.inl",
    #"C:\\tm\\tm-nim\\headers\\foundation\\allocator.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\api_registry.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\api_types.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\api_type_hashes.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\application.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\asset_database.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\asset_io.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\base64.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\bounding_volume.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\buddy_allocator.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\buffer.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\buffer_format.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\camera.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\carray.inl",
    "C:\\tm\\tm-nim\\headers\\foundation\\collaboration.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\collaboration_p2p.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\config.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\core.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\core_importer.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\crash_recovery.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\error.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\feature_flags.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\git_ignore.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\hash.inl",
    "C:\\tm\\tm-nim\\headers\\foundation\\image_loader.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\input.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\integration_test.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\job_system.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\json.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\localizer.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\log.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\lz4.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\lz4_external.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\macros.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\math.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\math.inl",
    "C:\\tm\\tm-nim\\headers\\foundation\\memory_tracker.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\os.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\path.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\plugin.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\plugin_assets.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\plugin_callbacks.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\profiler.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\progress_report.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\random.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\runtime_data_repository.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\sha1.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\sprintf.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\sse2neon.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\string.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\string_repository.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\task_system.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\temp_allocator.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\the_truth.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\the_truth_assets.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\the_truth_migration.h",
    #"C:\\tm\\tm-nim\\headers\\foundation\\the_truth_types.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\undo.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\unicode.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\unicode_symbols.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\unit_test.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\visibility_flags.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\web_socket.h",
    "C:\\tm\\tm-nim\\headers\\foundation\\web_talker.h"]
var order = getOrder(headers).filterIt( it notin excluded ).join("\n")
open("tm_header_deps.txt", fmWrite).write(order)