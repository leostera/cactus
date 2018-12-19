let compile: (Model.project, Rules.compilation_unit) => unit =
  (project, cunit) =>
    switch (cunit) {
    | `Create_dir(path) =>
      Logs.info(m => m("Creating dir: %s", path |> Fpath.to_string));
      Rules.mkdir(path);
    | `Compile(target) =>
      Logs.info(m =>
        Fpath.(
          m(
            "Compiling: %s => %s",
            target.input |> to_string,
            append(project.output_dir, target.output) |> to_string,
          )
        )
      );
      Rules.compile(project.output_dir, target);
    };

let compile_async: (Model.project, Rules.compilation_unit) => Lwt.t(unit) =
  (project, cunit) =>
    Lwt.Infix.(
      switch (cunit) {
      | `Create_dir(path) =>
        Rules.mkdir_async(path)
        >>= (
          _ =>
            Logs_lwt.info(m => m("Created dir: %s", path |> Fpath.to_string))
        )
      | `Compile(target) =>
        Rules.compile_async(project.output_dir, target)
        >>= (
          _ =>
            Logs_lwt.info(m =>
              Fpath.(
                m(
                  "Compiled: %s => %s",
                  target.input |> to_string,
                  append(project.output_dir, target.output) |> to_string,
                )
              )
            )
        )
      }
    );
