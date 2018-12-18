open Lwt.Infix;

let compile: (Model.project, Rules.compilation_unit) => Lwt.t(unit) =
  (project, cunit) =>
    switch (cunit) {
    | `Create_dir(path) =>
      Rules.mkdir(path)
      >>= (
        _ => {
          let path = path |> Fpath.to_string;
          Logs_lwt.info(m => m("Created dir: %s", path));
        }
      )

    | `Compile(target) =>
      Rules.compile(project.output_dir, target)
      >>= (
        () =>
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
    };
