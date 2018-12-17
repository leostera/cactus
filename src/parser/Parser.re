module Errors = Errors;

let read_project = (root, output_dir) =>
  if (Model.project_filename |> Filename.concat(root) |> Sys.file_exists) {
    /* TODO(@ostera): read cactus-project to get the lang version and name */
    let name = "words";
    Ok(Model.{root, name, output_dir});
  } else {
    Error(`No_project_file(root));
  };

/* TODO(@ostera): read path file, validate rules, and turn them into a
 * Model.site record */
let read_site = path =>
  Model.{
    path,
    dir: Filename.dirname(path),
    name: Filename.basename(path),
    charset: "utf-8",
  };
