const TM_TT_NO_UNDO_SCOPE = tm_tt_undo_scope_t(u64:0)

#template tm_tt_read(t: ptr tm_the_truth_api, tt: ptr tm_the_truth_o, obj: tm_tt_id_t): untyped =
  #t.read(tt, obj)
