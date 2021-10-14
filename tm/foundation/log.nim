template info*(l: ptr tm_logger_api, s: string) =
  l.print(TM_LOG_TYPE_INFO, cstring(s))