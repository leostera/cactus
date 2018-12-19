let create = (~f, _worker_count) => {
  Logs.debug(m => m("Reusing event loop from main process..."));

  let submit = f;

  let teardown = _ => Lwt.return_unit;

  (submit, teardown);
};
