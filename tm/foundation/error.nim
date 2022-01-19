
template TM_ERROR(error_i, msg: untyped): untyped = 
  ## Reports an error using ``tm_error_i.errorf()``. Often called as `TM_ERROR(tm_error_api->def, msg)`.
  let i = instantiationInfo()
  `error_i`.errorf(`error_i`.inst, cstring(i.filename), i.line.uint32, cstring(`msg`))


template TM_ASSERT(test, error_i, msg, body: untyped): untyped = 
  if not test:
    let i = instantiationInfo()
    `error_i`.errorf(`error_i`.inst, cstring(i.filename), i.line.uint32, cstring(`msg`))
    `body`