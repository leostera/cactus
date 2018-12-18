let compile: (Model.project, Rules.compilation_unit) => Lwt.t(unit) =
  (project, cunit) =>
    switch (cunit) {
    | `Create_dir(path) =>
      Logs.info(m => m("Creating dir: %s", path |> Fpath.to_string));
      Lwt.return(Rules.mkdir(path));
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
