let compile: (Model.project, Rules.compilation_unit) => unit =
  (project, cunit) =>
    switch (cunit) {
    | `Create_dir(path) =>
      Rules.mkdir(path);
      Logs.info(m => m("Created dir: %s", path |> Fpath.to_string));
    | `Compile(target) =>
      Rules.compile(project.output_dir, target);
      Logs.info(m =>
        Fpath.(
          m(
            "Compiled: %s => %s",
            target.input |> to_string,
            append(project.output_dir, target.output) |> to_string,
          )
        )
      );
    };
