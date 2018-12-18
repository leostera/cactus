let build = (_flags, project_root, output_dir, jobs) => {
  let began_at = Unix.gettimeofday();
  let project_root = project_root |> Fpath.v |> Fpath.to_dir_path;
  let output_dir = output_dir |> Fpath.v |> Fpath.to_dir_path;

  Logs.app(m => m({j|🌵 Compiling project... |j}));

  let parseResult = Parser.read_project(project_root, output_dir);
  switch (parseResult) {
  | Ok(project) =>
    Logs.debug(m => m("Creating build plan..."));
    let graph = project |> Buildplanner.plan_build;
    Logs.debug(m => {
      let finished_at = Unix.gettimeofday();
      let delta = finished_at -. began_at;
      m({j|Created build plan in %.3f s|j}, delta);
    });

    let size = Buildgraph.size(graph);
    Logs.debug(m => m("About to process %d compilation units...", size));

    let compile = Compiler.Exec.compile(project);

    if (size > 50 && jobs > 0) {
      Logs.debug(m => m("Spinning up worker pool..."));
      let (pool, pool_done) = Nproc.create(jobs);
      let submit =
        Nproc.submit(
          pool,
          ~f=plan => {
            Logs.debug(m =>
              m(
                "Received build plan with %d targets",
                plan |> Buildgraph.size,
              )
            );
            Lwt.catch(
              () => plan |> Buildgraph.execute(compile),
              exn =>
                Logs_lwt.err(m =>
                  m(
                    "Something went wrong in worker: %s",
                    exn |> Printexc.to_string,
                  )
                ),
            )
            |> Lwt_main.run;
            Logs.debug(m => m("Worker done."));
          },
        );
      Lwt.(
        Buildgraph.execute_p(submit, compile, graph)
        >>= (_ => Logs_lwt.debug(m => m("Execution finished.")))
        >>= (_ => Nproc.close(pool))
        >>= (_ => pool_done)
        |> Lwt_main.run
      );
      Logs.debug(m => m("Finished parallel execution."));
    } else {
      Buildgraph.execute(compile, graph) |> Lwt_main.run;
    };

    Logs.debug(m => {
      let finished_at = Unix.gettimeofday();
      let delta = finished_at -. began_at;
      m({j|Processes %d compilation units in %.3f s|j}, size, delta);
    });
    Logs.app(m => {
      let finished_at = Unix.gettimeofday();
      let delta = finished_at -. began_at;
      let has_errors = Logs.err_count() > 0;
      let msg =
        if (has_errors) {
          {j|💀 Failed|j};
        } else {
          Printf.sprintf({j|🌮 Done with %d targets|j}, size);
        };
      m("%s in %0.3fs", msg, delta);
    });
  | Error(err) => Logs.app(m => err |> Parser.Errors.to_string |> m("%s"))
  };
};
