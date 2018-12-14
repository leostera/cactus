let to_string = x =>
  switch (x) {
  | `No_project_file(_pwd) => {j| Could not find a project file at $pwd |j}
  | _ => "Unknown error"
  };
