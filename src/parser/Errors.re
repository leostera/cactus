let to_string = x =>
  switch (x) {
  | `No_project_file(pwd) =>
    Printf.sprintf(
      "Could not find a project file at %s",
      Fpath.(append(pwd, Model.project_filename) |> to_string),
    )
  | `Not_implemented(name) =>
    Printf.sprintf("This feature has not been implemented yet: %s", name)
  | _ => "Unknown error"
  };
