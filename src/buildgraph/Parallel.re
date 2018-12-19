let create = (~f, worker_count) => {
  open Lwt.Infix;
  Logs.debug(m =>
    m("Spinning up worker pool with %d workers...", worker_count)
  );
  let (pool, pool_done) = Nproc.create(worker_count);
  let submit = work =>
    work
    |> Nproc.submit(pool, ~f=x =>
         f(x)
         >>= (_ => Logs_lwt.debug(m => m("Task completed.")))
         |> Lwt_main.run
       )
    >>= (
      submission =>
        switch (submission) {
        | None => Logs_lwt.err(m => m("Failed to submit task"))
        | Some () => Logs_lwt.debug(m => m("Task submitted!"))
        }
    );

  let teardown = _ => Nproc.close(pool) >>= (_ => pool_done);

  (submit, teardown);
};
