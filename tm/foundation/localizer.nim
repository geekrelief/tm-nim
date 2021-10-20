template TM_LOCALIZE(l: ptr tm_localizer_api, s: string): untyped =
  (l.def[]).localize(l.def[].inst, cstring(s), "")

template TM_LOCALIZE(l: ptr tm_localizer_api, s: string, ctx: string): untyped =
  (l.def[]).localize(l.def[].inst, cstring(s), cstring(ctx))