let to_string = x =>
  switch (x) {
  | `No_project_file(pwd) =>
    Printf.sprintf(
      "Could not find a project file at %s",
      Filename.concat(pwd, Model.project_filename),
    )
  | _ => "Unknown error"
  };
