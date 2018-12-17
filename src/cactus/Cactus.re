let build = (_flags, project_root, output_dir) => {
  let began_at = Unix.gettimeofday();

  Logs.app(m => m({j|🌵 Compiling project... |j}));

  let parseResult = Parser.read_project(project_root, output_dir);
  switch (parseResult) {
  | Ok(project) =>
    let began_at = Unix.gettimeofday();

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

    if (size > 50) {
      Logs.debug(m => m("Spinning up worker pool..."));
      let thread_count = 4;
      let (pool, pool_done) = Nproc.create(thread_count);
      let submit = Nproc.submit(pool, ~f=compile);
      Lwt.(
        Buildgraph.execute_p(submit, compile, graph)
        >>= (_ => Nproc.close(pool))
        >>= (_ => pool_done)
        |> Lwt_main.run
      );
      Logs.debug(m => m("Finished parallel execution."));
    } else {
      Buildgraph.execute(compile, graph);
    };

    Logs.debug(m => {
      let finished_at = Unix.gettimeofday();
      let delta = finished_at -. began_at;
      m({j|Processes %d compilation units in %.3f s|j}, size, delta);
    });
  | Error(err) =>
    Logs.err(m => err |> Parser.Errors.to_string |> m("ERROR: %s"))
  };

  Logs.app(m => {
    let finished_at = Unix.gettimeofday();
    let delta = finished_at -. began_at;
    let has_errors = Logs.err_count() > 0;
    let msg = if (has_errors) {{j|💀 Failed in|j}} else {{j|🌮 Done in|j}};
    m("%s %0.3fs", msg, delta);
  });
};
